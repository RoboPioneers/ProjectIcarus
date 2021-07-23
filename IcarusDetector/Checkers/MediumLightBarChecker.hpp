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
            return true;
        }
    };
}