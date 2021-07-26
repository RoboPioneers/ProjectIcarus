#pragma once

#include "../Framework/ProcessorBase.hpp"
#include <opencv2/opencv.hpp>
#include <GaiaSharedPicture/GaiaSharedPicture.hpp>
#include <tbb/tbb.h>

namespace Icarus
{
    class DetectingPanel : public ProcessorBase
    {
        REFLECT_TYPE(Icarus, ProcessorBase)

    protected:

        std::unique_ptr<Gaia::SharedPicture::PictureWriter> MaskWriter;

        int Threshold;
        int MinWidth, MaxWidth, MinLength, MaxLength;
        double MinLengthWidthRatio, MaxLengthWidthRatio;
        int MaxArmLength;

        /// Address of the picture to process in the blackboard.
        cv::Mat* Picture {nullptr};

        std::optional<cv::RotatedRect>* Panel {nullptr};
        std::optional<cv::Rect>* R {nullptr};

        /// Load contour filtering configurations.
        void LoadConfigurations() override;

        /// Initialize blackboard values.
        void OnInitialize() override;

        /// Filtering all possible contour elements.
        BehaviorTree::Result OnExecute() override;
    };
}