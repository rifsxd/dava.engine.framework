#pragma once

#include "Base/Any.h"

namespace DAVA
{
class OperationInvoker
{
public:
    virtual void Invoke(uint32 operationId) = 0;
    virtual void Invoke(uint32 operationId, const Any& a) = 0;
    virtual void Invoke(uint32 operationId, const Any& a1, const Any& a2) = 0;
    virtual void Invoke(uint32 operationId, const Any& a1, const Any& a2, const Any& a3) = 0;
    virtual void Invoke(uint32 operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4) = 0;
    virtual void Invoke(uint32 operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5) = 0;
    virtual void Invoke(uint32 operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5, const Any& a6) = 0;
};
} // namespace DAVA
