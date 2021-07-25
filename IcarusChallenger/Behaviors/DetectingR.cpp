#include "DetectingR.hpp"
#include "../Modules/RectangleTool.hpp"

namespace Icarus
{
    /// Load contour filtering configurations.
    void DetectingR::LoadConfigurations()
    {
        auto template_path = GetConfigurator()->Get("RTemplate").value_or("RTemplate.png");
        TemplatePicture = cv::imread(template_path);
        cv::cvtColor(TemplatePicture, TemplatePicture, cv::COLOR_BGR2GRAY);
        GetLogger()->RecordMessage("Template picture loaded from " + template_path);
        R->width = TemplatePicture.cols;
        R->height = TemplatePicture.rows;

        // Template size is 22.
        RMinLength = GetConfigurator()->Get<int>("RMinLength").value_or(11);
        RMaxLength = GetConfigurator()->Get<int>("RMaxLength").value_or(33);

        Threshold = GetConfigurator()->Get<int>("Threshold").value_or(30);
    }

    /// Initialize blackboard values.
    void DetectingR::OnInitialize()
    {
        InitializeFacilities();
        MaskWriter = std::make_unique<Gaia::SharedPicture::PictureWriter>("icarus.challenger.mask",
                                                                          1920 * 1080);
        RWriter = std::make_unique<Gaia::SharedPicture::PictureWriter>("icarus.challenger.r",
                                                                       1920 * 1080 * 3);
        R = GetBlackboard()->GetPointer<cv::Rect>("R");
        Panel = GetBlackboard()->GetPointer<std::optional<cv::RotatedRect>>("Panel", std::nullopt);
        Picture = GetBlackboard()->GetPointer<cv::Mat>("MainPicture");

        if (!Picture) return;
        LoadConfigurations();
    }

    /// Filtering all possible contour elements.
    BehaviorTree::Result DetectingR::OnExecute()
    {
        DEBUG_BEGIN
        CheckReloadConfiguration();
        DEBUG_END

        cv::Rect search_area;
        search_area.x = 0;
        search_area.y = 0;
        search_area.width = Picture->cols;
        search_area.height = Picture->rows;

        // Only use ROI when previous R and Panel found.
        if (PreviousRFound)
        {
            static constexpr int search_area_ratio = 10;
            search_area.width = TemplatePicture.cols * search_area_ratio;
            search_area.height = TemplatePicture.rows * search_area_ratio;
            search_area.x = R->x - (search_area.width - TemplatePicture.cols) / 2;
            search_area.y = R->y - (search_area.height - TemplatePicture.rows) / 2;

            if (search_area.x < 0) search_area.x = 0;
            if (search_area.x >= Picture->cols) search_area.x = Picture->cols - 100;
            if (search_area.y < 0) search_area.y = 0;
            if (search_area.y >= Picture->rows) search_area.y = Picture->rows - 100;
            if (search_area.x + search_area.width > Picture->cols)
                search_area.width = Picture->cols - search_area.x;
            if (search_area.y + search_area.height > Picture->rows)
                search_area.height = Picture->rows - search_area.y;
        }

        cv::Mat gray_picture, match_scores, mask;
        gray_picture = (*Picture)(search_area);
        cv::cvtColor((*Picture)(search_area), gray_picture, cv::COLOR_BGR2GRAY);
        cv::threshold(gray_picture, mask, Threshold, 255, cv::THRESH_BINARY);
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(mask, contours, hierarchy,
                         cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);

        std::optional<cv::Point> r_position = std::nullopt;
        std::optional<double> r_score = std::nullopt;

        for (auto const& contour : contours)
        {
            auto bounding_box = cv::boundingRect(contour);
            if (bounding_box.width > RMaxLength || bounding_box.height > RMaxLength ||
                bounding_box.width < RMinLength || bounding_box.height < RMinLength)
                continue;

            bounding_box = Modules::RectangleTool::GetScaledRectangle(
                    bounding_box,2.0, 2.0);
            bounding_box = Modules::RectangleTool::GetSafeRectangle(
                    bounding_box, search_area.size());
            cv::Mat match_score;
            cv::matchTemplate(gray_picture(bounding_box), TemplatePicture,
                              match_score, cv::TM_CCORR);
            double min_value = 0.0, max_value = 0.0;
            cv::Point min_position, max_position;
            cv::minMaxLoc(match_score, &min_value, &max_value, &min_position, &max_position);
            if (!r_score || *r_score < max_value)
            {
                r_position = max_position;
                r_score = max_value;
            }
        }

        R->x = r_position->x + search_area.x + R->width / 2;
        R->y = r_position->y + search_area.y + R->height / 2;

        DEBUG_BEGIN
        GetInspector()->UpdateValue("R", std::to_string(R->x + R->width / 2) + ","
                                         + std::to_string(R->y + R->height / 2));
        GetInspector()->UpdateValue("R_similarity", std::to_string(r_score.value()));
        DEBUG_END

        if (r_score.value() < 0.6)
        {
            GetLogger()->RecordMessage("Can not found R, similarity " + std::to_string(r_score.value()));
            PreviousRFound = false;
            return BehaviorTree::Result::Failure;
        }

        PreviousRFound = true;

        DEBUG_BEGIN
        cv::Mat r_display_picture = Picture->clone();
        cv::Rect display_rectangle = *R;
        display_rectangle.x -= display_rectangle.width / 2;
        display_rectangle.y -= display_rectangle.height / 2;
        display_rectangle = Modules::RectangleTool::GetSafeRectangle(display_rectangle, Picture->size());
        cv::rectangle(r_display_picture, display_rectangle, cv::Scalar(0, 255, 0), 2);
        cv::resize(r_display_picture, r_display_picture, r_display_picture.size() / 2);
        RWriter->Write(r_display_picture);
        DEBUG_END

        return BehaviorTree::Result::Success;
    }


}