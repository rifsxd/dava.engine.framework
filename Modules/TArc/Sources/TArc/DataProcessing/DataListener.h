#pragma once

#include "DataWrapper.h"

#include "Base/BaseTypes.h"

namespace DAVA
{
class DataListener
{
public:
    virtual ~DataListener();
    virtual void OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& fields) = 0;

private:
    friend class DataWrapper;
    friend class MockListener;
    void AddWrapper(DataWrapper::DataWrapperWeak weak);
    void RemoveWrapper(DataWrapper::DataWrapperWeak weak);
    void RemoveEmptyWrappers();

private:
    struct DataWrapperNode
    {
        void* id = nullptr;
        DataWrapper::DataWrapperWeak weak;
    };

    struct DataWrapperLess
    {
        bool operator()(const DataWrapperNode& w1, const DataWrapperNode& w2) const
        {
            return w1.id < w2.id;
        }
    };

    Set<DataWrapperNode, DataWrapperLess> wrappers;
};
} // namespace DAVA
