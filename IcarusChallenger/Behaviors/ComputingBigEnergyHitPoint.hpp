#pragma once

#include "../Framework/ProcessorBase.hpp"
#include "../Modules/LastestValue.hpp"
#include <opencv2/opencv.hpp>
#include <GaiaSharedPicture/GaiaSharedPicture.hpp>
#include <tbb/tbb.h>

namespace Icarus
{
    class ComputingBigEnergyHitPoint : public ProcessorBase
    {
        REFLECT_TYPE(Icarus, ProcessorBase)

    protected:

        std::unique_ptr<Gaia::SharedPicture::PictureWriter> ResultWriter;

        cv::Mat* Picture;
        cv::Rect* R {nullptr};
        std::optional<cv::RotatedRect>* Panel {nullptr};
        cv::Point2i* HitPoint {nullptr};
        int* HitCommand {nullptr};

        /// In unit of milliseconds.
        double CompensationTime;
        double UpperSpeedThreshold;
        double LowerSpeedThreshold;

        std::optional<double> ScreenWidth;
        std::optional<double> ScreenHeight;

        std::chrono::system_clock::time_point LastConfigureTime;
        std::optional<std::chrono::steady_clock::time_point> PreviousFrameTime;
        std::optional<long> PreviousDeltaTime;
        std::optional<double> PreviousAngleDegree;
        std::optional<double> PreviousDeltaAngleDegree;
        double AccumulatedDeltaAngleSymbol {0};

        LatestValue<double, 12> HistorySpeed {};

        /// Whether the clock is synchronized with the big energy or not.
        bool Synchronized {false};
        /// The time point of the relative zero time point.
        std::chrono::steady_clock::time_point RelativeZeroTimePoint;

        /// Load contour filtering configurations.
        void LoadConfigurations() override;

        /// Initialize blackboard values.
        void OnInitialize() override;

        /// Filtering all possible contour elements.
        BehaviorTree::Result OnExecute() override;

        static double GetTargetSpeed(double target_relative_seconds);
        static double GetTargetIntegral(double target_relative_seconds);
        static double GetDeltaIntegral(double beginning_relative_seconds,
                                       double delta_relative_seconds);
    };
}