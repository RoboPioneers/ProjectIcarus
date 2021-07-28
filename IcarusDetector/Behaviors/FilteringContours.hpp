#pragma once

#include "../Framework/ProcessorBase.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/cudafilters.hpp>
#include <GaiaTags/GaiaTags.hpp>
#include <GaiaSharedPicture/GaiaSharedPicture.hpp>
#include <tbb/tbb.h>

#include "../Components/ContourElement.hpp"

namespace Icarus
{
    class FilteringContours : public ProcessorBase
    {
        REFLECT_TYPE(Icarus, ProcessorBase)

    private:
        cv::Ptr<cv::cuda::Filter> DilateFilter {nullptr};

        std::unique_ptr<SharedPicture::PictureWriter> MaskWriter;
    protected:

        /// Min hue of enemy color.
        unsigned int* EnemyMinHue {nullptr};
        /// Max hue of enemy color.
        unsigned int* EnemyMaxHue {nullptr};
        /// Min hue of alley color.
        unsigned int* AlleyMinHue {nullptr};
        /// Max hue of alley color.
        unsigned int* AlleyMaxHue {nullptr};

        unsigned int IntensityThreshold;
        unsigned int ValueThreshold;
        unsigned int SaturationThreshold;
        unsigned int EnemyDilateSize;
        unsigned int AlleyDilateSize;

        /// Main picture in the blackboard.
        cv::Mat* MainPicture {nullptr};
        cv::Mat* MaskPicture {nullptr};

        /// Generated interested area that very likely contains a target.
        cv::Rect* InterestedArea;
        tbb::concurrent_vector<ContourElement::Pointer>* Contours;

        /// Load contour filtering configurations.
        void LoadConfigurations() override;

        /// Initialize blackboard values.
        void OnInitialize() override;

        /// Filtering all possible contour elements.
        BehaviorTree::Result OnExecute() override;
    };
}