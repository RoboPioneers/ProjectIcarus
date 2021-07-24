#include "GeneralMessageTranslator.hpp"
#include "CRCTool.hpp"

namespace Gaia::Modules
{
    std::string GeneralMessageTranslator::Encode(int id, const std::string &content)
    {
        std::string head, tail;
        head.resize(5);
        *reinterpret_cast<unsigned char*>(&head[0]) = 0xBE;
        *reinterpret_cast<unsigned short*>(&head[1]) = id;
        *reinterpret_cast<unsigned short*>(&head[3]) = static_cast<short>(content.size());

        tail.resize(2);
        unsigned char crc8 = CRCTool::GetCRC8(reinterpret_cast<unsigned char *>(const_cast<char *>(
                content.data())), content.size());
        *reinterpret_cast<unsigned char*>(&tail[0]) = crc8;
        *reinterpret_cast<unsigned char*>(&tail[1]) = 0xED;

        std::string buffer;
        buffer.reserve(head.size() + content.size() + tail.size());
        buffer.insert(buffer.end(), head.begin(), head.end());
        buffer.insert(buffer.end(), content.begin(), content.end());
        buffer.insert(buffer.end(), tail.begin(), tail.end());

        return buffer;
    }

    std::tuple<int, std::string> GeneralMessageTranslator::Decode(
            const std::string& package)
    {
        if (package.empty() || package.size() < 7)
        {
            return {0,{}};
        }

        for (std::size_t package_offset = 0; package_offset < package.size(); ++package_offset)
        {
            if (*reinterpret_cast<const unsigned char*>(&package[package_offset]) != 0xBE) continue; // dropped for missing package head mark.
            unsigned short id = *reinterpret_cast<const unsigned short*>(
                    &package[package_offset + 1]);
            unsigned short size = *reinterpret_cast<const unsigned short*>(
                    &package[package_offset + 3]);
            // 7 is calculated by 2 * sizeof(unsigned shor) + 4 * sizeof(unsigned char)
            if (package.size() < package_offset + 7 + size)
            {
                continue; // dropped for incomplete message.
            }
            auto* content = const_cast<char *>(&package[package_offset + 5]);
            unsigned char crc8 = package[package_offset + 5 + size];
            if (*reinterpret_cast<const unsigned char*>(&package[package_offset + 6 + size]) != 0xED) continue; // dropped for missing package tail mark.

            if (crc8 != CRCTool::GetCRC8(reinterpret_cast<unsigned char *>(content), size))
            {
                continue;   // dropped for wrong CRC.
            }

            auto content_begin_iterator = package.begin() + static_cast<long>(package_offset) + 5;
            auto content_end_iterator = package.begin() + static_cast<long>(package_offset) + 5 + size;

            std::string result;

            result.reserve(size);
            result.insert(result.end(), content_begin_iterator, content_end_iterator);

            package_offset += 6 + size; // package_offset will increase by 1 when a turn in this loop is finished.

            return {id, result};
        }
        return {0, ""};
    }
}