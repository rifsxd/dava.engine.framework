#pragma once

#include <Reflection/Reflection.h>
#include <Reflection/ReflectedType.h>
#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectedStructure.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Functional/Function.h>

namespace DAVA
{
void ForEachField(const Reflection& r, const Function<void(Reflection::Field&& field)>& fn);

const ReflectedType* GetValueReflectedType(const Reflection& r);
const ReflectedType* GetValueReflectedType(const Any& value);

template <typename T>
const T* GetReflectedTypeMeta(const ReflectedType* type)
{
    if (type == nullptr)
    {
        return nullptr;
    }

    const ReflectedStructure* structure = type->GetStructure();
    if (structure == nullptr)
    {
        return nullptr;
    }

    if (structure->meta == nullptr)
    {
        return nullptr;
    }

    return structure->meta->GetMeta<T>();
}

template <typename T>
const T* GetTypeMeta(const Any& value)
{
    const ReflectedType* type = GetValueReflectedType(value);
    return GetReflectedTypeMeta<T>(type);
}

template <typename T>
Vector<const T*> GetTypeHierarhcyMeta(const Any& value)
{
    Vector<const T*> result;
    std::function<void(const Type*)> unpackMeta = [&result, &unpackMeta](const Type* t)
    {
        DVASSERT(t != nullptr);
        const ReflectedType* type = ReflectedTypeDB::GetByType(t);
        if (type != nullptr)
        {
            const T* m = GetReflectedTypeMeta<T>(type);
            if (m != nullptr)
            {
                result.push_back(m);
            }
        }

        const TypeInheritance* inheritance = t->GetInheritance();
        if (inheritance != nullptr)
        {
            const Vector<TypeInheritance::Info> baseTypes = inheritance->GetBaseTypes();
            for (const TypeInheritance::Info& baseType : baseTypes)
            {
                unpackMeta(baseType.type);
            }
        }
    };

    const ReflectedType* type = GetValueReflectedType(value);
    unpackMeta(type->GetType());

    return result;
}

template <typename TMeta, typename TIndex>
void EmplaceTypeMeta(const ReflectedType* type, Meta<TMeta, TIndex>&& meta)
{
    ReflectedStructure* structure = type->EditStructure();
    DVASSERT(structure != nullptr);

    if (structure->meta == nullptr)
    {
        structure->meta.reset(new ReflectedMeta());
    }

    structure->meta->Emplace(std::move(meta));
}

template <typename T, typename TMeta, typename TIndex>
void EmplaceTypeMeta(Meta<TMeta, TIndex>&& meta)
{
    const ReflectedType* type = ReflectedTypeDB::Get<T>();
    DVASSERT(type != nullptr);

    EmplaceTypeMeta(type, std::move(meta));
}

template <typename T, typename TMeta, typename TIndex>
void EmplaceFieldMeta(FastName fieldName, Meta<TMeta, TIndex>&& meta)
{
    const ReflectedType* type = ReflectedTypeDB::Get<T>();
    DVASSERT(type != nullptr);

    ReflectedStructure* structure = type->EditStructure();
    DVASSERT(structure != nullptr);

    for (std::unique_ptr<ReflectedStructure::Field>& field : structure->fields)
    {
        if (field->name == fieldName)
        {
            if (field->meta == nullptr)
            {
                field->meta.reset(new ReflectedMeta());
            }

            field->meta->Emplace(std::move(meta));
            break;
        }
    }
}
} // namespace DAVA
