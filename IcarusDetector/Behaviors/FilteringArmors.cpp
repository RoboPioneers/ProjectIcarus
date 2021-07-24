#include "FilteringArmors.hpp"
#include "../Modules/ImageDebugUtility.hpp"
#include "../Components/PONElement.hpp"

namespace Icarus
{

    void FilteringArmors::LoadConfigurations()
    {

    }

    void FilteringArmors::OnInitialize()
    {
        InitializeFacilities();

        ArmorsWriter = std::make_unique<SharedPicture::PictureWriter>("icarus.detector.armors", 1920 * 1080 * 3);

        MainPicture = GetBlackboard()->GetPointer<cv::Mat>("MainPicture");
        Contours = GetBlackboard()->GetPointer<
                tbb::concurrent_vector<ContourElement::Pointer>>("Contours");
        LightBarFarLayer = GetBlackboard()->GetPointer<Tags::Index>(
                "LightBarFarLayer");
        LightBarMediumLayer = GetBlackboard()->GetPointer<Tags::Index>(
                "LightBarMediumLayer");
        LightBarNearLayer = GetBlackboard()->GetPointer<Tags::Index>(
                "LightBarNearLayer");
        FoundTarget = GetBlackboard()->GetPointer<std::optional<cv::RotatedRect>>("FoundTarget", std::nullopt);
        HitPoint = GetBlackboard()->GetPointer<cv::Point2i>("HitPoint");
        HitCommand = GetBlackboard()->GetPointer<int>("HitCommand");
        FarChecker.Initialize(GetConfigurator());
        MediumChecker.Initialize(GetConfigurator());
        NearChecker.Initialize(GetConfigurator());

        LoadConfigurations();
    }

    BehaviorTree::Result FilteringArmors::OnExecute()
    {
        DEBUG_BEGIN
            CheckReloadConfiguration();
        DEBUG_END

        HitPoint->x = 0;
        HitPoint->y = 0;
        *HitCommand = 0;
        *FoundTarget = std::nullopt;

        std::vector<tbb::concurrent_vector<ContourElement*>> light_bars;
        light_bars.resize(3);
        auto layers = std::vector{LightBarFarLayer, LightBarMediumLayer, LightBarNearLayer};
        tbb::parallel_for(0ul, layers.size(),
                          [&light_bars, &layers](unsigned long layer_index){
            tbb::parallel_for_each(layers[layer_index]->GetHolders(), [&light_bars, &layer_index]
            (Tags::Holder* holder){
                light_bars[layer_index].push_back(dynamic_cast<ContourElement*>(holder));
            });
        });

        std::vector<tbb::concurrent_vector<PONElement::Pointer>> possible_armors;
        possible_armors.resize(3);
        tbb::parallel_for(0ul, layers.size(),
                          [this, &light_bars, &possible_armors](unsigned long layer_index){
            auto& layer = light_bars[layer_index];
            tbb::parallel_for(0ul, layer.size(),
                              [this, layer_index, &layer, &possible_armors]
            (unsigned long main_contour_index)
            {
                auto* main_contour = layer[main_contour_index];
                for (auto vice_contour_index = main_contour_index + 1; vice_contour_index < layer.size();
                    ++vice_contour_index)
                {
                    auto* vice_contour = layer[vice_contour_index];

                    auto PON_element = std::make_shared<PONElement>();
                    PON_element->ContourA = main_contour;
                    PON_element->ContourB = vice_contour;
                    std::vector<cv::Point> points;
                    points.reserve(main_contour->Points.size() + vice_contour->Points.size());
                    points.insert(points.end(), main_contour->Points.begin(), main_contour->Points.end());
                    points.insert(points.end(), vice_contour->Points.begin(), vice_contour->Points.end());
                    PON_element->Rectangle = cv::minAreaRect(points);
                    PON_element->Feature = Modules::GeometryFeature::Standardize(PON_element->Rectangle);

                    switch (layer_index)
                    {
                        case 0:
                            if (this->FarChecker.Check(PON_element.get()))
                            {
                                possible_armors[layer_index].push_back(PON_element);
                            }
                            break;
                        case 1:
                            if (this->MediumChecker.Check(PON_element.get()))
                            {
                                possible_armors[layer_index].push_back(PON_element);
                            }
                            break;
                        case 2:
                            if (this->NearChecker.Check(PON_element.get()))
                            {
                                possible_armors[layer_index].push_back(PON_element);
                            }
                            break;
                        default:
                            break;
                    }

                }
            });
        });

        std::optional<double> best_score = std::nullopt;
        std::optional<cv::RotatedRect> best_armor = std::nullopt;

        cv::Point2f screen_center;
        screen_center.x = static_cast<float>(MainPicture->cols) / 2;
        screen_center.y = static_cast<float>(MainPicture->rows) / 2;
        for (const auto& armor_layer : possible_armors)
        {
            for (const auto& armor : armor_layer)
            {
                auto delta_position = armor->Rectangle.center - screen_center;
                auto current_score = delta_position.ddot(delta_position);
                if (!best_score || current_score < *best_score)
                {
                    best_score = current_score;
                    best_armor = armor->Rectangle;
                }
            }
        }
        *FoundTarget = best_armor;

        if (*FoundTarget)
        {
            *HitCommand = 1;
            HitPoint->x = static_cast<int>(best_armor->center.x) - static_cast<int>(MainPicture->cols / 2.0);
            HitPoint->y = static_cast<int>(MainPicture->rows / 2.0) -  static_cast<int>(best_armor->center.y);
        }

        DEBUG_BEGIN
            cv::Mat armors_picture = MainPicture->clone();
            for (const auto& armor : possible_armors[0])
            {
                Modules::ImageDebugUtility::DrawRotatedRectangle(armors_picture, armor->Rectangle,
                                                                 cv::Scalar(0, 255, 0), 2);
            }
            for (const auto& armor : possible_armors[1])
            {
                Modules::ImageDebugUtility::DrawRotatedRectangle(armors_picture, armor->Rectangle,
                                                                 cv::Scalar(0, 255, 0), 2);
            }
            for (const auto& armor : possible_armors[2])
            {
                Modules::ImageDebugUtility::DrawRotatedRectangle(armors_picture, armor->Rectangle,
                                                                 cv::Scalar(0, 255, 0), 2);
            }
            cv::resize(armors_picture, armors_picture, armors_picture.size() / 2);
            ArmorsWriter->Write(armors_picture);
        DEBUG_END

        return BehaviorTree::Result::Success;
    }
}