#pragma once

#include "ArmorCheckerBase.hpp"

namespace Icarus
{
    class FarArmorChecker : public ArmorCheckerBase
    {
    public:
        FarArmorChecker()
        {
            ArmorCheckerBase::MaxLeaningAngle = 15.0;
            // Disable angle check for far distance because low confidence in angle estimation.
            ArmorCheckerBase::MaxDeltaAngle = 180.0;
            CheckerBase::ScenarioTags = {"Far"};
        }

    protected:
        bool CheckPattern(PONElement *candidate) override
        {
            if (candidate->Feature.Width <= 1) return false;
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