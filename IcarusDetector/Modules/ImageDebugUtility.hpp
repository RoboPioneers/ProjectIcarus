#pragma once

#include <opencv4/opencv2/opencv.hpp>

namespace Gaia::Modules
{
	class ImageDebugUtility
	{
	public:
		static void DrawRectangle(cv::Mat& canvas,
							const cv::Point& center, const cv::Size& size,
							const cv::Scalar& color, int thickness = 3);

		static void DrawRotatedRectangle(cv::Mat& canvas,
								   const cv::Point& center, const cv::Size& size, float angle,
								   const cv::Scalar& color, int thickness = 3);

		static void DrawRotatedRectangle(cv::Mat& canvas,
		                                 const cv::RotatedRect& rotated_rect,
		                                 const cv::Scalar& color, int thickness = 3);
	};
}
