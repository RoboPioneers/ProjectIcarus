#include "DetectingPanel.hpp"

#include "../Modules/GeometryFeature.hpp"

namespace Icarus
{
    /// Load contour filtering configurations.
    void DetectingPanel::LoadConfigurations()
    {
        Threshold = GetConfigurator()->Get<int>("Threshold").value_or(30);
        MinWidth = GetConfigurator()->Get<int>("MinWidth").value_or(30);
        MaxWidth = GetConfigurator()->Get<int>("MaxWidth").value_or(45);
        MinLength = GetConfigurator()->Get<int>("MinLength").value_or(45);
        MaxLength = GetConfigurator()->Get<int>("MaxLength").value_or(70);
        MaxArmLength = GetConfigurator()->Get<int>("MaxArmLength").value_or(200);
        MinLengthWidthRatio = GetConfigurator()->Get<double>("MinLengthWidthRatio").value_or(1.2);
        MaxLengthWidthRatio = GetConfigurator()->Get<double>("MaxLengthWidthRatio").value_or(2.0);
    }

    /// Initialize blackboard values.
    void DetectingPanel::OnInitialize()
    {
        InitializeFacilities();

        MaskWriter = std::make_unique<Gaia::SharedPicture::PictureWriter>("icarus.challenger.mask",
                                                                          1920 * 1080);
        Picture = GetBlackboard()->GetPointer<cv::Mat>("MainPicture");
        Panel = GetBlackboard()->GetPointer<std::optional<cv::RotatedRect>>("Panel");
        R = GetBlackboard()->GetPointer<std::optional<cv::Rect>>("R");

        LoadConfigurations();
    }

    /// Filtering all possible contour elements.
    BehaviorTree::Result DetectingPanel::OnExecute()
    {
        DEBUG_BEGIN
        CheckReloadConfiguration();
        DEBUG_END

        if (!R->has_value()) return BehaviorTree::Result::Failure;

        cv::Mat mask;
        cv::Mat gray_picture;

        cv::cvtColor(*Picture, gray_picture, cv::COLOR_BGR2GRAY);
        cv::threshold(gray_picture, mask, Threshold, 255, cv::THRESH_BINARY);
        cv::dilate(mask, mask,
                   cv::getStructuringElement(cv::MORPH_DILATE, cv::Size(3,3)));
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(mask, contours, hierarchy,
                         cv::RETR_TREE,cv::CHAIN_APPROX_SIMPLE);

        DEBUG_BEGIN
        cv::Mat mask_display;
        cv::resize(mask, mask_display, cv::Size(mask.cols / 2, mask.rows / 2));
        MaskWriter->Write(mask_display);
        DEBUG_END

        std::optional<cv::RotatedRect> target_panel = std::nullopt;

        tbb::parallel_for(0, static_cast<int>(contours.size()),
        [this, &target_panel, &contours, &hierarchy, picture = Picture, &mask, &gray_picture](int index){
            if (contours[index].size() <= 3) return;
            auto rotated_rectangle = cv::minAreaRect(contours[index]);
            auto candidate_geometry_feature = Modules::GeometryFeature::Standardize(rotated_rectangle);
            auto candidate_length_height_ratio = candidate_geometry_feature.Length / candidate_geometry_feature.Width;

            // Size check.
            if (candidate_length_height_ratio < MinLengthWidthRatio ||
                candidate_length_height_ratio > MaxLengthWidthRatio)
            {
                return;
            }

            if (candidate_geometry_feature.Length > MaxLength ||
                candidate_geometry_feature.Length < MinLength ||
                candidate_geometry_feature.Width > MaxWidth ||
                candidate_geometry_feature.Width < MinWidth )
            {
                return;
            }

            // R Check.
            auto r_center_x = R->value().x + R->value().width / 2;
            auto r_center_y = R->value().y + R->value().height / 2;
            auto target_center_x = rotated_rectangle.center.x;
            auto target_center_y = rotated_rectangle.center.y;
            auto delta_x = target_center_x - static_cast<float>(r_center_x);
            auto delta_y = target_center_y - static_cast<float>(r_center_y);
            // Check distance to R.
            auto distance = std::sqrt(delta_x * delta_x + delta_y * delta_y);
            if (distance > static_cast<float>(MaxArmLength) || distance < static_cast<float>(MaxLength))
            {
                return;
            }

            // Hierarchy check.
            struct relationship_description
            {
                int parent_index {-1};
                int children_index {-1};
                int next_index {-1};
                int previous_index {-1};
            }relationship;

            relationship.next_index = hierarchy[index][0];
            relationship.previous_index = hierarchy[index][1];
            relationship.children_index = hierarchy[index][2];
            relationship.parent_index = hierarchy[index][3];

            // Avoid miss detection of activated panel.
            if (relationship.parent_index == -1 || contours[relationship.parent_index].size() < 3 ||
                cv::contourArea(contours[relationship.parent_index]) > 6000)
            {
                return;
            }

            if (relationship.children_index != -1 && contours[relationship.children_index].size() > 3
                && cv::contourArea(contours[relationship.children_index]) >
                   rotated_rectangle.size.area() * 0.4)
            {
                return;
            }

            target_panel = rotated_rectangle;
        });

        if (target_panel)
        {
            *Panel = target_panel;
            return BehaviorTree::Result::Success;
        }
        else
        {
            *Panel = std::nullopt;
            GetLogger()->RecordMessage("Can not find panel.");
            return BehaviorTree::Result::Failure;
        }
    }
}