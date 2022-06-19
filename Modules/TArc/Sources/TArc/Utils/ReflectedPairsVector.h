#pragma once

#include <Reflection/Reflection.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
template <typename First, typename Second>
class ReflectedPairsVector;

template <typename First, typename Second>
class ReflectedPairsVectorStructureWrapper : public StructureWrapper
{
    using TCollection = ReflectedPairsVector<First, Second>;

public:
    ReflectedPairsVectorStructureWrapper()
    {
        caps.canAddField = false;
        caps.canCreateFieldValue = false;
        caps.canInsertField = false;
        caps.canRemoveField = false;
        caps.flatKeysType = Type::Instance<First>();
        caps.flatValuesType = Type::Instance<Second>();
        caps.hasDynamicStruct = false;
        caps.hasFlatStruct = true;
        caps.hasRangeAccess = false;
    }

    void Update() override
    {
    }

    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        TCollection* c = vw->GetValueObject(object).GetPtr<TCollection>();
        return c->values.empty() == false;
    }

    size_t GetFieldsCount(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        TCollection* c = vw->GetValueObject(object).GetPtr<TCollection>();
        return !c->values.size();
    }

    Reflection GetField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override
    {
        if (key.CanCast<First>())
        {
            TCollection* c = vw->GetValueObject(object).GetPtr<TCollection>();
            First k = key.Cast<First>();

            auto iter = std::find_if(c->values.begin(), c->values.end(), [&k](const std::pair<First, Second>& p) {
                return k == p.first;
            });

            if (iter != c->values.end())
            {
                Second* v = &(iter->second);
                return Reflection::Create(v);
            }
        }

        return Reflection();
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        TCollection* c = vw->GetValueObject(object).GetPtr<TCollection>();
        Vector<Reflection::Field> fields;

        for (auto& node : c->values)
        {
            Second* v = &node.second;
            fields.emplace_back(Any(node.first), Reflection::Create(v), nullptr);
        }

        return fields;
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& object, const ValueWrapper* vw, size_t first, size_t count) const override
    {
        DVASSERT(false);
        return Vector<Reflection::Field>();
    }

    const Reflection::FieldCaps& GetFieldsCaps(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return caps;
    }

    bool HasMethods(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return false;
    }

    AnyFn GetMethod(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override
    {
        return AnyFn();
    }

    Vector<Reflection::Method> GetMethods(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return Vector<Reflection::Method>();
    }

    AnyFn GetFieldCreator(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        return AnyFn();
    }

    bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const override
    {
        return false;
    }

    bool InsertField(const ReflectedObject& object, const ValueWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const override
    {
        return false;
    }

    bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override
    {
        return false;
    }

private:
    Reflection::FieldCaps caps;
};

template <typename First, typename Second>
class ReflectedPairsVector
{
public:
    Vector<std::pair<First, Second>> values;

    DAVA_REFLECTION(ReflectedPairsVector)
    {
        ReflectionRegistrator<ReflectedPairsVector>::Begin(std::make_unique<ReflectedPairsVectorStructureWrapper<First, Second>>())
        .End();
    }
};

template <typename First, typename Second>
struct AnyCompare<ReflectedPairsVector<First, Second>>
{
    using AnyT = ReflectedPairsVector<First, Second>;
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        return v1.Get<AnyT>().values == v2.Get<AnyT>().values;
    }
};

} // namespace DAVA
