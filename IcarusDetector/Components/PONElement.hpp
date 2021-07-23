#pragma once

#include <GaiaComponents/GaiaComponents.hpp>
#include <GaiaTags/GaiaTags.hpp>
#include <opencv2/opencv.hpp>
#include <memory>
#include <vector>
#include "../Modules/GeometryFeature.hpp"
#include "ContourElement.hpp"

namespace Icarus
{
    using namespace Gaia;

    /**
     * @brief Candidate PON element.
     */
    class PONElement : public Components::Component, public Tags::Holder
    {
    public:
        using Pointer = std::shared_ptr<PONElement>;

        ContourElement* ContourA {nullptr};
        ContourElement* ContourB {nullptr};

        cv::RotatedRect Rectangle;
        Modules::GeometryFeature Feature;
    };
}