#pragma once

#include <vector>
#include <opencv2/opencv.hpp>

namespace Gaia::Modules
{
    class PCATool
    {
    public:
        static double GetAngle(const std::vector<cv::Point>& contour);
    };
}