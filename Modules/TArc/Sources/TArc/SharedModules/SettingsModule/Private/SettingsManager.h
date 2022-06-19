#pragma once

#include "TArc/DataProcessing/DataWrapper.h"
#include "TArc/DataProcessing/DataListener.h"

#include <Reflection/Reflection.h>
#include <Base/Vector.h>

namespace DAVA
{
class TypeInheritance;
class PropertiesItem;
class ContextAccessor;
class SettingsNode;

class SettingsContainer
{
public:
    SettingsContainer();

private:
    friend class SettingsManager;
    Vector<SettingsNode*> nodes;

    DAVA_REFLECTION(SettingsContainer);
};

class SettingsManager final : public DataListener
{
public:
    ~SettingsManager();

    void CreateSettings();
    void ResetToDefault();

protected:
    void OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& fields);

private:
    void Load(const PropertiesItem& settingsProperties);
    void Save(const PropertiesItem& settingsProperties) const;
    void Save(const PropertiesItem& settingsProperties, SettingsNode* node) const;
    void CreateSettingsImpl(const TypeInheritance* inheritance);

    friend class SettingsModule;
    SettingsManager(ContextAccessor* accessor);

    ContextAccessor* accessor = nullptr;
    std::unique_ptr<SettingsContainer> settings;
    Vector<std::pair<DataWrapper, SettingsNode*>> wrappers;
};
} // namespace DAVA
