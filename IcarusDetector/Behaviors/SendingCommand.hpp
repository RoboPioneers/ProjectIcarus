#pragma once

#include "../Framework/ProcessorBase.hpp"
#include <GaiaSerialIOClient/GaiaSerialIOClient.hpp>
#include <opencv2/opencv.hpp>

namespace Icarus
{
    /**
     * @brief Sending the command.
     * @details
     *  Configuration:
     *      (float)SerialName
     *  Blackboard:
     *      Input -> (cv::Point2i)HitPoint, (int)HitCommand
     */
    class SendingCommand : public ProcessorBase
    {
        REFLECT_TYPE(Icarus, ProcessorBase)
    protected:
        /// Client for serial io service.
        std::unique_ptr<Gaia::SerialIO::SerialClient> Serial;

        cv::Point2i* HitPoint {nullptr};
        int* HitCommand {nullptr};
        double* HitDistance {nullptr};
        int* MotionStatus {nullptr};

        void OnInitialize() override;

        BehaviorTree::Result OnExecute() override;
    };
}
