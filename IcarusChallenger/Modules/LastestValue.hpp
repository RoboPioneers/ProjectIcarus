#pragma once

#include <list>
#include <algorithm>

namespace Icarus
{
    template <typename ValueType, unsigned int LatestCount = 5>
    class LatestValue
    {
    public:
        std::list<ValueType> Values;

    public:
        void Add(ValueType value)
        {
            while (Values.size() > LatestCount - 1)
            {
                Values.erase(Values.begin());
            }
            Values.push_back(value);
        }

        void Clear()
        {
            Values.clear();
        }

        ValueType Mean()
        {
            auto sum = static_cast<ValueType>(0);

            if (Values.empty()) return sum;

            for (const auto& value : Values)
            {
                sum += value;
            }

            return static_cast<ValueType>(static_cast<double>(sum) / static_cast<double>(Values.size()));
        }

        ValueType Middle()
        {
            if (Values.empty()) return 0;

            std::vector<ValueType> sorted_values(Values.begin(), Values.end());
            std::sort(sorted_values.begin(), sorted_values.end());

            unsigned int middle_index = sorted_values.size() / 2;
            if (middle_index >= Values.size()) middle_index = Values.size() - 1;

            return *std::next(Values.begin(), middle_index);
        }

        ValueType Latest()
        {
            if (Values.empty()) return static_cast<ValueType>(0);

            return *Values.rbegin();
        }

        ValueType LatestDelta()
        {
            if (Values.size() < 2) return static_cast<ValueType>(0);
            return *Values.rbegin() - *std::next(Values.rbegin(), 1);
        }
    };
}