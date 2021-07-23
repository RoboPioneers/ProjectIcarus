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

        MainPicture = GetBlackboard()->GetObject<cv::Mat>("MainPicture");
        InterestedArea = GetBlackboard()->GetObject<cv::Rect>(
                "InterestedArea", cv::Rect());
        FoundTarget = GetBlackboard()->GetObject<std::optional<cv::RotatedRect>>("FoundTarget", std::nullopt);

        LoadConfigurations();
    }

    BehaviorTree::Result FilteringArea::OnExecute()
    {
        cv::Rect main_rectangle;
        // Default: full screen ROI.
        main_rectangle.x = 0;
        main_rectangle.y = 0;
        main_rectangle.width = MainPicture->cols;
        main_rectangle.height = MainPicture->rows;

        auto current_time = std::chrono::steady_clock::now();

//        if (! (*FoundTarget))
//        {
//            // Armor not found.
//            if (PreviousInterestedArea)
//            {
//                // Memorized ROI exists.
//                if (current_time - *PreviousInterestedAreaTimestamp <
//                    std::chrono::milliseconds(InterestedAreaToleranceLosingTime))
//                {
//                    // Not expired, use the old ROI.
//                    main_rectangle = *PreviousInterestedArea;
//                }
//                else
//                {
//                    // Reset memorized ROI.
//                    PreviousInterestedArea = std::nullopt;
//                    PreviousInterestedAreaTimestamp = std::nullopt;
//                }
//            }
//        }
//        else
//        {
//            // Found target, calculate new ROI.
//            double width_inflate_ratio = 6;
//            double height_inflate_ratio = 4;
//            main_rectangle = FoundTarget->value().boundingRect();
//            main_rectangle.x -= static_cast<int>(main_rectangle.width * width_inflate_ratio * 0.5);
//            main_rectangle.y -= static_cast<int>(main_rectangle.height * height_inflate_ratio * 0.5);
//            main_rectangle.width += static_cast<int>(main_rectangle.width * width_inflate_ratio);
//            main_rectangle.height += static_cast<int>(main_rectangle.height * height_inflate_ratio);
//            main_rectangle = Modules::RectangleTool::GetSafeRectangle(
//                    main_rectangle,cv::Size(MainPicture->cols, MainPicture->rows));
//
//            PreviousInterestedArea = main_rectangle;
//            PreviousInterestedAreaTimestamp = current_time;
//        }

        main_rectangle = Modules::RectangleTool::GetSafeRectangle(
                main_rectangle,cv::Size(MainPicture->cols, MainPicture->rows));

        *InterestedArea = main_rectangle;

        return Gaia::BehaviorTree::Result::Success;
    }
}