#pragma once

#include "LightBarCheckerBase.hpp"

namespace Icarus
{
    class FarLightBarChecker : public LightBarCheckerBase
    {
    public:
        FarLightBarChecker()
        {
            // Disable light bar leaning check for far distance.
            LightBarCheckerBase::MaxLeaningAngle = 90;
            CheckerBase::ScenarioTags = {"Far"};
        }

    protected:
        bool CheckPattern(ContourElement *candidate) override
        {
            if (candidate->Feature.Length - candidate->Feature.Width > 6)
                return false;
            return true;
        }
    };
}