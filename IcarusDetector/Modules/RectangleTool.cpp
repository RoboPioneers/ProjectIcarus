#include "RectangleTool.hpp"

namespace Gaia::Modules
{
    /// Get safe rectangle insides the max size.
    cv::Rect RectangleTool::GetSafeRectangle(const cv::Rect &rectangle, const cv::Size& max_size)
    {
        auto safe_rectangle = rectangle;
        if (safe_rectangle.x < 0) safe_rectangle.x = 0;
        else if (safe_rectangle.x > max_size.width) safe_rectangle.x = max_size.width;
        if (safe_rectangle.y < 0) safe_rectangle.y = 0;
        else if (safe_rectangle.y > max_size.height) safe_rectangle.y = max_size.height;

        if (safe_rectangle.x + safe_rectangle.width > max_size.width)
            safe_rectangle.width = max_size.width - safe_rectangle.x;
        if (safe_rectangle.y + safe_rectangle.height > max_size.height)
            safe_rectangle.height = max_size.height - safe_rectangle.y;

        return safe_rectangle;
    }

    cv::Rect RectangleTool::GetScaledRectangle(const cv::Rect &rectangle, double width_scale, double height_scale)
    {
        auto scaled_rectangle = rectangle;
        scaled_rectangle.x -= scaled_rectangle.width * (width_scale - 1) / 2.0;
        scaled_rectangle.y -= scaled_rectangle.height * (height_scale - 1) / 2.0;
        scaled_rectangle.width = scaled_rectangle.width * width_scale;
        scaled_rectangle.height = scaled_rectangle.height * height_scale;
        return scaled_rectangle;
    }

    /// Translate the relationship vector into human readable relationship structure.
    RectangleTool::ContourRelationship RectangleTool::TranslateContourRelationship(cv::Vec4i relationship_vector)
    {
        ContourRelationship relationship;
        relationship.NextIndex = relationship_vector[0];
        relationship.PreviousIndex = relationship_vector[1];
        relationship.ChildrenIndex = relationship_vector[2];
        relationship.ParentIndex = relationship_vector[3];
        return relationship;
    }
}