#include "TArc/DataProcessing/SettingsNode.h"
#include "TArc/DataProcessing/PropertiesHolder.h"

#include <Reflection/Reflection.h>
#include <Base/Any.h>

namespace DAVA
{
namespace SettingsNodeDetail
{
void ReflectedLoadImpl(Reflection::Field& field, const PropertiesItem& node)
{
    Vector<Reflection::Field> fields = field.ref.GetFields();
    if (fields.empty())
    {
        Any value = node.Get(field.key.Cast<String>(), field.ref.GetValue(), field.ref.GetValueType());
        if (value.IsEmpty() == false)
        {
            field.ref.SetValueWithCast(value);
        }
    }
    else
    {
        for (Reflection::Field& f : fields)
        {
            PropertiesItem fieldItem = node.CreateSubHolder(f.key.Cast<String>());
            ReflectedLoadImpl(f, fieldItem);
        }
    }
}

void ReflectedSaveImpl(Reflection::Field& field, PropertiesItem& node)
{
    Vector<Reflection::Field> fields = field.ref.GetFields();
    if (fields.empty())
    {
        node.Set(field.key.Cast<String>(), field.ref.GetValue());
    }
    else
    {
        for (Reflection::Field& f : fields)
        {
            PropertiesItem fieldItem = node.CreateSubHolder(f.key.Cast<String>());
            ReflectedSaveImpl(f, fieldItem);
        }
    }
}
} // namespace SettingsNodeDetail

void SettingsNode::Load(const PropertiesItem& settingsNode)
{
    ReflectedLoad(settingsNode);
}

void SettingsNode::Save(PropertiesItem& settingsNode) const
{
    ReflectedSave(settingsNode);
}

void SettingsNode::ReflectedLoad(const PropertiesItem& settingsNode)
{
    Reflection::Field field;
    field.ref = Reflection::Create(ReflectedObject(this));
    field.key = String("Root");

    SettingsNodeDetail::ReflectedLoadImpl(field, settingsNode);
}

void SettingsNode::ReflectedSave(PropertiesItem& settingsNode) const
{
    Reflection::Field field;
    field.ref = Reflection::Create(ReflectedObject(this));
    field.key = String("Root");

    SettingsNodeDetail::ReflectedSaveImpl(field, settingsNode);
}

DAVA_VIRTUAL_REFLECTION_IMPL(SettingsNode)
{
    ReflectionRegistrator<SettingsNode>::Begin()
    .End();
}
} // namespace DAVA
