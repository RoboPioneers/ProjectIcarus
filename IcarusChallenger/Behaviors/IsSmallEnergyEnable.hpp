#pragma once

#include "../Framework/ProcessorBase.hpp"

namespace Icarus
{
    class IsSmallEnergyEnable : public ProcessorBase
    {
        REFLECT_TYPE(Icarus, ProcessorBase)
    protected:

        bool* SmallEnergyEnable {nullptr};

        void OnInitialize() override
        {
            InitializeFacilities();
            SmallEnergyEnable = GetBlackboard()->GetPointer<bool>("SmallEnergyEnable");
        }

        Gaia::BehaviorTree::Result OnExecute() override
        {
            if (SmallEnergyEnable && *SmallEnergyEnable)
            {
                return Gaia::BehaviorTree::Result::Success;
            }
            return Gaia::BehaviorTree::Result::Failure;
        }
    };
}