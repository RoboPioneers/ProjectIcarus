#include "ProcessorBase.hpp"

namespace Icarus
{
    /// Bind context facilities.
    void ProcessorBase::OnInitialize()
    {
        InitializeFacilities();
        LoadConfigurations();
    }

    /// Frequently reload configurations in debug mode
    void ProcessorBase::CheckReloadConfiguration()
    {
        if (std::chrono::system_clock::now() - LastConfigureTime
            < std::chrono::seconds(1))
            return;

        LoadConfigurations();

        LastConfigureTime = std::chrono::system_clock::now();
    }

    /// Initialize platform facilities.
    void ProcessorBase::InitializeFacilities()
    {
        Connection = *GetBlackboard()->GetVariable<
                std::shared_ptr<sw::redis::Redis>>("Connection");
        Logger = *GetBlackboard()->GetVariable<Framework::Clients::LogClient*>("Logger");
        Configurator = *GetBlackboard()->GetVariable<
                Framework::Clients::ConfigurationClient*>("Configurator");
        Inspector = *GetBlackboard()->GetVariable<
                Gaia::InspectionService::InspectionClient*>("Inspector");
        DebugMode = GetBlackboard()->GetVariable<bool>("DebugMode");

        LastConfigureTime = std::chrono::system_clock::now();
    }
}