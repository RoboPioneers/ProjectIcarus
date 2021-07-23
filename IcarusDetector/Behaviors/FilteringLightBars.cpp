#include "FilteringLightBars.hpp"

#include "../Modules/ImageDebugUtility.hpp"

namespace Icarus
{

    void FilteringLightBars::LoadConfigurations()
    {
        LightBarNearMediumSeparator = GetConfigurator()->Get<unsigned int>("LightBarNearMediumSeparator")
                .value_or(35);
        LightBarMediumFarSeparator = GetConfigurator()->Get<unsigned int>("LightBarNearMediumSeparator")
                .value_or(10);
        LightBarMaxLeaningAngle = GetConfigurator()->Get<unsigned int>("LightBarMaxLeaningAngle")
                .value_or(60);
    }

    void FilteringLightBars::OnInitialize()
    {
        InitializeFacilities();

        ContoursWriter = std::make_unique<SharedPicture::PictureWriter>("icarus.contours", 1920 * 1080 * 3);

        MainPicture = GetBlackboard()->GetObject<cv::Mat>("MainPicture");
        Contours = GetBlackboard()->GetObject<
                tbb::concurrent_vector<ContourElement::Pointer>>("Contours");
        LightBarLayer = GetBlackboard()->GetObject<Tags::Layer>(
                "LightBarLayer");
        LightBarFarLayer = GetBlackboard()->GetObject<Tags::Layer>(
                "LightBarFarLayer");
        LightBarMediumLayer = GetBlackboard()->GetObject<Tags::Layer>(
                "LightBarMediumLayer");
        LightBarNearLayer = GetBlackboard()->GetObject<Tags::Layer>(
                "LightBarNearLayer");

        FarChecker.Initialize(GetConfigurator());
        MediumChecker.Initialize(GetConfigurator());
        NearChecker.Initialize(GetConfigurator());

        LoadConfigurations();
    }

    BehaviorTree::Result FilteringLightBars::OnExecute()
    {
        DEBUG_BEGIN
        CheckReloadConfiguration();
        DEBUG_END

        this->LightBarFarLayer->ClearHolders();
        this->LightBarMediumLayer->ClearHolders();
        this->LightBarMediumLayer->ClearHolders();

        FarChecker.MaxLightBarLeaningAngle = LightBarMaxLeaningAngle;
        FarChecker.MinLength = 0;
        FarChecker.MaxLength = static_cast<int>(MainPicture->rows * LightBarMediumFarSeparator);
        MediumChecker.MaxLightBarLeaningAngle = LightBarMaxLeaningAngle;
        MediumChecker.MinLength = static_cast<int>(MainPicture->rows * LightBarMediumFarSeparator);
        MediumChecker.MaxLength = static_cast<int>(MainPicture->rows * LightBarNearMediumSeparator);
        NearChecker.MaxLightBarLeaningAngle = LightBarMaxLeaningAngle;
        NearChecker.MinLength = static_cast<int>(MainPicture->rows * LightBarNearMediumSeparator);
        NearChecker.MaxLength = static_cast<int>(MainPicture->rows * LightBarNearMediumSeparator);
        for (auto& contour_element : *Contours)
        {
            if (FarChecker.Check(contour_element.get()))
            {
                LightBarLayer->AddHolder(contour_element.get());
                LightBarFarLayer->AddHolder(contour_element.get());
            }
            else if (MediumChecker.Check(contour_element.get()))
            {
                LightBarLayer->AddHolder(contour_element.get());
                LightBarMediumLayer->AddHolder(contour_element.get());
            }
            else if (NearChecker.Check(contour_element.get()))
            {
                LightBarLayer->AddHolder(contour_element.get());
                LightBarNearLayer->AddHolder(contour_element.get());
            }
        }

        DEBUG_BEGIN

            cv::Mat contours_picture = MainPicture->clone();

            for (auto* contour : this->LightBarFarLayer->GetHolders())
            {
                auto* element = dynamic_cast<ContourElement*>(contour);
                if(!element)
                    continue;
                Modules::ImageDebugUtility::DrawRotatedRectangle(
                        contours_picture, (element)->Rectangle,
                        cv::Scalar(0, 0, 255));
            }
            for (auto* contour : this->LightBarMediumLayer->GetHolders())
            {
                auto* element = dynamic_cast<ContourElement*>(contour);
                if(!element)
                    continue;
                Modules::ImageDebugUtility::DrawRotatedRectangle(
                        contours_picture, (element)->Rectangle,
                        cv::Scalar(0, 255, 0));
            }
            for (auto* contour : this->LightBarNearLayer->GetHolders())
            {
                auto* element = dynamic_cast<ContourElement*>(contour);
                if(!element)
                    continue;
                Modules::ImageDebugUtility::DrawRotatedRectangle(
                        contours_picture, (element)->Rectangle,
                        cv::Scalar(255, 0, 0));
            }
            cv::resize(contours_picture, contours_picture, contours_picture.size() / 2);
            ContoursWriter->Write(contours_picture);
        DEBUG_END

        return BehaviorTree::Result::Success;
    }
}