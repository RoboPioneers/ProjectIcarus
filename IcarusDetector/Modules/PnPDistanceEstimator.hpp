#pragma once

#include <opencv2/opencv.hpp>

namespace Gaia::Modules
{
    /// Distance estimator that uses PnP.
    class PnPDistanceEstimator
    {
    public:
        std::vector<cv::Point3d> WorldPoints;
        cv::Mat_<double> CameraMatrix;
        cv::Mat_<double> DistortionCoefficient;

        /// Get estimated distance according to the camera points.
        [[nodiscard]] double GetDistance(const std::vector<cv::Point2d>& camera_points) const;
    };
}