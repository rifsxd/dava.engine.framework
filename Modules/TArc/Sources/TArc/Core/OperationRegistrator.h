#pragma once

#include "TArc/Core/OperationRegistrator.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
class OperationID
{
public:
    OperationID();

    const uint32 ID;

private:
    static uint32 nextOperationID;
};
} // namespace DAVA

#define DECLARE_OPERATION_ID(name) \
    extern DAVA::OperationID name

#define IMPL_OPERATION_ID(name) \
    DAVA::OperationID name
