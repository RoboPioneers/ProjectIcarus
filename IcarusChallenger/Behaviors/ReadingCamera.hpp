#pragma once

#include "../Framework/ProcessorBase.hpp"
#include <opencv2/opencv.hpp>
#include <GaiaCameraClient/GaiaCameraClient.hpp>

namespace Icarus
{
    /// Read a picture from the camera.
    class ReadingCamera : public ProcessorBase
    {
        REFLECT_TYPE(Icarus, ProcessorBase)

    private:
        /// Time point of last FPS measuring.
        std::chrono::system_clock::time_point LastMeasuringTime;
        /// Accumulated frames counter from last FPS measuring.
        unsigned int AccumulatedFramesCount {0};

    protected:

        /// Main picture in the blackboard.
        cv::Mat* MainPicture {nullptr};

        /// Client for camera service.
        std::shared_ptr<Gaia::CameraService::CameraClient> CameraClient {nullptr};
        /// Reader for camera picture.
        std::shared_ptr<Gaia::CameraService::CameraReader> CameraReader {nullptr};

        /// Initialize camera facilities.
        void OnInitialize() override;

        /// Read a picture.
        BehaviorTree::Result OnExecute() override;
    };
}