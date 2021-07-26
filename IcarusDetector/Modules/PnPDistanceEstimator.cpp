#include "PnPDistanceEstimator.hpp"

#include <cmath>

namespace Gaia::Modules
{
    /// Get estimated distance according to the camera points.
    double PnPDistanceEstimator::GetDistance(const std::vector<cv::Point2d>& camera_points) const
    {
        cv::Mat rotation_vector = cv::Mat::zeros(3, 1, CV_64FC1);
        cv::Mat translation_vector = cv::Mat::zeros(3, 1, CV_64FC1);
        solvePnP(WorldPoints, camera_points,
                 CameraMatrix, DistortionCoefficient,
                 rotation_vector, translation_vector,
                 false, cv::SOLVEPNP_ITERATIVE);
        return std::sqrt(translation_vector.at<double>(0)*translation_vector.at<double>(0) +
                         translation_vector.at<double>(1)*translation_vector.at<double>(1) +
                         translation_vector.at<double>(2)*translation_vector.at<double>(2));
    }
}