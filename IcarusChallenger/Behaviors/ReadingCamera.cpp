#include "ReadingCamera.hpp"
#include <GaiaExceptions/GaiaExceptions.hpp>
#include <boost/filesystem.hpp>

namespace Icarus
{
    /// Initialize camera facilities.
    void ReadingCamera::OnInitialize()
    {
        InitializeFacilities();

        MainPicture = GetBlackboard()->GetPointer<cv::Mat>("MainPicture");

        auto camera_type = GetConfigurator()->Get<std::string>("CameraType").value_or("*");
        auto camera_index = GetConfigurator()->Get<unsigned int>("CameraIndex").value_or(0);
        auto vision_name = GetConfigurator()->Get<std::string>("VisionName").value_or("main");
        RecordEnable = GetConfigurator()->Get<int>("RecordEnable").value_or(false);
        RecordInterval = GetConfigurator()->Get<int>("RecordInterval").value_or(1500);

        CameraClient = std::make_shared<Gaia::CameraService::CameraClient>(camera_type, camera_index,
                                                                           GetConnection());
        CameraReader = std::make_shared<Gaia::CameraService::CameraReader>(
                CameraClient->GetReader(vision_name));
        LastMeasuringTime = std::chrono::system_clock::now();

        using namespace boost;
        if (!filesystem::exists(RecordsSavePath))
        {
            filesystem::create_directories(RecordsSavePath);
        }

        auto records_end = filesystem::directory_iterator();
        for (auto records_iterator = filesystem::directory_iterator(RecordsSavePath);
        records_iterator != records_end; ++records_iterator)
        {
            ++RecordsSaveIndex;
        }
        ++RecordsSaveIndex;
        GetLogger()->RecordMessage("Save records from index: " + std::to_string(RecordsSaveIndex));
        LastRecordingTime = std::chrono::system_clock::now();
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
        if (current_time - CameraReader->ReadTimestamp() > std::chrono::milliseconds(1500))
        {
            GetLogger()->RecordError("Camera timestamp overtime, considered as camera offline.");
            return BehaviorTree::Result::Failure;
        }
        ++AccumulatedFramesCount;

        *MainPicture = CameraReader->Read();

        if (RecordEnable != 0 && current_time - LastRecordingTime > std::chrono::milliseconds(RecordInterval))
        {
            cv::imwrite(RecordsSavePath + "/Record_" + std::to_string(RecordsSaveIndex) + ".png", *MainPicture);
            ++RecordsSaveIndex;
            LastRecordingTime = current_time;
        }

        return BehaviorTree::Result::Success;
    }
}