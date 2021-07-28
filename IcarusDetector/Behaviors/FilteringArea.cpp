#include "FilteringArea.hpp"
#include "../Modules/RectangleTool.hpp"

namespace Icarus
{

    void FilteringArea::LoadConfigurations()
    {
        InterestedAreaToleranceLosingTime = GetConfigurator()->Get<int>("InterestedAreaToleranceLosingTime")
                .value_or(300);
    }

    void FilteringArea::OnInitialize()
    {
        InitializeFacilities();

        MainPicture = GetBlackboard()->GetPointer<cv::Mat>("MainPicture");
        InterestedArea = GetBlackboard()->GetPointer<cv::Rect>("InterestedArea");
        FoundTarget = GetBlackboard()->GetPointer<std::optional<cv::RotatedRect>>("FoundTarget", std::nullopt);

        LoadConfigurations();
    }

    BehaviorTree::Result FilteringArea::OnExecute()
    {
        cv::Rect main_rectangle;

        auto current_time = std::chrono::steady_clock::now();

        bool interested_area_used = false;

        if (*FoundTarget)
        {
            // Found target, calculate new ROI.
            double width_inflate_ratio = 4;
            double height_inflate_ratio = 4;
            main_rectangle = FoundTarget->value().boundingRect();
            main_rectangle.x -= static_cast<int>(main_rectangle.width * width_inflate_ratio * 0.5);
            main_rectangle.y -= static_cast<int>(main_rectangle.height * height_inflate_ratio * 0.5);
            main_rectangle.width += static_cast<int>(main_rectangle.width * width_inflate_ratio);
            main_rectangle.height += static_cast<int>(main_rectangle.height * height_inflate_ratio);
            main_rectangle = Modules::RectangleTool::GetSafeRectangle(
                    main_rectangle,cv::Size(MainPicture->cols, MainPicture->rows));

            PreviousInterestedArea = main_rectangle;
            PreviousInterestedAreaTimestamp = current_time;
            interested_area_used = true;
        }
        else
        {
            // Armor not found.
            if (PreviousInterestedArea)
            {
                // Memorized ROI exists.
                if (current_time - *PreviousInterestedAreaTimestamp <
                    std::chrono::milliseconds(InterestedAreaToleranceLosingTime))
                {
                    // Not expired, use the old ROI.
                    main_rectangle = *PreviousInterestedArea;
                    interested_area_used = true;
                }
                else
                {
                    // Reset memorized ROI.
                    PreviousInterestedArea = std::nullopt;
                    PreviousInterestedAreaTimestamp = std::nullopt;
                    InterestedAreaRefreshTimestamp = current_time;
                }
            }
            else
            {
                InterestedAreaRefreshTimestamp = current_time;
            }
        }

        if (!interested_area_used ||
            (current_time - InterestedAreaRefreshTimestamp) > std::chrono::seconds(1))
        {
            PreviousInterestedArea = std::nullopt;
            PreviousInterestedAreaTimestamp = std::nullopt;
            InterestedAreaRefreshTimestamp = current_time;

            main_rectangle.x = 0;
            main_rectangle.y = 0;
            main_rectangle.width = MainPicture->cols;
            main_rectangle.height = MainPicture->rows;
        }

        main_rectangle = Modules::RectangleTool::GetSafeRectangle(
                main_rectangle,cv::Size(MainPicture->cols, MainPicture->rows));

        #ifdef OFFLINE
        main_rectangle.x = 0;
        main_rectangle.y = 0;
        main_rectangle.width = MainPicture->cols;
        main_rectangle.height = MainPicture->rows;
        #endif

        *InterestedArea = main_rectangle;

        return Gaia::BehaviorTree::Result::Success;
    }
}