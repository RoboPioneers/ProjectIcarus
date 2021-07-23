#pragma once

#include "LightBarCheckerBase.hpp"

namespace Icarus
{
    class MediumLightBarChecker : public LightBarCheckerBase
    {
    public:
        MediumLightBarChecker()
        {
            CheckerBase::ScenarioTags = {"Far"};
        }

    protected:
        bool CheckPattern(ContourElement *candidate) override
        {
            auto length_ratio = candidate->Feature.Length / candidate->Feature.Width;
            if (length_ratio > 3.0) return false;
            if (length_ratio < 1.3) return false;
            return true;
        }
    };
}