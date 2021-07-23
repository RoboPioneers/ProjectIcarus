#include "PCATool.hpp"

namespace Gaia::Modules
{
    double PCATool::GetAngle(const std::vector<cv::Point>& contour)
    {
        int buffer_size = static_cast<int>(contour.size());
        auto data_points = cv::Mat(buffer_size, 2, CV_64FC1);
        for (int i = 0; i < data_points.rows; ++i)
        {
            data_points.at<double>(i, 0) = contour[i].x;
            data_points.at<double>(i, 1) = contour[i].y;
        }

        cv::PCA pca_analysis(data_points, cv::Mat(), cv::PCA::DATA_AS_ROW);
        std::vector<cv::Point2d> eigen_vectors(2);
        std::vector<double> eigen_values(2);
        for (int i = 0; i < 2; i++)
        {
            eigen_vectors[i] = cv::Point2d(pca_analysis.eigenvectors.at<double>(i, 0),
                                           pca_analysis.eigenvectors.at<double>(i, 1));
            eigen_values[i] = pca_analysis.eigenvalues.at<double>(i);
        }
        auto angle_degree = atan2(eigen_vectors[0].y, eigen_vectors[0].x) / 3.1415926535f * 180.0f;
        if (angle_degree > 0)
        {
            angle_degree = 180.0 - angle_degree;
        }
        else if (angle_degree < 0)
        {
            angle_degree = -angle_degree;
        }
        return angle_degree;
    }
}