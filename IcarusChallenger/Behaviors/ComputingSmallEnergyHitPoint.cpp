#include "ComputingSmallEnergyHitPoint.hpp"
#include "../Modules/AngleTool.hpp"

namespace Icarus
{

    void ComputingSmallEnergyHitPoint::LoadConfigurations()
    {
        CompensationAngle = GetConfigurator()->Get<int>("CompensationAngle").value_or(21);
    }

    void ComputingSmallEnergyHitPoint::OnInitialize()
    {
        InitializeFacilities();

        R = GetBlackboard()->GetPointer<cv::Rect>("R");
        Panel = GetBlackboard()->GetPointer<std::optional<cv::RotatedRect>>("Panel");
        HitPoint = GetBlackboard()->GetPointer<cv::Point2i>("HitPoint");
        HitCommand = GetBlackboard()->GetPointer<int>("HitCommand");
        ResultWriter = std::make_unique<Gaia::SharedPicture::PictureWriter>("icarus.challenger.result",
                                                                             1920 * 1080 * 3);
        Picture = GetBlackboard()->GetPointer<cv::Mat>("MainPicture");
        LoadConfigurations();
    }

    BehaviorTree::Result ComputingSmallEnergyHitPoint::OnExecute()
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

        if (!Panel->has_value())
        {
            return BehaviorTree::Result::Failure;
        }

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
        auto r_x = R->x;
        auto r_y = R->y;
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
        if (!PreviousAngleDegree) PreviousAngleDegree = target_angle_degree;
        auto delta_angle_degree = target_angle_degree - *PreviousAngleDegree;

        //============================================================
        // Calculate delta data.
        //============================================================

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
            delta_angle_degree = *PreviousDeltaAngleDegree;
        }

        DEBUG_BEGIN
        GetInspector()->UpdateValue("DeltaAngle", delta_angle_degree);
        DEBUG_END

        // Reset the symbol accumulator to prevent overflow.
        if(AccumulatedDeltaAngleSymbol < -100) AccumulatedDeltaAngleSymbol = -50;
        else if (AccumulatedDeltaAngleSymbol > 100) AccumulatedDeltaAngleSymbol = 50;

        auto predicting_angle = target_angle_degree;
        if (AccumulatedDeltaAngleSymbol > 0)
        {
            predicting_angle += CompensationAngle;
        }
        else
        {
            predicting_angle -= CompensationAngle;
        }

        DEBUG_BEGIN
        GetInspector()->UpdateValue("PredictingAngle", predicting_angle);
        DEBUG_END

        double hit_point_angle_radian = Modules::AngleTool::ToRadian(predicting_angle);
        double hit_point_arm_length = distance * 1.00f;
        HitPoint->x = r_x + static_cast<int>(hit_point_arm_length * std::cos(hit_point_angle_radian));
        HitPoint->y = r_y + static_cast<int>(hit_point_arm_length * std::sin(hit_point_angle_radian));

        #ifdef DEBUG
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
        cv::circle(result_picture, cv::Point(R->x, R->y), 3,
                   cv::Scalar(0, 255, 0), 2);
        // Draw the expectation circle
        cv::circle(result_picture, cv::Point(R->x, R->y), static_cast<int>(distance),
                   cv::Scalar(0, 200), 2);
        // Shrink the size of the picture.
        cv::resize(result_picture, result_picture,
                   cv::Size(result_picture.cols / 2, result_picture.rows / 2));
        ResultWriter->Write(result_picture);
        #endif

        // Calculate hit point in the turret protocol coordinate.
        *HitCommand = 1;
        HitPoint->x = HitPoint->x - static_cast<int>(*ScreenWidth / 2.0);
        HitPoint->y = static_cast<int>(*ScreenHeight / 2.0) -  HitPoint->y;
        if (std::fabs(delta_angle_degree) > 0.01f && std::fabs(delta_angle_degree) < 10.00f)
        {
            PreviousDeltaAngleDegree = delta_angle_degree;
        }
        PreviousAngleDegree = target_angle_degree;

        return BehaviorTree::Result::Success;
    }
}