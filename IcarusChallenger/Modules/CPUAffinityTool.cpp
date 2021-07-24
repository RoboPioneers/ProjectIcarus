#include "CPUAffinityTool.hpp"

#include <pthread.h>

namespace Gaia::Modules
{
    int CPUAffinityTool::SetCurrentThreadCPUAffinity(const std::vector<unsigned int>& cpus)
    {
        cpu_set_t mask;

        CPU_ZERO(&mask);

        for (const auto index : cpus)
        {
            CPU_SET(index, &mask);
        }
        return sched_setaffinity(0, sizeof(mask), &mask);
    }
}