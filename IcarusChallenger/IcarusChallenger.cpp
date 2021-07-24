#include "IcarusChallenger.hpp"
#include "Modules/GeneralMessageTranslator.hpp"
#include "Signal.pb.h"

namespace Icarus
{
    IcarusChallenger::IcarusChallenger() : Gaia::Framework::Service("IcarusChallenger")
    {
        Gaia::Framework::Service::OptionDescription.add_options()
                ("debug,d", "enable debug mode.");
    }

    /// Inject basic facilities into the behavior tree.
    void IcarusChallenger::OnInstall()
    {
        Inspector = std::make_unique<Gaia::InspectionService::InspectionClient>(Name, GetConnection());
        auto context = std::make_shared<Gaia::Blackboards::Blackboard>();

        context->GetPointer<std::shared_ptr<sw::redis::Redis>>("Connection", GetConnection());
        context->GetPointer<Gaia::Framework::Clients::LogClient*>("Logger", GetLogger());
        context->GetPointer<Gaia::Framework::Clients::ConfigurationClient*>("Configurator", GetConfigurator());
        context->GetPointer<Gaia::InspectionService::InspectionClient*>("Inspector", Inspector.get());
        DebugMode = context->GetPointer<bool>("DebugMode", false);
        SmallEnergyEnable = context->GetPointer<bool>("SmallEnergyEnable", false);
        BigEnergyEnable = context->GetPointer<bool>("BigEnergyEnable", false);

        if (OptionVariables.count("debug"))
        {
            *DebugMode = true;
        }

        DetectionBehaviors.Initialize(context);

        auto serial_port = GetConfigurator()->Get<std::string>("SerialPort").value_or("ttyTHS2");

        AddCommand("to_ec_b", [this](const std::string& content){
            *(this->BigEnergyEnable) = true;
            *(this->SmallEnergyEnable) = false;
        });
        AddCommand("to_ec_s", [this](const std::string& content){
            *(this->BigEnergyEnable) = false;
            *(this->SmallEnergyEnable) = true;
        });
        AddCommand("stop_ec", [this](const std::string& content){
            *(this->BigEnergyEnable) = false;
            *(this->SmallEnergyEnable) = false;
        });

        AddSubscription("serial_ports/" + serial_port + "/read",
                        [this](const std::string& content){
            auto [id, package] = Modules::GeneralMessageTranslator::Decode(content);
            if (id != 3) return;
            Signal signal;
            signal.ParseFromString(package);
            if (signal.name() == "to_ec_b")
            {
                *(this->SmallEnergyEnable) = false;
                *(this->BigEnergyEnable) = true;
            } else if (signal.name() == "to_ec_s")
            {
                *(this->SmallEnergyEnable) = true;
                *(this->BigEnergyEnable) = false;
            } else if (signal.name() == "stop_ec" || signal.name() == "to_a")
            {
                *(this->SmallEnergyEnable) = false;
                *(this->BigEnergyEnable) = false;
            }
        });
    }

    /// Finalize the whole behavior tree.
    void IcarusChallenger::OnUninstall()
    {
        DetectionBehaviors.Finalize();
    }

    /// Execute the main detection behavior tree.
    void IcarusChallenger::OnUpdate()
    {
        DetectionBehaviors.Execute();
    }
}