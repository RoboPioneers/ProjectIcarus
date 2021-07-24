#pragma once

#include <GaiaFramework/GaiaFramework.hpp>
#include <GaiaBackground/GaiaBackground.hpp>
#include <GaiaInspectionClient/GaiaInspectionClient.hpp>
#include <GaiaSerialIOClient/GaiaSerialIOClient.hpp>
#include <atomic>
#include "DetectionLayout.hpp"

namespace Icarus
{
    class IcarusDetector : public Gaia::Framework::Service
    {
    public:
        IcarusDetector();

    protected:
        bool* DebugMode {nullptr};
        bool* SmallEnergyEnable {nullptr};
        bool* BigEnergyEnable {nullptr};

        std::unique_ptr<Gaia::InspectionService::InspectionClient> Inspector {nullptr};

    protected:
        /// Behavior tree for detection.
        DetectionLayout DetectionBehaviors;

        /// Inject basic facilities into the behavior tree.
        void OnInstall() override;

        /// Finalize the detection behavior tree.
        void OnUninstall() override;

        /// Execute the detection behavior tree.
        void OnUpdate() override;
    };
}