#pragma once

#include "LightBarCheckerBase.hpp"

namespace Icarus
{
    class FarLightBarChecker : public LightBarCheckerBase
    {
    public:
        FarLightBarChecker()
        {
            CheckerBase::ScenarioTags = {"Far"};
        }

    protected:
        bool CheckPattern(ContourElement *candidate) override
        {
            if (candidate->Feature.Length < 3) return false;
            if (candidate->Feature.Length - candidate->Feature.Width > 5) return false;
            return true;
        }
    };
}