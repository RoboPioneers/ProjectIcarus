#include "IcarusDetector.hpp"
#include "Modules/GeneralMessageTranslator.hpp"
#include "Signal.pb.h"

namespace Icarus
{
    IcarusDetector::IcarusDetector() : Gaia::Framework::Service("IcarusDetector")
    {
        Gaia::Framework::Service::OptionDescription.add_options()
                ("debug,d", "enable debug mode.");
    }

    /// Inject basic facilities into the behavior tree.
    void IcarusDetector::OnInstall()
    {
        Inspector = std::make_unique<Gaia::InspectionService::InspectionClient>(Name, GetConnection());
        auto context = std::make_shared<Gaia::Blackboards::Blackboard>();

        context->GetPointer<std::shared_ptr<sw::redis::Redis>>("Connection", GetConnection());
        context->GetPointer<Gaia::Framework::Clients::LogClient*>("Logger", GetLogger());
        context->GetPointer<Gaia::Framework::Clients::ConfigurationClient*>("Configurator", GetConfigurator());
        context->GetPointer<Gaia::InspectionService::InspectionClient*>("Inspector", Inspector.get());
        DebugMode = context->GetPointer<bool>("DebugMode", false);

        EnemyColorMinHue = context->GetPointer<unsigned int>("EnemyMinHue", 100);
        EnemyColorMaxHue = context->GetPointer<unsigned int>("EnemyMaxHue", 120);

        if (OptionVariables.count("debug"))
        {
            *DebugMode = true;
        }

        DetectionBehaviors.Initialize(context);

        auto serial_port = GetConfigurator()->Get<std::string>("SerialPort").value_or("ttyTHS2");

        AddSubscription("serial_ports/" + serial_port + "/read",
                        [this](const std::string& content){
            auto [id, package] = Modules::GeneralMessageTranslator::Decode(content);
            if (id != 3) return;
            Signal signal;
            signal.ParseFromString(package);
            if (signal.name() == "sc_b")
            {
                this->EnemyColorInitialized = true;
                *(this->EnemyColorMinHue) = 0;
                *(this->EnemyColorMaxHue) = 20;
                this->GetLogger()->RecordMilestone(
                        "Color received: alley color blue, enemy color red ["
                        + std::to_string(*(this->EnemyColorMinHue)) + ","
                        + std::to_string(*(this->EnemyColorMaxHue)) + "]");
            } else if (signal.name() == "sc_r")
            {
                this->EnemyColorInitialized = true;
                *(this->EnemyColorMinHue) = 100;
                *(this->EnemyColorMaxHue) = 120;
                this->GetLogger()->RecordMilestone(
                        "Color received: alley color red, enemy color blue ["
                        + std::to_string(*(this->EnemyColorMinHue)) + ","
                        + std::to_string(*(this->EnemyColorMaxHue)) + "]");
            } else if (signal.name() == "to_ec_b" || signal.name() == "to_ec_s")
            {
                this->Enable = false;
                GetLogger()->RecordMilestone("Switch to energy mode, battle assistant paused.");
            } else if (signal.name() == "to_a")
            {
                this->Enable = true;
                GetLogger()->RecordMilestone("Switch back to back assistant mode.");
            }
        });
    }

    /// Finalize the whole behavior tree.
    void IcarusDetector::OnUninstall()
    {
        DetectionBehaviors.Finalize();
    }

    /// Execute the main detection behavior tree.
    void IcarusDetector::OnUpdate()
    {
        if (!EnemyColorInitialized) return;
        DetectionBehaviors.Execute();
    }
}