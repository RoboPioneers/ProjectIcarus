#pragma once

#include "ArmorCheckerBase.hpp"

namespace Icarus
{
    class NearArmorChecker : public ArmorCheckerBase
    {
    public:
        NearArmorChecker()
        {
            MaxDeltaAngle = 8.0f;
            CheckerBase::ScenarioTags = {"Near"};
        }

    protected:
        bool CheckPattern(PONElement *candidate) override
        {
            if (std::fabs(candidate->ContourA->Feature.Center.y - candidate->ContourB->Feature.Center.y)
                > std::max({candidate->ContourA->Feature.Length, candidate->ContourB->Feature.Length}))
                return false;

            if (std::fabs(candidate->ContourA->Feature.Length - candidate->ContourB->Feature.Length)
                > std::min({candidate->ContourA->Feature.Length, candidate->ContourB->Feature.Length}) * 0.6)
                return false;
            return true;
        }
    };
}