#pragma once

#include "TArc/DataProcessing/DataListener.h"

#include <gmock/gmock-generated-function-mockers.h>

namespace DAVA
{
class MockListener : public DataListener
{
public:
    MOCK_METHOD2(OnDataChanged, void(const DataWrapper& wrapper, const Vector<Any>& fields));

    bool HasWrappers();
};

inline bool MockListener::HasWrappers()
{
    RemoveEmptyWrappers();
    return wrappers.empty() == false;
}
} // namespace DAVA
