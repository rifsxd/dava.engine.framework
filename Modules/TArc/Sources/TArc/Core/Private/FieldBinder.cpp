#include "TArc/Core/FieldBinder.h"
#include "TArc/DataProcessing/DataListener.h"
#include <Reflection/Reflection.h>
#include <Reflection/ReflectedTypeDB.h>

namespace DAVA
{
class UniversalDataListener : public DataListener
{
public:
    UniversalDataListener(const Reflection& model_, ContextAccessor* accessor_)
        : type(nullptr)
        , model(model_)
        , accessor(accessor_)
    {
        const Type* t = model.GetValueType();
        if (t->IsPointer())
        {
            t = t->Deref();
        }
        type = ReflectedTypeDB::GetByType(t);
        wrapper = accessor->CreateWrapper([this](const DataContext*) {
            return model;
        });
        wrapper.SetListener(this);
    }

    UniversalDataListener(const ReflectedType* type_, ContextAccessor* accessor_)
        : type(type_)
        , accessor(accessor_)
    {
        wrapper = accessor->CreateWrapper(type);
        wrapper.SetListener(this);
    }

    UniversalDataListener(const UniversalDataListener& other) = delete;
    UniversalDataListener& operator=(const UniversalDataListener& other) = delete;

    UniversalDataListener(UniversalDataListener&& other)
    {
        std::swap(type, other.type);
        model = std::move(other.model);
        listeners = std::move(other.listeners);
        std::swap(accessor, other.accessor);

        other.wrapper.SetListener(nullptr);
        wrapper = std::move(other.wrapper);
        wrapper.SetListener(this);
    }

    UniversalDataListener& operator=(UniversalDataListener&& other) = delete;

    ~UniversalDataListener()
    {
        wrapper.SetListener(nullptr);
    }

    void OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& changedFields) override
    {
        if (!wrapper.HasData())
        {
            // Data was removed. Invalidate all listeners by empty value
            DVASSERT(changedFields.empty());
            for (auto& listener : listeners)
            {
                for (auto& fn : listener.second)
                {
                    fn(Any());
                }
            }
            return;
        }

        Reflection reflection = model;
        if (reflection.IsValid() == false)
        {
            DataContext* ctx = accessor->GetActiveContext();
            if (ctx == nullptr)
            {
                ctx = accessor->GetGlobalContext();
            }
            DVASSERT(ctx);
            TArcDataNode* dataNode = ctx->GetData(type);
            DVASSERT(dataNode);
            reflection = Reflection::Create(dataNode);
        }

        if (changedFields.empty())
        {
            // Data appeared. All fields was changed
            Vector<Reflection::Field> dataFields = reflection.GetFields();
            UnorderedMap<FastName, size_t> intermidiateFieldMap;
            for (size_t i = 0; i < dataFields.size(); ++i)
            {
                Reflection::Field& dataField = dataFields[i];
                DVASSERT(dataField.key.CanCast<FastName>());
                intermidiateFieldMap.emplace(dataField.key.Cast<FastName>(), i);
            }

            for (auto& listener : listeners)
            {
                auto iter = intermidiateFieldMap.find(listener.first);
                if (iter == intermidiateFieldMap.end())
                {
                    for (auto& fn : listener.second)
                    {
                        fn(Any());
                    }
                }
                else
                {
                    Any value = dataFields[iter->second].ref.GetValue();
                    for (auto& fn : listener.second)
                    {
                        fn(value);
                    }
                }
            }
        }
        else
        {
            for (const Any& fieldAnyName : changedFields)
            {
                DVASSERT(fieldAnyName.CanCast<FastName>());
                auto iter = listeners.find(fieldAnyName.Cast<FastName>());
                if (iter != listeners.end())
                {
                    Reflection field = reflection.GetField(fieldAnyName);
                    Any value = field.GetValue();
                    for (auto& fn : iter->second)
                    {
                        fn(value);
                    }
                }
            }
        }
    }

    void BindField(FastName fieldName, const Function<void(const Any&)>& fn)
    {
        listeners[fieldName].push_back(fn);
    }

    const ReflectedType* GetType() const
    {
        return type;
    }

    bool IsSameReflectedObject(const Reflection& ref) const
    {
        return model.GetValueObject() == ref.GetValueObject();
    }

private:
    friend class FieldBinder;
    const ReflectedType* type = nullptr;
    Reflection model;
    UnorderedMap<FastName, Vector<Function<void(const Any&)>>> listeners;
    ContextAccessor* accessor = nullptr;
    DataWrapper wrapper;
};

class FieldBinder::Impl
{
public:
    ContextAccessor* accessor = nullptr;
    Vector<std::unique_ptr<UniversalDataListener>> listeners;
};

FieldBinder::FieldBinder(ContextAccessor* accessor_)
    : impl(std::make_unique<Impl>())
{
    impl->accessor = accessor_;
}

FieldBinder::~FieldBinder() = default;

void FieldBinder::BindField(const FieldDescriptor& fieldDescr, const Function<void(const Any&)>& fn)
{
    for (std::unique_ptr<UniversalDataListener>& listener : impl->listeners)
    {
        if (listener->GetType() == fieldDescr.type)
        {
            listener->BindField(fieldDescr.fieldName, fn);
            return;
        }
    }

    impl->listeners.push_back(std::make_unique<UniversalDataListener>(fieldDescr.type, impl->accessor));
    impl->listeners.back()->BindField(fieldDescr.fieldName, fn);
}

void FieldBinder::BindField(const Reflection& model, const FastName& fieldName, const Function<void(const Any&)>& fn)
{
    for (std::unique_ptr<UniversalDataListener>& listener : impl->listeners)
    {
        if (listener->GetType() == ReflectedTypeDB::GetByType(model.GetValueType()))
        {
            DVASSERT(listener->IsSameReflectedObject(model));
            listener->BindField(fieldName, fn);
            return;
        }
    }

    impl->listeners.emplace_back(std::make_unique<UniversalDataListener>(model, impl->accessor));
    impl->listeners.back()->BindField(fieldName, fn);
}

void FieldBinder::SetValue(const FieldDescriptor& fieldDescr, const Any& v)
{
    for (std::unique_ptr<UniversalDataListener>& listener : impl->listeners)
    {
        if (listener->GetType() == fieldDescr.type)
        {
            listener->wrapper.SetFieldValue(fieldDescr.fieldName, v);
        }
    }
}

Any FieldBinder::GetValue(const FieldDescriptor& fieldDescr) const
{
    for (const std::unique_ptr<UniversalDataListener>& listener : impl->listeners)
    {
        if (listener->GetType() == fieldDescr.type)
        {
            return listener->wrapper.GetFieldValue(fieldDescr.fieldName);
        }
    }
    return Any();
}
} // namespace DAVA
