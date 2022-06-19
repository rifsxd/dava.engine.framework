#pragma once

#include "REPlatform/Scene/Systems/EditorSceneSystem.h"

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>

namespace DAVA
{
class Entity;
class EditorLightSystem final : public SceneSystem, public EditorSceneSystem
{
    friend class SceneEditor2;
    friend class EditorScene;

public:
    EditorLightSystem(Scene* scene);
    ~EditorLightSystem() override;

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    void SceneDidLoaded() override;

    void Process(float32 timeElapsed) override;

    void SetCameraLightEnabled(bool enabled);
    bool GetCameraLightEnabled() const;

private:
    void UpdateCameraLightState();
    void UpdateCameraLightPosition();

    void AddCameraLightOnScene();
    void RemoveCameraLightFromScene();

private:
    Entity* cameraLight = nullptr;
    uint32 lightEntities = 0;
    bool isEnabled = true;
};

inline bool EditorLightSystem::GetCameraLightEnabled() const
{
    return isEnabled;
}
} // namespace DAVA
