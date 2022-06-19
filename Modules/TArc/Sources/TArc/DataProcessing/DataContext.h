#pragma once

#include "TArc/DataProcessing/TArcDataNode.h"

#include <Functional/Function.h>
#include <Reflection/Reflection.h>
#include <Base/BaseTypes.h>
#include <Base/Type.h>

namespace DAVA
{
class DataContext
{
public:
    DataContext() = default;
    DataContext(DataContext* parentContext);
    ~DataContext();

    void CreateData(std::unique_ptr<TArcDataNode>&& node);

    template <typename T>
    T* GetData() const; // returns nullptr if T not exists

    template <typename T>
    void DeleteData();

    TArcDataNode* GetData(const ReflectedType* type) const; // returns nullptr if T not exists
    void DeleteData(const ReflectedType* type);

    using ContextID = uint64;
    ContextID GetID() const;

    static const ContextID Empty = 0;

private:
    DataContext* parentContext = nullptr;
    UnorderedMap<const ReflectedType*, TArcDataNode*> dataMap;

    DAVA_REFLECTION(DataContext);
};
} // namespace DAVA

#define DAVAENGINE_DATACONTEXT__H
#include "TArc/DataProcessing/Private/DataContext_impl.h"
#undef DAVAENGINE_DATACONTEXT__H