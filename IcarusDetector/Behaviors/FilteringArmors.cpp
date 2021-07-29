#include "FilteringArmors.hpp"
#include "../Modules/ImageDebugUtility.hpp"
#include "../Components/PONElement.hpp"
#include <boost/algorithm/string.hpp>

namespace Icarus
{
    std::vector<double> CastStringToDoubleVector(const std::string& text)
    {
        std::vector<std::string> element_texts;
        boost::split(element_texts, text, boost::is_any_of(",;"), boost::token_compress_on);
        std::vector<double> element_values;
        element_values.reserve(element_texts.size());
        for (const auto& value_text : element_texts)
        {
            element_values.push_back(std::stod(value_text));
        }
        return element_values;
    }

    void FilteringArmors::LoadConfigurations()
    {
        FarChecker.LoadConfiguration();
        MediumChecker.LoadConfiguration();
        NearChecker.LoadConfiguration();
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
        HitDistance = GetBlackboard()->GetPointer<double>("HitDistance", 0);
        MotionStatus = GetBlackboard()->GetPointer<int>("MotionStatus", 0);
        InterestedAreaUsed = GetBlackboard()->GetPointer<bool>("InterestedAreaUsed", false);
        FarChecker.Initialize(GetConfigurator());
        MediumChecker.Initialize(GetConfigurator());
        NearChecker.Initialize(GetConfigurator());

        auto camera_matrix_string = GetConfigurator()->Get("CameraMatrix");
        auto distortion_coefficient_string = GetConfigurator()->Get("DistortionCoefficient");
        ArmorSizeRatioSeparator = GetConfigurator()->Get<double>("ArmorSizeRatioSeparator").value_or(8.0);
        if (camera_matrix_string.has_value() && distortion_coefficient_string.has_value())
        {
            auto camera_matrix_vector = CastStringToDoubleVector(camera_matrix_string.value());
            auto distortion_coefficient_vector = CastStringToDoubleVector(
                    distortion_coefficient_string.value());

            if (camera_matrix_vector.size() == 9 && distortion_coefficient_vector.size() == 4)
            {
                auto camera_matrix = cv::Mat(camera_matrix_vector, true);
                camera_matrix = camera_matrix.reshape(0, 3);

                auto distortion_coefficient = cv::Mat(distortion_coefficient_vector, true);
                distortion_coefficient = distortion_coefficient.reshape(0, 1);

                static constexpr double BigArmorHalfLength = 23.0 / 2.0;
                static constexpr double SmallArmorHalfLength = 13.0 / 2.0;
                static constexpr double ArmorHalfHeight = 5.5 / 2.0;
                BigArmorDistanceEstimator = std::make_unique<Modules::PnPDistanceEstimator>();
                BigArmorDistanceEstimator->CameraMatrix = camera_matrix;
                BigArmorDistanceEstimator->DistortionCoefficient = distortion_coefficient;
                BigArmorDistanceEstimator->WorldPoints = {
                        cv::Point3d(-BigArmorHalfLength, -ArmorHalfHeight, 0),
                        cv::Point3d(-BigArmorHalfLength, ArmorHalfHeight, 0),
                        cv::Point3d(BigArmorHalfLength, ArmorHalfHeight, 0),
                        cv::Point3d(BigArmorHalfLength, -ArmorHalfHeight, 0)};
                SmallArmorDistanceEstimator = std::make_unique<Modules::PnPDistanceEstimator>();
                SmallArmorDistanceEstimator->CameraMatrix = camera_matrix;
                SmallArmorDistanceEstimator->DistortionCoefficient = distortion_coefficient;
                SmallArmorDistanceEstimator->WorldPoints = {
                        cv::Point3d(-SmallArmorHalfLength, -ArmorHalfHeight, 0),
                        cv::Point3d(-SmallArmorHalfLength, ArmorHalfHeight, 0),
                        cv::Point3d(SmallArmorHalfLength, ArmorHalfHeight, 0),
                        cv::Point3d(SmallArmorHalfLength, -ArmorHalfHeight, 0)};
            }
        }

        LoadConfigurations();
    }

