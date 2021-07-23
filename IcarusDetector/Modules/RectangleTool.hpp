#pragma once

#include <opencv2/opencv.hpp>

namespace Gaia::Modules
{
    class RectangleTool
    {
    public:
        /// Get safe rectangle insides the max size.
        static cv::Rect GetSafeRectangle(const cv::Rect& rectangle, const cv::Size& max_size);
        /// Get safe scaled rectangle.
        static cv::Rect GetScaledRectangle(const cv::Rect& rectangle, double width_scale, double height_scale);

        struct ContourRelationship
        {
            int ParentIndex {-1};
            int ChildrenIndex {-1};
            int NextIndex {-1};
            int PreviousIndex {-1};
        };
        /// Translate the relationship vector into human readable relationship structure.
        static ContourRelationship TranslateContourRelationship(cv::Vec4i relationship);
    };
}