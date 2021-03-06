#include "SendingCommand.hpp"
#include <sw/redis++/redis++.h>
#include "../Modules/GeneralMessageTranslator.hpp"
#include "TurretCommand.pb.h"

namespace Icarus
{
    using namespace Gaia::BehaviorTree;

    void SendingCommand::OnInitialize()
    {
        InitializeFacilities();
        auto connection = GetBlackboard()
                ->GetPointer<std::shared_ptr<sw::redis::Redis>>(
                        "Connection");
        Serial = std::make_unique<Gaia::SerialIO::SerialClient>(
                GetConfigurator()->Get("SerialPort").value_or("ttyTHS2"),
                *connection);

        HitPoint = GetBlackboard()->GetPointer<cv::Point2i>("HitPoint", cv::Point2i());
        HitCommand = GetBlackboard()->GetPointer<int>("HitCommand", 0);
        HitDistance = GetBlackboard()->GetPointer<double>("HitDistance");
        MotionStatus = GetBlackboard()->GetPointer<int>("MotionStatus");

        LoadConfigurations();
    }

    Result SendingCommand::OnExecute()
    {
        GetInspector()->UpdateValue("HitCommand", std::to_string(*HitCommand));
        GetInspector()->UpdateValue("HitPoint",
                               std::to_string(HitPoint->x) + "," + std::to_string(HitPoint->y));

        TurretCommand command;
        command.set_yaw(static_cast<float>(HitPoint->x));
        command.set_pitch(static_cast<float>(HitPoint->y));
        command.set_command(*HitCommand);
        command.set_distance(static_cast<unsigned int>(*HitDistance));
        command.set_motion_status(*MotionStatus);

        std::string command_bytes;
        command_bytes.resize(command.ByteSize());
        command.SerializeToArray(command_bytes.data(), static_cast<int>(command_bytes.size()));

        std::string package_bytes;
        package_bytes = Modules::GeneralMessageTranslator::Encode(3, command_bytes);

        Serial->Send(package_bytes);

        *HitCommand = 0;

        return Result::Success;
    }
}