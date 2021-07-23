#pragma once

#include <GaiaComponents/GaiaComponents.hpp>
#include <GaiaTags/GaiaTags.hpp>
#include <opencv2/opencv.hpp>
#include <memory>
#include <vector>
#include "../Modules/GeometryFeature.hpp"

namespace Icarus
{
    using namespace Gaia;

    /**
     * @brief Candidate contour element.
     */
    class ContourElement : public Components::Component, public Tags::Holder
    {
    public:
        using Pointer = std::shared_ptr<ContourElement>;

        /// Contour points.
        std::vector<cv::Point> Points;
        /// Bounding rotated rectangle.
        cv::RotatedRect Rectangle;
        /// Geometry feature of this contour.
        Modules::GeometryFeature Feature;
    };
}