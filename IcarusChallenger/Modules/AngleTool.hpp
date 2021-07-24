#pragma once

#include <cmath>

namespace Gaia::Modules
{
    class AngleTool
    {
    public:
        static constexpr double Pi = 3.1415926535f;

        [[nodiscard]] static inline constexpr double ToDegree(double angle_radian)
        {
            return angle_radian / Pi * 180.0f;
        }

        [[nodiscard]] static inline constexpr double ToRadian(double angle_degree)
        {
            return angle_degree / 180.0f * Pi;
        }
    };
}