    BehaviorTree::Result FilteringArmors::OnExecute()
    {
        DEBUG_BEGIN
            CheckReloadConfiguration();
        DEBUG_END

        *HitCommand = 0;
        if (!*InterestedAreaUsed)
        {
            *HitDistance = 0;
            HitPoint->x = 0;
            HitPoint->y = 0;
        }
        *MotionStatus = 0;
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

        // The 'center' position to lock.
        cv::Point2f locking_center;
        locking_center.x = static_cast<float>(MainPicture->cols) * 0.5f;
        locking_center.y = static_cast<float>(MainPicture->rows) * 0.618f;
        for (const auto& armor_layer : possible_armors)
        {
            for (const auto& armor : armor_layer)
            {
                auto delta_position = armor->Rectangle.center - locking_center;
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

            if (BigArmorDistanceEstimator && SmallArmorDistanceEstimator)
            {
                auto best_armor_feature = Modules::GeometryFeature::Standardize(*best_armor);
                std::vector<cv::Point2d> camera_points;
                if (best_armor_feature.Angle > 90)
                {
                    camera_points.emplace_back(static_cast<cv::Vec2d>(
                            static_cast<cv::Point_<double>>(best_armor_feature.Center)) +
                            static_cast<cv::Vec2d>(best_armor_feature.Vectors.ClockwiseDiagonal));
                    camera_points.emplace_back(static_cast<cv::Vec2d>(
                            static_cast<cv::Point_<double>>(best_armor_feature.Center)) +
                            static_cast<cv::Vec2d>(best_armor_feature.Vectors.AnticlockwiseDiagonal));
                    camera_points.emplace_back(static_cast<cv::Vec2d>(
                            static_cast<cv::Point_<double>>(best_armor_feature.Center)) -
                            static_cast<cv::Vec2d>(best_armor_feature.Vectors.ClockwiseDiagonal));
                    camera_points.emplace_back(static_cast<cv::Vec2d>(
                            static_cast<cv::Point_<double>>(best_armor_feature.Center)) -
                            static_cast<cv::Vec2d>(best_armor_feature.Vectors.AnticlockwiseDiagonal));
                }
                else
                {
                    camera_points.emplace_back(static_cast<cv::Vec2d>(
                            static_cast<cv::Point_<double>>(best_armor_feature.Center)) -
                            static_cast<cv::Vec2d>(best_armor_feature.Vectors.ClockwiseDiagonal));
                    camera_points.emplace_back(static_cast<cv::Vec2d>(
                            static_cast<cv::Point_<double>>(best_armor_feature.Center)) -
                            static_cast<cv::Vec2d>(best_armor_feature.Vectors.AnticlockwiseDiagonal));
                    camera_points.emplace_back(static_cast<cv::Vec2d>(
                            static_cast<cv::Point_<double>>(best_armor_feature.Center)) +
                            static_cast<cv::Vec2d>(best_armor_feature.Vectors.ClockwiseDiagonal));
                    camera_points.emplace_back(static_cast<cv::Vec2d>(
                            static_cast<cv::Point_<double>>(best_armor_feature.Center)) +
                            static_cast<cv::Vec2d>(best_armor_feature.Vectors.AnticlockwiseDiagonal));
                }
                if (best_armor_feature.Length / best_armor_feature.Width > ArmorSizeRatioSeparator)
                {
                    *HitDistance = BigArmorDistanceEstimator->GetDistance(camera_points);
                }
                else
                {
                    *HitDistance = SmallArmorDistanceEstimator->GetDistance(camera_points);
                }
            }
        }

        DEBUG_BEGIN

            cv::Mat armors_picture = MainPicture->clone();
            for (const auto& armor : possible_armors[0])
            {
                Modules::ImageDebugUtility::DrawRotatedRectangle(armors_picture, armor->Rectangle,
                                                                 cv::Scalar(0, 0, 255), 2);
            }
            for (const auto& armor : possible_armors[1])
            {
                Modules::ImageDebugUtility::DrawRotatedRectangle(armors_picture, armor->Rectangle,
                                                                 cv::Scalar(0, 255, 0), 2);
            }
            for (const auto& armor : possible_armors[2])
            {
                Modules::ImageDebugUtility::DrawRotatedRectangle(armors_picture, armor->Rectangle,
                                                                 cv::Scalar(255, 0, 0), 2);
            }
            cv::resize(armors_picture, armors_picture, armors_picture.size() / 2);
            ArmorsWriter->Write(armors_picture);
        DEBUG_END

        return BehaviorTree::Result::Success;
    }
}