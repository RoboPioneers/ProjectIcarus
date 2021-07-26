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
            if (candidate->Feature.Length < 3 || candidate->Feature.Width < 2)
                return false;
            if (candidate->Feature.Length - candidate->Feature.Width > 6)
                return false;
            if (candidate->Rectangle.center.y < 240)
                return false;
            return true;
        }
    };
}