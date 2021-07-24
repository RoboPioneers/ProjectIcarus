#pragma once

#include <array>

namespace Gaia::Modules
{
    class CRCTool
    {
    public:
        static std::array<unsigned char, 256> CRC8Table;
        static std::array<unsigned short, 256> CRC16Table;

        static unsigned char GetCRC8(unsigned char* data, unsigned int length,
                                     unsigned char initializer = 0xff);

        static unsigned short GetCRC16(unsigned char* data, unsigned int length,
                                       unsigned short initializer = 0xffff);
    };
}
