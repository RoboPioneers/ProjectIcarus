#pragma once

#include "../Framework/ProcessorBase.hpp"
#include <opencv2/opencv.hpp>

namespace Icarus
{
    /// Display the main picture.
    class ReadingPicture : public ProcessorBase
    {
        REFLECT_TYPE(Icarus, ProcessorBase)

    protected:

        /// Main picture in the blackboard.
        cv::Mat* MainPicture {nullptr};

        /// Initialize camera facilities.
        void OnInitialize() override
        {
            InitializeFacilities();

            MainPicture = GetBlackboard()->GetObject<cv::Mat>("MainPicture");
        }

        /// Read a picture.
        BehaviorTree::Result OnExecute() override
        {
            *MainPicture = cv::imread("6.5m.1.bmp");
            return BehaviorTree::Result::Success;
        }
    };
}