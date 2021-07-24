#include "DetectingR.hpp"

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
    }

    /// Initialize blackboard values.
    void DetectingR::OnInitialize()
    {
        InitializeFacilities();
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
        if (PreviousRFound && *Panel)
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

        cv::Mat gray_picture, match_scores;
        gray_picture = (*Picture)(search_area);
        cv::cvtColor((*Picture)(search_area), gray_picture, cv::COLOR_BGR2GRAY);

        cv::matchTemplate(gray_picture, TemplatePicture, match_scores, cv::TM_CCORR_NORMED);
        double min_value = 0.0, max_value = 0.0;
        cv::Point min_position, max_position;
        cv::minMaxLoc(match_scores, &min_value, &max_value, &min_position, &max_position);

        R->x = max_position.x + search_area.x + R->width / 2;
        R->y = max_position.y + search_area.y + R->height / 2;

        DEBUG_BEGIN
        GetInspector()->UpdateValue("R", std::to_string(R->x + R->width / 2) + ","
                                         + std::to_string(R->y + R->height / 2));
        GetInspector()->UpdateValue("R_similarity", std::to_string(max_value));
        DEBUG_END

        if (max_value < 0.6)
        {
            GetLogger()->RecordMessage("Can not found R, similarity " + std::to_string(max_value));
            PreviousRFound = false;
            return BehaviorTree::Result::Failure;
        }

        PreviousRFound = true;

        DEBUG_BEGIN
        cv::Mat display_r = Picture->clone();
        cv::rectangle(display_r, *R, cv::Scalar(0, 255, 0), 2);
        cv::resize(display_r, display_r, display_r.size() / 2);
        RWriter->Write(display_r);
        DEBUG_END

        return BehaviorTree::Result::Success;
    }


}