#pragma once

#include "Functional/Signal.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
class TArcDataNode : public ReflectionBase
{
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(TArcDataNode)
    {
    }

public:
    virtual ~TArcDataNode() = default;
};
} // namespace DAVA
