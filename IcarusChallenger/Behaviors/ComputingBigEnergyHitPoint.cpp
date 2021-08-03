#include "ComputingBigEnergyHitPoint.hpp"
#include "../Modules/AngleTool.hpp"
#include <cmath>

namespace Icarus
{
    double ComputingBigEnergyHitPoint::GetTargetSpeed(double target_relative_seconds)
    {
        return 0.785 * std::sin(1.884 * target_relative_seconds) + 1.305;
    }

    double ComputingBigEnergyHitPoint::GetTargetIntegral(double target_relative_seconds)
    {
        return -0.785 / 1.884 * std::cos(1.884 * target_relative_seconds)
               + 1.305 * target_relative_seconds;
    }

    double ComputingBigEnergyHitPoint::GetDeltaIntegral(double beginning_relative_seconds,
                                                        double delta_relative_seconds)
    {
        return GetTargetIntegral(
                beginning_relative_seconds + delta_relative_seconds)
               - GetTargetIntegral(beginning_relative_seconds);
    }

    void ComputingBigEnergyHitPoint::LoadConfigurations()
    {
        CompensationTime = GetConfigurator()->Get<double>("CompensationTime").value_or(250.0);
        UpperSpeedThreshold = GetConfigurator()->Get<double>("UpperSpeedThreshold").value_or(0.1180);
        LowerSpeedThreshold = GetConfigurator()->Get<double>("LowerSpeedThreshold").value_or(0.0310);
    }

    void ComputingBigEnergyHitPoint::OnInitialize()
    {
        InitializeFacilities();

        R = GetBlackboard()->GetPointer<std::optional<cv::Rect>>("R");
        Panel = GetBlackboard()->GetPointer<std::optional<cv::RotatedRect>>("Panel");
        HitPoint = GetBlackboard()->GetPointer<cv::Point2i>("HitPoint");
        HitCommand = GetBlackboard()->GetPointer<int>("HitCommand");
        ResultWriter = std::make_unique<Gaia::SharedPicture::PictureWriter>("icarus.challenger.result_b",
                                                                             1920 * 1080 * 3);
        Picture = GetBlackboard()->GetPointer<cv::Mat>("MainPicture");
        LoadConfigurations();
    }

    BehaviorTree::Result ComputingBigEnergyHitPoint::OnExecute()
    {
        DEBUG_BEGIN
        CheckReloadConfiguration();
        DEBUG_END

        //============================================================
        // Initialize result.
        //============================================================

        *HitCommand = 0;
        HitPoint->x = 0;
        HitPoint->y = 0;

        if (!R->has_value() || !Panel->has_value())
            return BehaviorTree::Result::Failure;

        if (!ScreenWidth || !ScreenHeight)
        {
            ScreenWidth = static_cast<double>(Picture->cols);
            ScreenHeight = static_cast<double>(Picture->rows);
        }

        //============================================================
        // Calculate current data.
        //============================================================

        auto panel_x = (*Panel)->center.x;
        auto panel_y = (*Panel)->center.y;
        auto r_x = R->value().x;
        auto r_y = R->value().y;
        double delta_x = panel_x - static_cast<float>(r_x);
        double delta_y = panel_y - static_cast<float>(r_y);
        double distance = std::sqrt(delta_x * delta_x + delta_y * delta_y);

        double target_angle_radian = std::acos(delta_x / distance);
        double target_angle_degree = Modules::AngleTool::ToDegree(target_angle_radian);
        // Adjust the range of target_angle_degree to [0, 360)
        if (delta_y < 0)
        {
            target_angle_degree = 360.0f - target_angle_degree;
            if (target_angle_degree >= 359.0f)
            {
                target_angle_degree = 0.0f;
            }
        }

        auto current_steady_time = std::chrono::steady_clock::now();

        //============================================================
        // Check history data.
        //============================================================

        // Frame time.
        if (!PreviousFrameTime.has_value())
        {
            PreviousFrameTime = std::chrono::steady_clock::now();
            PreviousAngleDegree = target_angle_degree;
            PreviousDeltaAngleDegree = 0.0f;
            PreviousDeltaTime = 0.0f;
            GetLogger()->RecordMessage("No previous frame data, skipped.");
            return BehaviorTree::Result::Failure;
        }

        if (!PreviousAngleDegree) PreviousAngleDegree = target_angle_degree;
        auto delta_angle_degree = target_angle_degree - *PreviousAngleDegree;

        //============================================================
        // Calculate delta data.
        //============================================================

        auto delta_time = std::chrono::duration_cast<std::chrono::microseconds>(
                current_steady_time - *PreviousFrameTime).count();

        DEBUG_BEGIN
        GetInspector()->UpdateValue("DeltaTime", delta_time);
        DEBUG_END

        // Otherwise the panel sector has changed.
        if (std::fabs(delta_angle_degree) > 0.01f && std::fabs(delta_angle_degree) < 10.00f)
        {
            if (delta_angle_degree > 0.0f)
            {
                AccumulatedDeltaAngleSymbol++;
            }
            else
            {
                AccumulatedDeltaAngleSymbol--;
            }
        }
        else
        {
            delta_time = *PreviousDeltaTime;
            delta_angle_degree = *PreviousDeltaAngleDegree;
        }

        auto current_angle_speed = delta_angle_degree / static_cast<double>(delta_time) * 1000.0f;

        HistorySpeed.Add(current_angle_speed);

        auto angle_speed = HistorySpeed.Mean();

        if (std::fabs(angle_speed) > UpperSpeedThreshold)
        {
            Synchronized = true;
            RelativeZeroTimePoint = current_steady_time - std::chrono::microseconds(840000);
            GetLogger()->RecordMilestone("Highest speed:" + std::to_string(angle_speed) + ", time synchronized.");
        } else if (std::fabs(angle_speed) < LowerSpeedThreshold)
        {
            Synchronized = true;
            RelativeZeroTimePoint = current_steady_time - std::chrono::microseconds(2498664);
            GetLogger()->RecordMilestone("Lowest speed:" + std::to_string(angle_speed) + ", time synchronized.");
        }

        DEBUG_BEGIN
        GetInspector()->UpdateValue("DeltaAngle", delta_angle_degree);
        GetInspector()->UpdateValue("AngleSpeed", angle_speed);
        DEBUG_END

        // Reset the symbol accumulator to prevent overflow.
        if(AccumulatedDeltaAngleSymbol < -100) AccumulatedDeltaAngleSymbol = -50;
        else if (AccumulatedDeltaAngleSymbol > 100) AccumulatedDeltaAngleSymbol = 50;

        double compensation_angle = 0.0f;
        if (Synchronized)
        {

            // Current past milliseconds from the zero time point.
            auto deduced_current_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                    current_steady_time - RelativeZeroTimePoint).count();

            auto deduced_current_seconds = static_cast<double>(deduced_current_milliseconds) / 1000.0f;
            auto delta_seconds = CompensationTime / 1000.0f;

            compensation_angle = Modules::AngleTool::ToDegree(GetDeltaIntegral(
                    deduced_current_seconds, delta_seconds));
        }

