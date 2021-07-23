#pragma once

#include <GaiaFramework/GaiaFramework.hpp>
#include <GaiaBehaviorTree/GaiaBehaviorTree.hpp>
#include <GaiaInspectionClient/GaiaInspectionClient.hpp>
#include <chrono>
#include <memory>

#ifndef DEBUG_MACRO
#define DEBUG_MACRO
#define DEBUG_BEGIN if (IsDebugMode()) {
#define DEBUG_END }
#endif

namespace Icarus
{
    using namespace Gaia;

    /**
     * @brief Base class for processor behaviors.
     * @details Provide interface for accessing basic facilities.
     */
    class ProcessorBase : public Gaia::BehaviorTree::Behavior
    {
        REFLECT_TYPE(Gaia::BehaviorTree, Behavior)
    private:
        /// Connection for this context.
        std::shared_ptr<sw::redis::Redis> Connection {nullptr};
        /// Logger for this context.
        Framework::Clients::LogClient* Logger {nullptr};
        /// Configurator for this context.
        Framework::Clients::ConfigurationClient* Configurator {nullptr};
        /// Inspector for this context.
        Gaia::InspectionService::InspectionClient* Inspector {nullptr};
        /// Switch for debug functions.
        bool* DebugMode {nullptr};

        /// Time point of last configuration.
        std::chrono::system_clock::time_point LastConfigureTime;

    protected:
        /// Load configurations here.
        virtual void LoadConfigurations() {};

        /// Check timestamp and auto reload configuration every 1 second.
        void CheckReloadConfiguration();

        /// Initialize basic facilities.
        void InitializeFacilities();

        /// Get the connection of the context.
        [[nodiscard]] inline const decltype(Connection)& GetConnection() const noexcept
        {
            return Connection;
        }
        /// Get the logger of the context.
        [[nodiscard]] inline decltype(Logger) GetLogger() const noexcept
        {
            return Logger;
        }
        /// Get the configurator of the context.
        [[nodiscard]] inline decltype(Configurator) GetConfigurator() const noexcept
        {
            return Configurator;
        }
        /// Get the inspector of the context.
        [[nodiscard]] inline decltype(Inspector) GetInspector() const noexcept
        {
            return Inspector;
        }
        /// Check whether the context is in debug mode or not.
        [[nodiscard]] inline bool IsDebugMode() const
        {
            return *DebugMode;
        }

        /// Bind context facilities.
        void OnInitialize() override;
    };
}
