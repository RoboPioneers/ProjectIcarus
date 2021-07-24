#include <GaiaFramework/GaiaFramework.hpp>

#include "IcarusChallenger.hpp"

int main(int arguments_count, char** arguments)
{
    using namespace Gaia::Framework;

    Launch<Icarus::IcarusChallenger>(arguments_count, arguments);

    return 0;
}