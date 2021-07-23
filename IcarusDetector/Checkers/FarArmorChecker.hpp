#pragma once

#include "ArmorCheckerBase.hpp"

namespace Icarus
{
    class FarArmorChecker : public ArmorCheckerBase
    {
    public:
        FarArmorChecker()
        {
            MaxLightBarLeaningAngle = 10.0;
            CheckerBase::ScenarioTags = {"Far"};
        }

    protected:
        bool CheckPattern(PONElement *candidate) override
        {
            if (std::fabs(candidate->ContourA->Feature.Center.y - candidate->ContourB->Feature.Center.y)
                > 5)
                return false;
            if (std::fabs(candidate->ContourA->Feature.Length - candidate->ContourB->Feature.Length)
                > 3)
                return false;
            if (candidate->Feature.Length / candidate->Feature.Width > 4.0)
                return false;

            return true;
        }
    };
}