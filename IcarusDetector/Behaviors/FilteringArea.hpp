#pragma once

#include "../Framework/ProcessorBase.hpp"
#include <opencv2/opencv.hpp>
#include <GaiaTags/GaiaTags.hpp>
#include <GaiaSharedPicture/GaiaSharedPicture.hpp>
#include <tbb/tbb.h>

namespace Icarus
{
    class FilteringArea : public ProcessorBase
    {
        REFLECT_TYPE(Icarus, ProcessorBase)

    protected:
        /// Milliseconds to fully lost the ROI
        int InterestedAreaToleranceLosingTime;

        /// Main picture in the blackboard.
        cv::Mat* MainPicture {nullptr};
        /// Found target in the previous frame.
        std::optional<cv::RotatedRect>* FoundTarget {nullptr};
        /// Generated interested area that very likely contains a target.
        cv::Rect* InterestedArea;

        /// The time point of the last time when refresh ROI.
        std::chrono::steady_clock::time_point InterestedAreaRefreshTimestamp;

        /// The previous interested area.
        std::optional<cv::Rect> PreviousInterestedArea {std::nullopt};
        /// The beginning time of firstly lost the ROI.
        std::optional<std::chrono::steady_clock::time_point> PreviousInterestedAreaTimestamp {std::nullopt};

        /// Load contour filtering configurations.
        void LoadConfigurations() override;

        /// Initialize blackboard values.
        void OnInitialize() override;

        /// Filtering all possible contour elements.
        BehaviorTree::Result OnExecute() override;
    };
}