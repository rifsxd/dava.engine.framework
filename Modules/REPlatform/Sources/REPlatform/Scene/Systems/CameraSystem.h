#pragma once

#include "REPlatform/Scene/Systems/EditorSceneSystem.h"

#include <Entity/SceneSystem.h>
#include <Render/Highlevel/Camera.h>
#include <UI/UIEvent.h>

namespace DAVA
{
class SelectableGroup;
class WASDControllerSystem;
class RotationControllerSystem;
class HoodSystem;
class SceneCollisionSystem;
class PropertiesHolder;
class ContextAccessor;

class SceneCameraSystem : public SceneSystem, public EditorSceneSystem
{
    friend class SceneEditor2;
    friend class EditorLightSystem;

public:
    SceneCameraSystem(Scene* scene);
    ~SceneCameraSystem();

    Camera* GetCurCamera() const;

    Vector3 GetPointDirection(const Vector2& point) const;
    Vector3 GetCameraPosition() const;
    Vector3 GetCameraDirection() const;
    void GetRayTo2dPoint(const Vector2& point, float32 maxRayLen, Vector3& outPointFrom, Vector3& outPointTo) const;

    float32 GetMoveSpeed();

    uint32 GetActiveSpeedIndex();
    void SetMoveSpeedArrayIndex(uint32 index);

    void SetViewportRect(const Rect& rect);
    const Rect& GetViewportRect() const;

    void LookAt(const AABBox3& box);
    void MoveTo(const Vector3& pos);
    void MoveTo(const Vector3& pos, const Vector3& target);

    Vector2 GetScreenPos(const Vector3& pos3) const;
    Vector3 GetScreenPosAndDepth(const Vector3& pos3) const;
    Vector3 GetScenePos(const float32 x, const float32 y, const float32 z) const;

    float32 GetDistanceToCamera() const;
    void UpdateDistanceToCamera();

    void Process(float32 timeElapsed) override;
    bool Input(UIEvent* event) override;

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    void LoadLocalProperties(PropertiesHolder* holder, ContextAccessor* accessor) override;
    void SaveLocalProperties(PropertiesHolder* holder) override;

    bool SnapEditorCameraToLandscape(bool snap);
    bool IsEditorCameraSnappedToLandscape() const;

    void MoveToSelection(const SelectableGroup& objects);
    void MoveToStep(int ofs);

    void EnableSystem() override;

    std::unique_ptr<Command> PrepareForSave(bool saveForGame) override;

protected:
    void Draw() override;

private:
    void PrecacheSystems();
    void OnKeyboardInput(UIEvent* event);
    void ScrollCamera(float32 dy);

    void CreateDebugCameras();
    void RecalcCameraAspect();

    void MoveAnimate(float32 timeElapsed);
    Entity* GetEntityFromCamera(Camera* camera) const;
    Entity* GetEntityWithEditorCamera() const;
    Entity* topCameraEntity = nullptr;

    Rect viewportRect;

    Camera* curSceneCamera = nullptr;

    bool animateToNewPos = false;
    float32 animateToNewPosTime = 0;
    Vector3 newPos;
    Vector3 newTar;

    Vector<Entity*> sceneCameras;
    float32 distanceToCamera = 0.f;

    uint32 activeSpeedIndex = 0;

    WASDControllerSystem* wasdSystem = nullptr;
    RotationControllerSystem* rotationSystem = nullptr;
    HoodSystem* hoodSystem = nullptr;
    SceneCollisionSystem* collisionSystem = nullptr;
};
} // namespace DAVA
