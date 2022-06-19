#include "REPlatform/Commands/MaterialConfigCommands.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
MaterialConfigModify::MaterialConfigModify(NMaterial* material_, const String& text)
    : RECommand(text)
    , material(SafeRetain(material_))
{
    DVASSERT(material);
}

MaterialConfigModify::~MaterialConfigModify()
{
    SafeRelease(material);
}

DAVA_VIRTUAL_REFLECTION_IMPL(MaterialConfigModify)
{
    ReflectionRegistrator<MaterialConfigModify>::Begin()
    .End();
}

MaterialChangeCurrentConfig::MaterialChangeCurrentConfig(NMaterial* material, uint32 newCurrentConfigIndex)
    : MaterialConfigModify(material, "Change current material config")
    , newCurrentConfig(newCurrentConfigIndex)
    , oldCurrentConfig(material->GetCurrentConfigIndex())
{
}

void MaterialChangeCurrentConfig::Undo()
{
    material->SetCurrentConfigIndex(oldCurrentConfig);
}

void MaterialChangeCurrentConfig::Redo()
{
    material->SetCurrentConfigIndex(newCurrentConfig);
}

DAVA_VIRTUAL_REFLECTION_IMPL(MaterialChangeCurrentConfig)
{
    ReflectionRegistrator<MaterialChangeCurrentConfig>::Begin()
    .End();
}

MaterialRemoveConfig::MaterialRemoveConfig(NMaterial* material, uint32 configIndex_)
    : MaterialConfigModify(material, "Remove material config")
    , config(material->GetConfig(configIndex_))
    , configIndex(configIndex_)
{
    uint32 configCount = material->GetConfigCount();
    uint32 newCurrConfig = material->GetCurrentConfigIndex();
    DVASSERT(configCount > 1);
    DVASSERT(configIndex < configCount);
    if ((configIndex == newCurrConfig && configIndex == configCount - 1) ||
        configIndex < newCurrConfig)
    {
        --newCurrConfig;
    }

    changeCurrentConfigCommand.reset(new MaterialChangeCurrentConfig(material, newCurrConfig));
}

void MaterialRemoveConfig::Undo()
{
    material->InsertConfig(configIndex, config);
    changeCurrentConfigCommand->Undo();
}

void MaterialRemoveConfig::Redo()
{
    material->RemoveConfig(configIndex);
    changeCurrentConfigCommand->Redo();
}

DAVA_VIRTUAL_REFLECTION_IMPL(MaterialRemoveConfig)
{
    ReflectionRegistrator<MaterialRemoveConfig>::Begin()
    .End();
}

MaterialCreateConfig::MaterialCreateConfig(NMaterial* material, const MaterialConfig& config_)
    : MaterialConfigModify(material, "Create material config")
    , config(config_)
{
    changeCurrentConfigCommand.reset(new MaterialChangeCurrentConfig(material, material->GetConfigCount()));
}

void MaterialCreateConfig::Undo()
{
    changeCurrentConfigCommand->Undo();
    DVASSERT(configIndex != -1);
    material->RemoveConfig(configIndex);
    configIndex = -1;
}

void MaterialCreateConfig::Redo()
{
    DVASSERT(configIndex == -1);
    configIndex = material->GetConfigCount();
    material->InsertConfig(configIndex, config);
    changeCurrentConfigCommand->Redo();
}

DAVA_VIRTUAL_REFLECTION_IMPL(MaterialCreateConfig)
{
    ReflectionRegistrator<MaterialCreateConfig>::Begin()
    .End();
}

} // namespace DAVA
