#pragma once

#include "../Framework/ProcessorBase.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/cudafilters.hpp>
#include <GaiaTags/GaiaTags.hpp>
#include <GaiaSharedPicture/GaiaSharedPicture.hpp>
#include <tbb/tbb.h>
#include "../Components/ContourElement.hpp"
#include "../Checkers/FarLightBarChecker.hpp"
#include "../Checkers/MediumLightBarChecker.hpp"
#include "../Checkers/NearLightBarChecker.hpp"

namespace Icarus
{
    class FilteringLightBars : public ProcessorBase
    {
        REFLECT_TYPE(Icarus, ProcessorBase)

    private:
        std::unique_ptr<SharedPicture::PictureWriter> ContoursWriter;

    protected:

        unsigned int LightBarNearMediumSeparator;
        unsigned int LightBarMediumFarSeparator;
        unsigned int LightBarMaxLeaningAngle;

        /// Main picture in the blackboard.
        cv::Mat* MainPicture {nullptr};
        /// Detected contours.
        tbb::concurrent_vector<ContourElement::Pointer>* Contours;

        Tags::Layer* LightBarLayer;
        Tags::Layer* LightBarFarLayer;
        Tags::Layer* LightBarMediumLayer;
        Tags::Layer* LightBarNearLayer;

        /// Load contour filtering configurations.
        void LoadConfigurations() override;

        /// Initialize blackboard values.
        void OnInitialize() override;

        /// Filtering all possible contour elements.
        BehaviorTree::Result OnExecute() override;

        FarLightBarChecker FarChecker;
        MediumLightBarChecker MediumChecker;
        NearLightBarChecker NearChecker;
    };
}
