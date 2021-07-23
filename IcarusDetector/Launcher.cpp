#include <GaiaFramework/GaiaFramework.hpp>

#include "IcarusDetector.hpp"

int main(int arguments_count, char** arguments)
{
    using namespace Gaia::Framework;

    Launch<Icarus::IcarusDetector>(arguments_count, arguments);

    return 0;
}