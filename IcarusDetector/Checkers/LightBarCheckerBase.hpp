#pragma once

#include "../Framework/CheckerBase.hpp"
#include "../Components/ContourElement.hpp"
#include <cmath>

namespace Icarus
{
    /// Distance scenario based light bar checker base class.
    class LightBarCheckerBase : public CheckerBase<ContourElement>
    {
    public:
        int MinLength {0};
        int MaxLength {0};
        double MaxLightBarLeaningAngle {60.0};

        LightBarCheckerBase()
        {
            CheckerBase::PatternTags = {"LightBar"};
        }

    protected:
        /// Check distance scenario.
        bool CheckScenario(ContourElement *candidate) override
        {
            if (candidate->Feature.Angle > (180.0 - MaxLightBarLeaningAngle) ||
                candidate->Feature.Angle < MaxLightBarLeaningAngle)
                return false;
            if (candidate->Feature.Length < MinLength || candidate->Feature.Length > MaxLength) return false;
            return true;
        }
    };
}