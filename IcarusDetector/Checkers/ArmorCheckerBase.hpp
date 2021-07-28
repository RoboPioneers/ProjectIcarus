#pragma once

#include "../Framework/CheckerBase.hpp"
#include "../Components/PONElement.hpp"
#include <cmath>

namespace Icarus
{
    /// Distance scenario based light bar checker base class.
    class ArmorCheckerBase : public CheckerBase<PONElement>
    {
    public:
        double MaxLeaningAngle {};
        double MaxDeltaAngle {};

        ArmorCheckerBase()
        {
            CheckerBase::PatternTags = {"Armor"};
        }

        void LoadConfiguration() override
        {
            MaxLeaningAngle = Configurator->Get<double>("Armor/Base/MaxLeaningAngle").value_or(20);
            MaxDeltaAngle = Configurator->Get<double>("Armor/Base/MaxDeltaAngle").value_or(20);
        }
    protected:
        /// Check distance scenario.
        bool CheckScenario(PONElement *candidate) override
        {
            if (candidate->Feature.Angle < (180.0 - MaxLeaningAngle) &&
                candidate->Feature.Angle > MaxLeaningAngle)
                return false;
            if (std::fabs(candidate->ContourA->Feature.Angle - candidate->ContourB->Feature.Angle)
                > MaxDeltaAngle)
                return false;
            return true;
        }
    };
}