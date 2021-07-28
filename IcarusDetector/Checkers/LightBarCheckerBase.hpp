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
        double MaxLeaningAngle {};

        LightBarCheckerBase()
        {
            CheckerBase::PatternTags = {"LightBar"};
        }

    protected:
        /// Check distance scenario.
        bool CheckScenario(ContourElement *candidate) override
        {
            if (candidate->Feature.Angle > (90 + MaxLeaningAngle) ||
                candidate->Feature.Angle < (90 - MaxLeaningAngle))
                return false;
            if (candidate->Feature.Length < MinLength || candidate->Feature.Length > MaxLength)
                return false;
            return true;
        }
    public:
        void LoadConfiguration() override
        {
            MinLength = Configurator->Get<int>("Bar/Base/MinLength").value_or(0);
            MaxLength = Configurator->Get<int>("Bar/Base/MaxLength").value_or(0);
            MaxLeaningAngle = Configurator->Get<double>("Bar/Base/MaxLeaningAngle").value_or(30.0);
        }
    };
}