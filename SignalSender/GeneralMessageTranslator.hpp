#pragma once

#include <tuple>
#include <string>

namespace Gaia::Modules
{
    class GeneralMessageTranslator
    {
    public:
        static std::string Encode(int id, const std::string& content);
        static std::tuple<int, std::string> Decode(const std::string&);
    };
}
