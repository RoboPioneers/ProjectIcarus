#pragma once

#include "LightBarCheckerBase.hpp"

namespace Icarus
{
    class NearLightBarChecker : public LightBarCheckerBase
    {
    public:
        NearLightBarChecker()
        {
            CheckerBase::ScenarioTags = {"Far"};
        }

    protected:
        bool CheckPattern(ContourElement *candidate) override
        {
            auto length_ratio = candidate->Feature.Length / candidate->Feature.Width;
            if (length_ratio < 3.0) return false;
            return true;
        }
    };
}