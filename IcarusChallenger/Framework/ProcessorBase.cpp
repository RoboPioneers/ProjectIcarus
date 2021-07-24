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
        Connection = *GetBlackboard()->GetPointer<
                std::shared_ptr<sw::redis::Redis>>("Connection");
        Logger = *GetBlackboard()->GetPointer<Framework::Clients::LogClient*>("Logger");
        Configurator = *GetBlackboard()->GetPointer<
                Framework::Clients::ConfigurationClient*>("Configurator");
        Inspector = *GetBlackboard()->GetPointer<
                Gaia::InspectionService::InspectionClient*>("Inspector");
        DebugMode = GetBlackboard()->GetPointer<bool>("DebugMode");

        LastConfigureTime = std::chrono::system_clock::now();
    }
}