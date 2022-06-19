#pragma once

#include <Command/Command.h>

namespace DAVA
{
class CommandMatchHelper
{
public:
    template <typename... Args>
    static bool IsMatch(const Command* command)
    {
        bool isMatched[] = { false, IsMatchImpl<Args>(command)... };
        return std::any_of(std::begin(isMatched), std::end(isMatched), [](bool v) { return v == true; });
    }

private:
    template <typename T>
    static bool IsMatchImpl(const Command* command)
    {
        return command->Cast<T>() != nullptr;
    }
};
} // namespace DAVA
