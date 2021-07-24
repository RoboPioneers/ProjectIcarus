#pragma once

#include "../Framework/ProcessorBase.hpp"
#include <opencv2/opencv.hpp>
#include <GaiaSharedPicture/GaiaSharedPicture.hpp>
#include <tbb/tbb.h>

namespace Icarus
{
    class ComputingSmallEnergyHitPoint : public ProcessorBase
    {
        REFLECT_TYPE(Icarus, ProcessorBase)

    protected:

        std::unique_ptr<Gaia::SharedPicture::PictureWriter> ResultWriter;

        cv::Mat* Picture {nullptr};
        cv::Rect* R {nullptr};
        std::optional<cv::RotatedRect>* Panel {nullptr};
        cv::Point2i* HitPoint {nullptr};
        int* HitCommand {nullptr};

        /// In unit of pixel.
        int CompensationAngle {0};

        std::optional<double> ScreenWidth;
        std::optional<double> ScreenHeight;

        std::chrono::system_clock::time_point LastConfigureTime;
        std::optional<double> PreviousAngleDegree;
        std::optional<double> PreviousDeltaAngleDegree;
        double AccumulatedDeltaAngleSymbol {0};

        /// Load contour filtering configurations.
        void LoadConfigurations() override;

        /// Initialize blackboard values.
        void OnInitialize() override;

        /// Filtering all possible contour elements.
        BehaviorTree::Result OnExecute() override;
    };
}