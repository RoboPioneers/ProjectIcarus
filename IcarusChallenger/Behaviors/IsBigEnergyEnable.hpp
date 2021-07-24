#pragma once

#include "../Framework/ProcessorBase.hpp"

namespace Icarus
{
    class IsBigEnergyEnable : public ProcessorBase
    {
        REFLECT_TYPE(Icarus, ProcessorBase)
    protected:

        bool* BigEnergyEnable {nullptr};

        void OnInitialize() override
        {
            InitializeFacilities();
            BigEnergyEnable = GetBlackboard()->GetPointer<bool>("BigEnergyEnable");
        }

        Gaia::BehaviorTree::Result OnExecute() override
        {
            if (BigEnergyEnable && *BigEnergyEnable)
            {
                return Gaia::BehaviorTree::Result::Success;
            }
            return Gaia::BehaviorTree::Result::Failure;
        }
    };
}