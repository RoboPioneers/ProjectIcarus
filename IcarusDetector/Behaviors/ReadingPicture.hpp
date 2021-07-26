#pragma once

#include "../Framework/ProcessorBase.hpp"
#include <opencv2/opencv.hpp>
#include <utility>

namespace Icarus
{
    /// Display the main picture.
    class ReadingPicture : public ProcessorBase
    {
        REFLECT_TYPE(Icarus, ProcessorBase)

    protected:

        /// Main picture in the blackboard.
        cv::Mat* MainPicture {nullptr};

        std::string PictureName;

        /// Initialize camera facilities.
        void OnInitialize() override
        {
            InitializeFacilities();

            MainPicture = GetBlackboard()->GetPointer<cv::Mat>("MainPicture");
        }

        /// Read a picture.
        BehaviorTree::Result OnExecute() override
        {
            *MainPicture = cv::imread(PictureName);
            return BehaviorTree::Result::Success;
        }

    public:
        ReadingPicture(Behavior *parent_behavior, const std::unordered_set<std::string>& type_names,
                       std::string picture_name) : ProcessorBase(parent_behavior, type_names),
                       PictureName(std::move(picture_name))
        {}
    };
}