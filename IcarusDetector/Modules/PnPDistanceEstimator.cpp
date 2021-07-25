#include "PnPDistanceEstimator.hpp"

#include <cmath>

namespace Gaia::Modules
{
    /// Get estimated distance according to the camera points.
    float PnPDistanceEstimator::GetDistance(const std::vector<cv::Point2f>& camera_points) const
    {
        cv::Mat rotation_vector = cv::Mat::zeros(3, 1, CV_64FC1);
        cv::Mat translation_vector = cv::Mat::zeros(3, 1, CV_64FC1);
        cv::Mat_<float> rotation_matrix,translation_matrix;
        solvePnP(WorldPoints, camera_points,
                 CameraMatrix, DistortionCoefficient,
                 rotation_vector, translation_vector,
                 false, cv::SOLVEPNP_ITERATIVE);
        return std::sqrt(translation_vector.at<float>(0)*translation_vector.at<float>(0) +
                    translation_vector.at<float>(1)*translation_vector.at<float>(1) +
                    translation_vector.at<float>(2)*translation_vector.at<float>(2));;
    }
}