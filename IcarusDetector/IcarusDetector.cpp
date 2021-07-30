#include "IcarusDetector.hpp"
#include "Modules/GeneralMessageTranslator.hpp"
#include "Signal.pb.h"
#include <thread>
#ifdef OFFLINE
#include <iostream>
#endif

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
        auto camera_type = GetConfigurator()->Get<std::string>("CameraType").value_or("*");
        auto camera_index = GetConfigurator()->Get<unsigned int>("CameraIndex").value_or(0);
        #ifndef OFFLINE
        CameraClient = std::make_shared<Gaia::CameraService::CameraClient>(camera_type, camera_index,
                                                                           GetConnection());
        auto camera_exposure = GetConfigurator()->Get<unsigned int>("Exposure").value_or(2500);
        auto camera_gain = GetConfigurator()->Get<unsigned int>("Gain").value_or(16);
        CameraClient->SetExposure(camera_exposure);
        CameraClient->SetGain(camera_gain);
        #endif
        auto context = std::make_shared<Gaia::Blackboards::Blackboard>();

        context->GetPointer<std::shared_ptr<sw::redis::Redis>>("Connection", GetConnection());
        context->GetPointer<Gaia::Framework::Clients::LogClient*>("Logger", GetLogger());
        context->GetPointer<Gaia::Framework::Clients::ConfigurationClient*>("Configurator", GetConfigurator());
        context->GetPointer<Gaia::InspectionService::InspectionClient*>("Inspector", Inspector.get());
        DebugMode = context->GetPointer<bool>("DebugMode", false);

        IsEnemyRed = context->GetPointer<bool>("IsEnemyRed", true);

        if (OptionVariables.count("debug"))
        {
            *DebugMode = true;
        }

        #ifdef OFFLINE
        *DebugMode = true;
        #endif

        DetectionBehaviors.Initialize(context);

        auto serial_port = GetConfigurator()->Get<std::string>("SerialPort").value_or("ttyTHS2");

        AddCommand("debug", [this](const std::string& content){
            if (content == "on")
            {
                *(this->DebugMode) = true;
            }
            else if (content == "off")
            {
                *(this->DebugMode) = false;
            }
        });

        AddSubscription("serial_ports/" + serial_port + "/read",
                        [this](const std::string& content){
            auto [id, package] = Modules::GeneralMessageTranslator::Decode(content);
            if (id != 3) return;
            Signal signal;
            signal.ParseFromString(package);
            if (signal.name() == "sc_b")
            {
                this->EnemyColorInitialized = true;
                *IsEnemyRed = true;
                this->GetLogger()->RecordMilestone(
                        "Color received: alley color blue, enemy color red.");
            } else if (signal.name() == "sc_r")
            {
                this->EnemyColorInitialized = true;
                *IsEnemyRed = false;
                this->GetLogger()->RecordMilestone(
                        "Color received: alley color red, enemy color blue.");
            } else if (signal.name() == "to_ec_b" || signal.name() == "to_ec_s")
            {
                this->Pause();
                GetLogger()->RecordMilestone("Switch to energy mode, battle assistant paused.");
            } else if (signal.name() == "to_a")
            {
                this->Resume();
                GetLogger()->RecordMilestone("Switch back to back assistant mode.");
            }
            LastFrameTimePoint = std::chrono::steady_clock::now();
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
        auto current_time = std::chrono::steady_clock::now();

        // Suppress frequency to approximately 130 fps.
        if (current_time < LastFrameTimePoint + std::chrono::microseconds(7692))
        {
//            std::this_thread::sleep_for(std::chrono::microseconds(7692) -
//                std::chrono::duration_cast<std::chrono::microseconds>(current_time - LastFrameTimePoint));
        }
        current_time = std::chrono::steady_clock::now();
        LastFrameTimePoint = current_time;

        #ifdef OFFLINE
        auto measure_time = std::chrono::system_clock::now();

        if (measure_time - LastMeasuringTime > std::chrono::seconds(3))
        {
            auto fps = static_cast<double>(AccumulatedFramesCount) / static_cast<double>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(measure_time - LastMeasuringTime).count())
                            * 1000;
            AccumulatedFramesCount = 0;
            std::cout << "FPS: " + std::to_string(fps) << std::endl;
            LastMeasuringTime = measure_time;
        }
        ++AccumulatedFramesCount;
        #endif
        if (!EnemyColorInitialized) return;
        DetectionBehaviors.Execute();
    }

    void IcarusDetector::OnResume()
    {
        #ifndef OFFLINE
        auto camera_exposure = GetConfigurator()->Get<unsigned int>("Exposure").value_or(2500);
        auto camera_gain = GetConfigurator()->Get<unsigned int>("Gain").value_or(16);
        CameraClient->SetExposure(camera_exposure);
        CameraClient->SetGain(camera_gain);
        #endif
    }
}