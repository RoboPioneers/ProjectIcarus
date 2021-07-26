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
        std::unique_ptr<Gaia::SharedPicture::PictureWriter> MaskWriter;
        std::unique_ptr<Gaia::SharedPicture::PictureWriter> RWriter;

        /// Template picture of R for template matching.
        cv::Mat TemplatePicture;
        /// Threshold for mask.
        int Threshold;

        int RMinLength {18};
        int RMaxLength {24};

        /// Address of the picture to process in the blackboard.
        cv::Mat* Picture {nullptr};

        std::optional<cv::Rect>* R {nullptr};

        /// Load contour filtering configurations.
        void LoadConfigurations() override;

        /// Initialize blackboard values.
        void OnInitialize() override;

        /// Detect a possible R.
        BehaviorTree::Result OnExecute() override;
    };
}