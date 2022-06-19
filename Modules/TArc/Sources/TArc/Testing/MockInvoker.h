#pragma once

#include "TArc/Core/OperationInvoker.h"
#include "TArc/Testing/GMockInclude.h"

namespace DAVA
{
class MockInvoker : public OperationInvoker
{
public:
    MOCK_METHOD1(Invoke, void(uint32 operationId));
    MOCK_METHOD2(Invoke, void(uint32 operationId, const Any& a));
    MOCK_METHOD3(Invoke, void(uint32 operationId, const Any& a1, const Any& a2));
    MOCK_METHOD4(Invoke, void(uint32 operationId, const Any& a1, const Any& a2, const Any& a3));
    MOCK_METHOD5(Invoke, void(uint32 operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4));
    MOCK_METHOD6(Invoke, void(uint32 operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5));
    MOCK_METHOD7(Invoke, void(uint32 operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5, const Any& a6));
};
} // namespace DAVA
