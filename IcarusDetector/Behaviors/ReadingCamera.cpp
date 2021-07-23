#include "ReadingCamera.hpp"
#include <GaiaExceptions/GaiaExceptions.hpp>

namespace Icarus
{
    /// Initialize camera facilities.
    void ReadingCamera::OnInitialize()
    {
        InitializeFacilities();

        MainPicture = GetBlackboard()->GetObject<cv::Mat>("MainPicture");

        auto camera_type = GetConfigurator()->Get<std::string>("CameraType").value_or("*");
        auto camera_index = GetConfigurator()->Get<unsigned int>("CameraIndex").value_or(0);
        auto vision_name = GetConfigurator()->Get<std::string>("VisionName").value_or("main");

        CameraClient = std::make_shared<Gaia::CameraService::CameraClient>(camera_type, camera_index,
                                                                           GetConnection());
        CameraReader = std::make_shared<Gaia::CameraService::CameraReader>(
                CameraClient->GetReader(vision_name));

        LastRetrievingTime = std::chrono::system_clock::now();
        LastMeasuringTime = std::chrono::system_clock::now();
    }

    Gaia::BehaviorTree::Result ReadingCamera::OnExecute()
    {
        auto current_time = std::chrono::system_clock::now();

        if (current_time - LastMeasuringTime > std::chrono::seconds(3))
        {
            auto fps = static_cast<double>(AccumulatedFramesCount) / static_cast<double>(
                    std::chrono::duration_cast<std::chrono::milliseconds>(current_time - LastMeasuringTime).count())
                    * 1000;
            AccumulatedFramesCount = 0;
            GetLogger()->RecordMessage("FPS: " + std::to_string(fps));
            LastMeasuringTime = current_time;
        }
        if (CameraReader->ReadTimestamp() - LastRetrievingTime > std::chrono::milliseconds(1500))
        {
            GetLogger()->RecordError("Camera timestamp overtime, considered as camera offline.");
            throw Gaia::Exceptions::ExceptionWrapper<std::runtime_error>("Camera timestamp overtime");
        }
        ++AccumulatedFramesCount;

        *MainPicture = CameraReader->Read();
        LastRetrievingTime = CameraReader->ReadTimestamp();
        return BehaviorTree::Result::Success;
    }
}