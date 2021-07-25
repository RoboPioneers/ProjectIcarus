#pragma once

#include "ArmorCheckerBase.hpp"

namespace Icarus
{
    class MediumArmorChecker : public ArmorCheckerBase
    {
    public:
        MediumArmorChecker()
        {
            ArmorCheckerBase::MaxDeltaAngle = 20;
            ArmorCheckerBase::MaxLeaningAngle = 45;
            CheckerBase::ScenarioTags = {"Medium"};
        }

    protected:
        bool CheckPattern(PONElement *candidate) override
        {
            if (candidate->Feature.Length - candidate->ContourA->Feature.Width
                - candidate->ContourB->Feature.Width <
                std::max({candidate->ContourA->Feature.Width, candidate->ContourB->Feature.Width}))
                return false;

            if (std::fabs(candidate->ContourA->Feature.Angle - 90.0) > MaxLeaningAngle ||
                std::fabs(candidate->ContourB->Feature.Angle - 90.0) > MaxLeaningAngle)
                return false;

            if (std::fabs(candidate->ContourA->Feature.Angle - 90) > 2.0 ||
                std::fabs(candidate->ContourB->Feature.Angle - 90) > 2.0)
            {
                if ((candidate->ContourA->Feature.Angle - 90) * (candidate->ContourB->Feature.Angle - 90) < 0)
                {
                    return false;
                }
            }

            if (std::fabs(candidate->ContourA->Feature.Center.y - candidate->ContourB->Feature.Center.y)
                > std::max({candidate->ContourA->Feature.Length, candidate->ContourB->Feature.Length}))
                return false;

            if (std::fabs(candidate->ContourA->Feature.Angle - 90.0) > 3.0||
                std::fabs(candidate->ContourB->Feature.Angle - 90.0) > 3.0)
            {
                auto a_direction = candidate->ContourA->Feature.Angle - 90.0;
                auto b_direction = candidate->ContourB->Feature.Angle - 90.0;
                double direction = std::fabs(a_direction) > std::fabs(b_direction) ? a_direction : b_direction;
                auto relative_position_symbol =
                        (candidate->ContourA->Rectangle.center.x - candidate->ContourB->Rectangle.center.x) *
                        (candidate->ContourA->Rectangle.center.y - candidate->ContourB->Rectangle.center.y);
                if (direction > 0.0 && relative_position_symbol > 0)
                    return false;
                if (direction < 0.0 && relative_position_symbol < 0)
                    return false;
            }

            if (std::fabs(candidate->ContourA->Feature.Angle - 90.0) < 3 &&
                std::fabs(candidate->ContourB->Feature.Angle - 90.0) < 3 &&
                std::fabs(candidate->ContourA->Rectangle.center.y - candidate->ContourB->Rectangle.center.y)
                > 10)
                return false;

            if (std::fabs(candidate->ContourA->Feature.Length - candidate->ContourB->Feature.Length)
                > std::min({candidate->ContourA->Feature.Length, candidate->ContourB->Feature.Length}) * 0.6)
                return false;

            auto length_ratio = candidate->Feature.Length / candidate->Feature.Width;
            if (length_ratio > 9)
                return false;
            if (length_ratio < 1.5)
                return false;

            return true;
        }
    };
}