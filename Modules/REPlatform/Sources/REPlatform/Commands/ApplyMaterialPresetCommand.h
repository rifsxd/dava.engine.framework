#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Base/RefPtr.h>
#include <Base/ScopedPtr.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
class KeyedArchive;
class Scene;
class NMaterial;
class SerializationContext;
class FilePath;

class ApplyMaterialPresetCommand : public RECommand
{
public:
    enum eMaterialPart : uint32
    {
        NOTHING = 0,

        TEMPLATE = 1 << 0,
        GROUP = 1 << 1,
        PROPERTIES = 1 << 2,
        TEXTURES = 1 << 3,

        ALL = TEMPLATE | GROUP | PROPERTIES | TEXTURES
    };

    ApplyMaterialPresetCommand(const FilePath& presetPath, NMaterial* material, Scene* scene);

    bool IsValidPreset() const;
    void Init(uint32 materialParts);

    void Undo() override;
    void Redo() override;
    bool IsClean() const override;

    static void StoreCurrentConfigPreset(KeyedArchive* archive, NMaterial* material, const SerializationContext& context);
    static void StoreAllConfigsPreset(KeyedArchive* archive, NMaterial* material, const SerializationContext& context);

private:
    void LoadMaterialPreset(KeyedArchive* archive, uint32 parts, bool loadForUndo);
    void PrepareSerializationContext(SerializationContext& context);

private:
    ScopedPtr<KeyedArchive> redoInfo;
    ScopedPtr<KeyedArchive> undoInfo;
    RefPtr<NMaterial> material;
    Scene* scene;
    uint32 materialParts = 0;

    DAVA_VIRTUAL_REFLECTION(ApplyMaterialPresetCommand, RECommand);
};

} // namespace DAVA
