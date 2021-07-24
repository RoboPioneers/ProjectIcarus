#pragma once

#include <GaiaFramework/GaiaFramework.hpp>
#include <GaiaBackground/GaiaBackground.hpp>
#include <GaiaInspectionClient/GaiaInspectionClient.hpp>
#include <GaiaSerialIOClient/GaiaSerialIOClient.hpp>
#include <GaiaCameraClient/GaiaCameraClient.hpp>
#include <atomic>
#include "DetectionLayout.hpp"

namespace Icarus
{
    class IcarusChallenger : public Gaia::Framework::Service
    {
    public:
        IcarusChallenger();

    protected:
        bool* DebugMode {nullptr};
        bool* SmallEnergyEnable {nullptr};
        bool* BigEnergyEnable {nullptr};

        std::unique_ptr<Gaia::InspectionService::InspectionClient> Inspector {nullptr};
        /// Client for camera service.
        std::shared_ptr<Gaia::CameraService::CameraClient> CameraClient {nullptr};

    protected:
        /// Behavior tree for detection.
        DetectionLayout DetectionBehaviors;

        /// Reset camera settings.
        void OnResume() override;

        /// Inject basic facilities into the behavior tree.
        void OnInstall() override;

        /// Finalize the detection behavior tree.
        void OnUninstall() override;

        /// Execute the detection behavior tree.
        void OnUpdate() override;
    };
}