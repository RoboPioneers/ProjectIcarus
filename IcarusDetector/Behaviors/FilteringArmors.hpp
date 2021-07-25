#pragma once

#include "../Framework/ProcessorBase.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/cudafilters.hpp>
#include <GaiaTags/GaiaTags.hpp>
#include <GaiaSharedPicture/GaiaSharedPicture.hpp>
#include <tbb/tbb.h>
#include "../Components/ContourElement.hpp"
#include "../Checkers/FarArmorChecker.hpp"
#include "../Checkers/MediumArmorChecker.hpp"
#include "../Checkers/NearArmorChecker.hpp"
#include "../Modules/PnPDistanceEstimator.hpp"

namespace Icarus
{
    class FilteringArmors : public ProcessorBase
    {
        REFLECT_TYPE(Icarus, ProcessorBase)

    private:
        std::unique_ptr<SharedPicture::PictureWriter> ArmorsWriter;
        std::unique_ptr<Modules::PnPDistanceEstimator> BigArmorDistanceEstimator;
        std::unique_ptr<Modules::PnPDistanceEstimator> SmallArmorDistanceEstimator;
        /// The length-width ratio to separate big armor and small armor, is 8.0 by default.
        double ArmorSizeRatioSeparator;
    protected:

        unsigned int ArmorMaxLeaningAngle;

        /// Main picture in the blackboard.
        cv::Mat* MainPicture {nullptr};
        /// Detected contours.
        tbb::concurrent_vector<ContourElement::Pointer>* Contours;

        Tags::Index* LightBarLayer;
        Tags::Index* LightBarFarLayer;
        Tags::Index* LightBarMediumLayer;
        Tags::Index* LightBarNearLayer;

        cv::Point2i* HitPoint {nullptr};
        int* HitCommand {nullptr};
        double* HitDistance {nullptr};
        int* MotionStatus {nullptr};

        std::optional<cv::RotatedRect>* FoundTarget;

        /// Load contour filtering configurations.
        void LoadConfigurations() override;

        /// Initialize blackboard values.
        void OnInitialize() override;

        /// Filtering all possible contour elements.
        BehaviorTree::Result OnExecute() override;

        FarArmorChecker FarChecker;
        MediumArmorChecker MediumChecker;
        NearArmorChecker NearChecker;
    };
}
