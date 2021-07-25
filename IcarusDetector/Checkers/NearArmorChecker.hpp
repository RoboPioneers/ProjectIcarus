#pragma once

#include "ArmorCheckerBase.hpp"

namespace Icarus
{
    class NearArmorChecker : public ArmorCheckerBase
    {
    public:
        NearArmorChecker()
        {
            ArmorCheckerBase::MaxDeltaAngle = 6.0f;
            ArmorCheckerBase::MaxLeaningAngle = 30.0f;
            CheckerBase::ScenarioTags = {"Near"};
        }

    protected:
        bool CheckPattern(PONElement *candidate) override
        {
            if (candidate->Feature.Length - candidate->ContourA->Feature.Width
                - candidate->ContourB->Feature.Width < std::max({
                    candidate->ContourA->Feature.Width, candidate->ContourB->Feature.Width, 35.0
                }))
                return false;
            if (std::fabs(candidate->ContourA->Feature.Angle - 90.0) > MaxLeaningAngle ||
                std::fabs(candidate->ContourB->Feature.Angle - 90.0) > MaxLeaningAngle)
                return false;

            if (std::fabs(candidate->ContourA->Feature.Angle - 90) > 2.0 &&
                std::fabs(candidate->ContourB->Feature.Angle - 90) > 2.0)
            {
                if ((candidate->ContourA->Feature.Angle - 90) * (candidate->ContourB->Feature.Angle - 90) < 0)
                {
                    return false;
                }
            }

            if (std::fabs(candidate->ContourA->Feature.Angle - 90.0) < 3 &&
                std::fabs(candidate->ContourB->Feature.Angle - 90.0) < 3 &&
                std::fabs(candidate->ContourA->Rectangle.center.y - candidate->ContourB->Rectangle.center.y)
                > 10)
                return false;

            if (std::fabs(candidate->ContourA->Feature.Length - candidate->ContourB->Feature.Length)
                > std::min({candidate->ContourA->Feature.Length, candidate->ContourB->Feature.Length}) * 0.6)
                return false;
            return true;
        }
    };
}