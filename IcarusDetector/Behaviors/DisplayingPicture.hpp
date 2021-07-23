#pragma once

#include "../Framework/ProcessorBase.hpp"
#include <opencv2/opencv.hpp>

namespace Icarus
{
    /// Display the main picture.
    class DisplayingPicture : public ProcessorBase
    {
        REFLECT_TYPE(Icarus, ProcessorBase)

    protected:

        /// Main picture in the blackboard.
        cv::Mat* MainPicture {nullptr};

        /// Initialize camera facilities.
        void OnInitialize() override
        {
            InitializeFacilities();

            MainPicture = GetBlackboard()->GetObject<cv::Mat>("ColorMask");
        }

        /// Read a picture.
        BehaviorTree::Result OnExecute() override
        {
            cv::imshow("Picture", *MainPicture);
            if (cv::waitKey(1) == 27) return BehaviorTree::Result::Failure;

            return BehaviorTree::Result::Success;
        }
    };
}