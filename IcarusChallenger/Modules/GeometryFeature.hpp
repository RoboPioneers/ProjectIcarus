#pragma once

#include <opencv2/opencv.hpp>

namespace Gaia::Modules
{
    class GeometryFeature
    {
    public:
        double Angle {0.0f};
        double Width {0.0f};
        double Length {0.0f};

        struct DirectionalVector
        {
            cv::Vec2f Direction;

            cv::Vec2f ClockwiseDiagonal;
            cv::Vec2f AnticlockwiseDiagonal;
        }Vectors;

        cv::Point2i Center;

        static GeometryFeature Standardize(const cv::RotatedRect& rectangle);

        static bool IsIdentical(const GeometryFeature& a, const GeometryFeature& b);
    };
}
