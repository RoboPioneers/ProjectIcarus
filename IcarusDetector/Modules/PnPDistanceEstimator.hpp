#pragma once

#include <opencv2/opencv.hpp>

namespace Gaia::Modules
{
    /// Distance estimator that uses PnP.
    class PnPDistanceEstimator
    {
    public:
        std::vector<cv::Point3d> WorldPoints;
        cv::Mat CameraMatrix;
        cv::Mat DistortionCoefficient;

        /// Get estimated distance according to the camera points.
        [[nodiscard]] float GetDistance(const std::vector<cv::Point2f>& camera_points) const;
    };
}