#pragma once

#include <vector>

namespace Gaia::Modules
{
    class CPUAffinityTool
    {
    public:
        static int SetCurrentThreadCPUAffinity(const std::vector<unsigned int>& cpus);
    };
}