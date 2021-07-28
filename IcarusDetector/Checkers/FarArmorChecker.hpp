#pragma once

#include "ArmorCheckerBase.hpp"

namespace Icarus
{
    class FarArmorChecker : public ArmorCheckerBase
    {
    public:
        FarArmorChecker()
        {
            CheckerBase::ScenarioTags = {"Far"};
        }
    protected:
        double MaxLengthRatio {};

        bool CheckPattern(PONElement *candidate) override
        {
            if (candidate->Feature.Width <= 1) return false;
            if (std::fabs(candidate->ContourA->Feature.Center.y - candidate->ContourB->Feature.Center.y)
                > 5)
                return false;
            if (std::fabs(candidate->ContourA->Feature.Length - candidate->ContourB->Feature.Length)
                > 3)
                return false;
            if (candidate->Feature.Length / candidate->Feature.Width > MaxLengthRatio)
                return false;

            return true;
        }
    public:
        void LoadConfiguration() override
        {
            ArmorCheckerBase::LoadConfiguration();
            ArmorCheckerBase::MaxLeaningAngle = Configurator->Get<double>("Armor/Far/MaxLeaningAngle")
                    .value_or(15.0);
            ArmorCheckerBase::MaxDeltaAngle = Configurator->Get<double>("Armor/Far/MaxDeltaAngle")
                    .value_or(180.0);
            MaxLengthRatio = Configurator->Get<double>("Armor/Far/MaxLengthRatio").value_or(4.0);
        }
    };
}