        DEBUG_BEGIN
        GetInspector()->UpdateValue("CompensationAngle", compensation_angle);
        DEBUG_END

        auto predicting_angle = target_angle_degree;
        if (AccumulatedDeltaAngleSymbol > 0)
        {
            predicting_angle += compensation_angle;
        }
        else
        {
            predicting_angle -= compensation_angle;
        }


        DEBUG_BEGIN
        GetInspector()->UpdateValue("PredictingAngle", predicting_angle);
        DEBUG_END

        double hit_point_angle_radian = Modules::AngleTool::ToRadian(predicting_angle);
        double hit_point_arm_length = distance * 1.00f;
        HitPoint->x = r_x + static_cast<int>(hit_point_arm_length * std::cos(hit_point_angle_radian));
        HitPoint->y = r_y + static_cast<int>(hit_point_arm_length * std::sin(hit_point_angle_radian));

        DEBUG_BEGIN
        auto result_picture = Picture->clone();
        // Draw hit point mark.
        cv::circle(result_picture, *HitPoint, 8,
                   cv::Scalar(0, 255, 0), 2);
        cv::Point2i lp = *HitPoint, rp = *HitPoint, up = *HitPoint, bp = *HitPoint;
        lp.x -= 12; rp.x += 12;
        up.y += 12; bp.y -= 12;
        cv::line(result_picture, lp, rp, cv::Scalar(0, 255, 0), 2);
        cv::line(result_picture, up, bp, cv::Scalar(0, 255, 0), 2);
        // Draw the position of R.
        cv::circle(result_picture, cv::Point(R->value().x, R->value().y), 3,
                   cv::Scalar(0, 255, 0), 2);
        // Draw the expectation circle
        cv::circle(result_picture, cv::Point(R->value().x, R->value().y), static_cast<int>(distance),
                   cv::Scalar(0, 200), 2);
        // Shrink the size of the picture.
        cv::resize(result_picture, result_picture,
                   cv::Size(result_picture.cols / 2, result_picture.rows / 2));
        ResultWriter->Write(result_picture);
        DEBUG_END

        // Calculate hit point in the turret protocol coordinate.
        if (Synchronized)
        {
            *HitCommand = 1;
        }
        else
        {
            *HitCommand = 2;
        }

        HitPoint->x = HitPoint->x - static_cast<int>(*ScreenWidth / 2.0);
        HitPoint->y = static_cast<int>(*ScreenHeight / 2.0) -  HitPoint->y;

        PreviousFrameTime = current_steady_time;
        PreviousDeltaTime = delta_time;
        if (std::fabs(delta_angle_degree) > 0.01f && std::fabs(delta_angle_degree) < 10.00f)
        {
            PreviousDeltaAngleDegree = delta_angle_degree;
        }
        PreviousAngleDegree = target_angle_degree;

        return BehaviorTree::Result::Success;
    }
}