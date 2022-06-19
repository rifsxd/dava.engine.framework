#include "TArc/SharedModules/SettingsModule/Private/SettingsManager.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/SettingsNode.h"
#include "TArc/Utils/ReflectionHelpers.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedTypeDB.h>

#include <Logger/Logger.h>
#include <Base/Type.h>
#include <Base/TypeInheritance.h>

namespace DAVA
{
namespace SettingsManagerDetails
{
const String propertiesNodeName("SettingsManager");
}
SettingsContainer::SettingsContainer()
{
}

DAVA_REFLECTION_IMPL(SettingsContainer)
{
    ReflectionRegistrator<SettingsContainer>::Begin()
    .Field("Settings", &SettingsContainer::nodes)
    .End();
}

SettingsManager::~SettingsManager()
{
    Save(accessor->CreatePropertiesNode(SettingsManagerDetails::propertiesNodeName));
}

void SettingsManager::CreateSettings()
{
    const ReflectedType* settingsNodeRefType = ReflectedTypeDB::Get<SettingsNode>();
    const Type* settingsNodeType = settingsNodeRefType->GetType();
    const TypeInheritance* typeInheritance = settingsNodeType->GetInheritance();

    CreateSettingsImpl(settingsNodeType->GetInheritance());

    std::sort(settings->nodes.begin(), settings->nodes.end(), [](SettingsNode* node1, SettingsNode* node2)
              {
                  const ReflectedType* type1 = ReflectedTypeDB::GetByPointer(node1);
                  const ReflectedType* type2 = ReflectedTypeDB::GetByPointer(node2);

                  const M::SettingsSortKey* sortKey1 = GetTypeMeta<M::SettingsSortKey>(node1);
                  const M::SettingsSortKey* sortKey2 = GetTypeMeta<M::SettingsSortKey>(node2);

                  if (sortKey1 != nullptr && sortKey2 != nullptr)
                  {
                      if (sortKey1->sortKey != sortKey2->sortKey)
                      {
                          return sortKey1->sortKey > sortKey2->sortKey;
                      }
                  }
                  else if (sortKey1 != nullptr && sortKey2 == nullptr)
                  {
                      return true;
                  }
                  else if (sortKey1 == nullptr && sortKey2 != nullptr)
                  {
                      return false;
                  }

                  return type1->GetPermanentName() < type2->GetPermanentName();
              });

    Load(accessor->CreatePropertiesNode(SettingsManagerDetails::propertiesNodeName));
}

void SettingsManager::ResetToDefault()
{
    DataContext* ctx = accessor->GetGlobalContext();

    for (size_t i = 0; i < settings->nodes.size(); ++i)
    {
        SettingsNode* node = settings->nodes[i];
        Reflection refl = Reflection::Create(ReflectedObject(node));
        if (refl.GetMeta<M::HiddenField>() != nullptr)
        {
            continue;
        }

        const ReflectedType* type = ReflectedTypeDB::GetByPointer(node);
        SettingsNode* newNode = type->CreateObject(ReflectedType::CreatePolicy::ByPointer).Cast<SettingsNode*>();
        Reflection newNodeReflection = Reflection::Create(ReflectedObject(newNode));
        Vector<Reflection::Field> fields = newNodeReflection.GetFields();
        for (Reflection::Field& f : fields)
        {
            DVASSERT(f.ref.GetValueType()->IsPointer() == false && f.ref.GetValueType()->IsReference() == false);

            if (f.ref.GetMeta<M::HiddenField>() != nullptr && f.ref.GetMeta<M::ForceResetToDefault>() == nullptr)
            {
                continue;
            }

            refl.GetField(f.key).SetValue(f.ref.GetValue());
        }

        delete newNode;
    }
}

void SettingsManager::OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& fields)
{
    auto iter = std::find_if(wrappers.begin(), wrappers.end(), [&wrapper](const std::pair<DataWrapper, SettingsNode*>& node)
                             {
                                 return node.first == wrapper;
                             });

    DVASSERT(iter != wrappers.end());
    Save(accessor->CreatePropertiesNode(SettingsManagerDetails::propertiesNodeName), iter->second);
}

void SettingsManager::Load(const PropertiesItem& settingsProperties)
{
    for (SettingsNode* node : settings->nodes)
    {
        const ReflectedType* type = ReflectedTypeDB::GetByPointer(node);
        PropertiesItem propeties = settingsProperties.CreateSubHolder(type->GetPermanentName());
        node->Load(propeties);
    }
}

void SettingsManager::Save(const PropertiesItem& settingsProperties) const
{
    for (SettingsNode* node : settings->nodes)
    {
        Save(settingsProperties, node);
    }
}

void SettingsManager::Save(const PropertiesItem& settingsProperties, SettingsNode* node) const
{
    const ReflectedType* type = ReflectedTypeDB::GetByPointer(node);
    PropertiesItem propeties = settingsProperties.CreateSubHolder(type->GetPermanentName());
    node->Save(propeties);
}

void SettingsManager::CreateSettingsImpl(const TypeInheritance* inheritance)
{
    const Vector<TypeInheritance::Info>& derivedClasses = inheritance->GetDerivedTypes();

    for (const TypeInheritance::Info& typeInfo : derivedClasses)
    {
        const ReflectedType* nodeType = ReflectedTypeDB::GetByType(typeInfo.type);
        DVASSERT(nodeType != nullptr, Format("Settings type without reflected type detected: %s", typeInfo.type->GetName()).c_str());
        DVASSERT(nodeType->GetCtor(nodeType->GetType()->Pointer()) != nullptr);

        SettingsNode* node = nodeType->CreateObject(ReflectedType::CreatePolicy::ByPointer).Cast<SettingsNode*>();
        settings->nodes.push_back(node);
        accessor->GetGlobalContext()->CreateData(std::unique_ptr<TArcDataNode>(node));
        DataWrapper wrapper = accessor->CreateWrapper(nodeType);
        wrapper.SetListener(this);
        wrappers.emplace_back(wrapper, node);

        CreateSettingsImpl(typeInfo.type->GetInheritance());
    }
}

SettingsManager::SettingsManager(ContextAccessor* accessor_)
    : accessor(accessor_)
    , settings(new SettingsContainer())
{
}
} // namespace DAVA
