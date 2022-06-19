#include "TArc/DataProcessing/DataListener.h"

namespace DAVA
{
DataListener::~DataListener()
{
    Vector<DataWrapper> wrappersCopy;
    for (const DataWrapperNode& node : wrappers)
    {
        wrappersCopy.push_back(DataWrapper(node.weak));
    }

    wrappers.clear();
    for (DataWrapper& wrapper : wrappersCopy)
    {
        wrapper.ClearListener(this);
    }
}

void DataListener::AddWrapper(DataWrapper::DataWrapperWeak weak)
{
    RemoveEmptyWrappers();
    DataWrapperNode node;
    node.id = weak.impl.lock().get();
    node.weak = weak;
    wrappers.emplace(node);
}

void DataListener::RemoveWrapper(DataWrapper::DataWrapperWeak weak)
{
    RemoveEmptyWrappers();
    DataWrapperNode node;
    node.id = weak.impl.lock().get();
    node.weak = weak;
    wrappers.erase(node);
}

void DataListener::RemoveEmptyWrappers()
{
    auto iter = wrappers.begin();
    while (iter != wrappers.end())
    {
        if (iter->weak.impl.lock() == nullptr)
        {
            iter = wrappers.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}
} // namespace DAVA
