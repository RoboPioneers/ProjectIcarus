#pragma once

#include <GaiaBehaviorTree/GaiaBehaviorTree.hpp>

#include "Behaviors/ReadingCamera.hpp"
#include "Behaviors/FilteringArea.hpp"
#include "Behaviors/FilteringContours.hpp"
#include "Behaviors/FilteringLightBars.hpp"
#include "Behaviors/FilteringArmors.hpp"
#include "Behaviors/SendingCommand.hpp"
#include "Behaviors/DisplayingPicture.hpp"
#include "Behaviors/ReadingPicture.hpp"

#include <atomic>

namespace Icarus
{
    using namespace Gaia::BehaviorTree;

    class DetectionLayout : public Gaia::BehaviorTree::Containers::Sequence
    {
    public:
        BEHAVIOR(ReadingCamera, reading_camera)
        BEHAVIOR(FilteringArea, filtering_area)
        BEHAVIOR(FilteringContours, filtering_contours)
        BEHAVIOR(FilteringLightBars, filtering_light_bars)
        BEHAVIOR(FilteringArmors, filtering_armors)
        BEHAVIOR(SendingCommand, sending_command)
    };
}