#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Render/Material/NMaterial.h>

namespace DAVA
{
class MaterialConfigModify : public RECommand
{
public:
    MaterialConfigModify(NMaterial* material, const String& text = String());
    ~MaterialConfigModify();

    NMaterial* GetMaterial() const;
    Entity* GetEntity() const;

protected:
    NMaterial* material;
    DAVA_VIRTUAL_REFLECTION(MaterialConfigModify, RECommand);
};

class MaterialChangeCurrentConfig : public MaterialConfigModify
{
public:
    MaterialChangeCurrentConfig(NMaterial* material, uint32 newCurrentConfigIndex);

    void Undo() override;
    void Redo() override;

private:
    uint32 newCurrentConfig = static_cast<uint32>(-1);
    uint32 oldCurrentConfig = static_cast<uint32>(-1);

    DAVA_VIRTUAL_REFLECTION(MaterialChangeCurrentConfig, MaterialConfigModify);
};

class MaterialRemoveConfig : public MaterialConfigModify
{
public:
    MaterialRemoveConfig(NMaterial* material, uint32 configIndex);

    void Undo() override;
    void Redo() override;

private:
    MaterialConfig config;
    uint32 configIndex;
    std::unique_ptr<MaterialChangeCurrentConfig> changeCurrentConfigCommand;

    DAVA_VIRTUAL_REFLECTION(MaterialRemoveConfig, MaterialConfigModify);
};

class MaterialCreateConfig : public MaterialConfigModify
{
public:
    MaterialCreateConfig(NMaterial* material, const MaterialConfig& config);

    void Undo() override;
    void Redo() override;

private:
    MaterialConfig config;
    uint32 configIndex = static_cast<uint32>(-1);
    std::unique_ptr<MaterialChangeCurrentConfig> changeCurrentConfigCommand;

    DAVA_VIRTUAL_REFLECTION(MaterialCreateConfig, MaterialConfigModify);
};

inline NMaterial* MaterialConfigModify::GetMaterial() const
{
    return material;
}

inline Entity* MaterialConfigModify::GetEntity() const
{
    return nullptr;
}
} // namespace DAVA
