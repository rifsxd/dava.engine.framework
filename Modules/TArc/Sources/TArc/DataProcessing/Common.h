#pragma once

#include <Reflection/ReflectedType.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Base/FastName.h>

namespace DAVA
{
struct FieldDescriptor
{
    const ReflectedType* type = nullptr;
    FastName fieldName;

    FieldDescriptor() = default;

    FieldDescriptor(const ReflectedType* type_, const FastName& fieldName_)
        : type(type_)
        , fieldName(fieldName_)
    {
    }

    bool IsEmpty() const;
};

inline bool FieldDescriptor::IsEmpty() const
{
    return type == nullptr || fieldName.IsValid() == false;
}

template <typename T>
FieldDescriptor MakeFieldDescriptor(const FastName& fieldName)
{
    FieldDescriptor descr;
    descr.type = ReflectedTypeDB::Get<T>();
    descr.fieldName = fieldName;
    return descr;
}

template <typename T>
FieldDescriptor MakeFieldDescriptor(const char* fieldName)
{
    FieldDescriptor descr;
    descr.type = ReflectedTypeDB::Get<T>();
    descr.fieldName = FastName(fieldName);
    return descr;
}
} // namespace DAVA
