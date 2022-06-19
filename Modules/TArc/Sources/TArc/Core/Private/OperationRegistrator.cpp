#include "TArc/Core/OperationRegistrator.h"

namespace DAVA
{
OperationID::OperationID()
    : ID(nextOperationID)
{
    nextOperationID++;
}

uint32 OperationID::nextOperationID = 0;
} // namespace DAVA