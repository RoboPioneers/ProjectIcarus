#pragma once

#include <GaiaBehaviorTree/GaiaBehaviorTree.hpp>

#include "Behaviors/ReadingCamera.hpp"
#include "Behaviors/SendingCommand.hpp"
#include "Behaviors/DisplayingPicture.hpp"
#include "Behaviors/ReadingPicture.hpp"
#include "Behaviors/DetectingR.hpp"
#include "Behaviors/DetectingPanel.hpp"
#include "Behaviors/ComputingSmallEnergyHitPoint.hpp"
#include "Behaviors/ComputingBigEnergyHitPoint.hpp"
#include "Behaviors/IsSmallEnergyEnable.hpp"
#include "Behaviors/IsBigEnergyEnable.hpp"
#include <atomic>

namespace Icarus
{
    using namespace Gaia::BehaviorTree;

    class DetectionLayout : public Gaia::BehaviorTree::Containers::Sequence
    {
    public:
        BEHAVIOR(ReadingCamera, reading_camera)
        DECORATOR_BEGIN(Decorators::ForceResult<Result::Success>, detection_decorator)
            CONTAINER_BEGIN(Containers::Sequence, detection_sequence)
                BEHAVIOR(DetectingR, detecting_R);
                BEHAVIOR(DetectingPanel, detecting_panel);
            CONTAINER_END(detection_sequence)
        DECORATOR_END(detection_decorator)
        DECORATOR_BEGIN(Decorators::ForceResult<Result::Success>, caculating_decorator)
            CONTAINER_BEGIN(Containers::Select, calculating_select)
                DECORATOR_BEGIN(Decorators::If, small_branch)
                    CONDITION(IsSmallEnergyEnable, check_small)
                    BEHAVIOR(ComputingSmallEnergyHitPoint, computing_small)
                DECORATOR_END(small_branch)
                DECORATOR_BEGIN(Decorators::If, big_branch)
                    CONDITION(IsBigEnergyEnable, check_big)
                    BEHAVIOR(ComputingBigEnergyHitPoint, computing_big)
                DECORATOR_END(big_branch)
            CONTAINER_END(calculating_select)
        DECORATOR_END(caculating_decorator)
        BEHAVIOR(SendingCommand, sending_command)
    };
}