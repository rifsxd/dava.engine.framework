#pragma once

#include "Base/BaseMath.h"
#include "Entity/SceneSystem.h"

#include "Scene3D/Components/SkeletonComponent.h"

namespace DAVA
{
class MotionComponent;
class CharacterControllerComponent;

class TestCharacterMoveSystem;
class TestCharacterWeaponSystem;
class TestCharacterCameraSystem;
class TestCharacterControllerSystem : public DAVA::SceneSystem
{
public:
    TestCharacterControllerSystem(DAVA::Scene* scene);
    virtual ~TestCharacterControllerSystem();

    void PrepareForRemove() override;

    void Process(DAVA::float32 timeElapsed) override;
    bool Input(DAVA::UIEvent* uiEvent) override;

    void SetCharacterEntity(DAVA::Entity* entity);
    void SetJoypadDirection(const DAVA::Vector2& direction);

private:
    DAVA::Entity* characterEntity = nullptr;
    DAVA::CharacterControllerComponent* controllerComponent = nullptr;

    DAVA::Entity* characterMeshEntity = nullptr;
    DAVA::Entity* weaponEntity = nullptr;
    DAVA::Entity* shootEffect = nullptr;
    DAVA::MotionComponent* characterMotionComponent = nullptr;
    DAVA::SkeletonComponent* characterSkeleton = nullptr;

    DAVA::Vector2 inputJoypadDirection;
    DAVA::Vector2 inputBeginPosition;
    DAVA::Vector2 inputEndPosition;

    DAVA::Vector3 characterForward;
    DAVA::Vector3 characterLeft;
    DAVA::Vector3 cameraDirection;
    DAVA::float32 cameraAngle = 0.f;

    DAVA::Vector2 directionParam;
    DAVA::float32 runningParam = 0.f;
    DAVA::float32 crouchingParam = 0.f;
    DAVA::float32 aimAngleParam = 0.f;
    DAVA::float32 zoomFactor = 0.f;

    DAVA::uint32 headJointIndex = DAVA::SkeletonComponent::INVALID_JOINT_INDEX;
    DAVA::uint32 weaponPointJointIndex = DAVA::SkeletonComponent::INVALID_JOINT_INDEX;

    bool isMoving = false;
    bool isRun = false;
    bool isCrouching = false;
    bool isZooming = false;

    bool doubleTapped = false;
    bool waitReloadEnd = false;

    friend class TestCharacterMoveSystem;
    friend class TestCharacterWeaponSystem;
    friend class TestCharacterCameraSystem;
};

class TestCharacterMoveSystem : public DAVA::SceneSystem
{
public:
    TestCharacterMoveSystem(DAVA::Scene* scene);

    void PrepareForRemove() override;
    void Process(DAVA::float32 timeElapsed) override;

protected:
    TestCharacterControllerSystem* controllerSystem = nullptr;
};

class TestCharacterWeaponSystem : public DAVA::SceneSystem
{
public:
    TestCharacterWeaponSystem(DAVA::Scene* scene);

    void PrepareForRemove() override;
    void Process(DAVA::float32 timeElapsed) override;

protected:
    TestCharacterControllerSystem* controllerSystem = nullptr;
};

class TestCharacterCameraSystem : public DAVA::SceneSystem
{
public:
    TestCharacterCameraSystem(DAVA::Scene* scene);

    void PrepareForRemove() override;
    void Process(DAVA::float32 timeElapsed) override;

protected:
    TestCharacterControllerSystem* controllerSystem = nullptr;
};
};