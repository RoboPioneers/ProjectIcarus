#pragma once

#include "../Framework/ProcessorBase.hpp"
#include <opencv2/opencv.hpp>
#include <GaiaSharedPicture/GaiaSharedPicture.hpp>
#include <tbb/tbb.h>

namespace Icarus
{
    class DetectingR : public ProcessorBase
    {
        REFLECT_TYPE(Icarus, ProcessorBase)

    protected:
        /// Template picture of R for template matching.
        cv::Mat TemplatePicture;
        /// Address of the picture to process in the blackboard.
        cv::Mat* Picture {nullptr};

        cv::Rect* R {nullptr};
        std::optional<cv::RotatedRect>* Panel {nullptr};

        bool PreviousRFound {false};

        /// Load contour filtering configurations.
        void LoadConfigurations() override;

        /// Initialize blackboard values.
        void OnInitialize() override;

        /// Detect a possible R.
        BehaviorTree::Result OnExecute() override;
    };
}