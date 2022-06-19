#pragma once

#include "REPlatform/Scene/Systems/EditorSceneSystem.h"
#include "REPlatform/DataNodes/Settings/RESettings.h"

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>

namespace DAVA
{
class RECommandNotificationObject;
class NMaterial;
class Entity;
class RenderBatch;
class EditorMaterialSystem : public SceneSystem, public EditorSceneSystem
{
public:
    EditorMaterialSystem(Scene* scene);
    ~EditorMaterialSystem();

    const Set<NMaterial*>& GetTopParents() const;

    Entity* GetEntity(NMaterial*) const;
    const RenderBatch* GetRenderBatch(NMaterial*) const;

    void SetLightViewMode(CommonInternalSettings::MaterialLightViewMode viewMode, bool set);
    bool GetLightViewMode(CommonInternalSettings::MaterialLightViewMode viewMode) const;

    void SetLightViewMode(int fullViewMode);
    int GetLightViewMode();

    void SetLightmapCanvasVisible(bool enable);
    bool IsLightmapCanvasVisible() const;

    bool HasMaterial(NMaterial*) const;

protected:
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

private:
    struct MaterialMapping
    {
        MaterialMapping() = default;
        MaterialMapping(Entity* entity_, RenderBatch* renderBatch_);

        ~MaterialMapping();

        MaterialMapping(const MaterialMapping& other);
        MaterialMapping(MaterialMapping&& other) = delete;

        MaterialMapping& operator=(const MaterialMapping& other);
        MaterialMapping& operator=(MaterialMapping&& other) = delete;

        Entity* entity = nullptr;
        RenderBatch* renderBatch = nullptr;
    };
    using MaterialToObjectsMap = Map<NMaterial*, MaterialMapping>;

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    void AddMaterials(Entity* entity);
    void AddMaterial(NMaterial*, const MaterialMapping& mapping);

    void RemoveMaterial(NMaterial* material);

    void ApplyViewMode();
    void ApplyViewMode(NMaterial* material);

    bool IsEditable(NMaterial* material) const;

private:
    MaterialToObjectsMap materialToObjectsMap;
    Set<NMaterial*> ownedParents;
    uint32 curViewMode = CommonInternalSettings::LIGHTVIEW_ALL;
    bool showLightmapCanvas = false;
};
} // namespace DAVA