#pragma once

#include <GaiaFramework/GaiaFramework.hpp>
#include <GaiaBackground/GaiaBackground.hpp>
#include <GaiaInspectionClient/GaiaInspectionClient.hpp>
#include <GaiaSerialIOClient/GaiaSerialIOClient.hpp>
#include <GaiaCameraClient/GaiaCameraClient.hpp>
#include <atomic>
#include <chrono>
#include "DetectionLayout.hpp"

namespace Icarus
{
    class IcarusDetector : public Gaia::Framework::Service
    {
    public:
        IcarusDetector();

    protected:
        bool* DebugMode {nullptr};

        std::unique_ptr<Gaia::InspectionService::InspectionClient> Inspector {nullptr};
        /// Client for camera service.
        std::shared_ptr<Gaia::CameraService::CameraClient> CameraClient {nullptr};

        std::atomic_bool EnemyColorInitialized {false};
        bool* IsEnemyRed {nullptr};

        /// Time point of the last frame.
        std::chrono::steady_clock::time_point LastFrameTimePoint;

        #ifdef OFFLINE
        /// Time point of last FPS measuring.
        std::chrono::system_clock::time_point LastMeasuringTime;
        /// Accumulated frames counter from last FPS measuring.
        unsigned int AccumulatedFramesCount {0};
        #endif
    protected:
        /// Behavior tree for detection.
        DetectionLayout DetectionBehaviors;

        /// Inject basic facilities into the behavior tree.
        void OnInstall() override;

        /// Finalize the detection behavior tree.
        void OnUninstall() override;

        /// Execute the detection behavior tree.
        void OnUpdate() override;

        /// Reset camera settings.
        void OnResume() override;
    };
}