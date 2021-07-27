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
#include "Behaviors/ReadingPictures.hpp"

#include <atomic>

namespace Icarus
{
    using namespace Gaia::BehaviorTree;

    class DetectionLayout : public Gaia::BehaviorTree::Containers::Sequence
    {
    public:
        CONTAINER_BEGIN(Containers::Sequence, upper_sequence)
            #ifdef OFFLINE
            BEHAVIOR(ReadingPictures, reading_pictures, {"Behavior"}, "Pictures/")
            #else
            BEHAVIOR(ReadingCamera, reading_camera)
            #endif
            BEHAVIOR(FilteringArea, filtering_area)
            BEHAVIOR(FilteringContours, filtering_contours)
            BEHAVIOR(FilteringLightBars, filtering_light_bars)
            BEHAVIOR(FilteringArmors, filtering_armors)
        CONTAINER_END(upper_sequence)
        BEHAVIOR(SendingCommand, sending_command)
    };
}