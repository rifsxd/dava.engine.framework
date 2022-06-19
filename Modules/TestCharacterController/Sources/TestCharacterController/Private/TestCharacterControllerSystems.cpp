#include "TestCharacterController/TestCharacterControllerSystems.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <DeviceManager/DeviceManager.h>

#include <Input/InputSystem.h>
#include <Input/Keyboard.h>
#include <Input/Mouse.h>

#include <Render/Highlevel/Camera.h>

#include <Math/Transform.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/MotionComponent.h>
#include <Scene3D/Components/SingleComponents/MotionSingleComponent.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Utils/Utils.h>

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
#include <Physics/PhysicsUtils.h>
#include <Physics/CharacterControllerComponent.h>
#endif

namespace DAVA
{
TestCharacterControllerSystem::TestCharacterControllerSystem(Scene* scene)
    : SceneSystem(scene)
{
    characterForward = Vector3::UnitX;
    cameraDirection = characterForward;

    characterLeft = Vector3::UnitZ.CrossProduct(characterForward);
}

TestCharacterControllerSystem::~TestCharacterControllerSystem()
{
    SafeRelease(characterEntity);
}

void TestCharacterControllerSystem::SetCharacterEntity(DAVA::Entity* entity)
{
    SafeRelease(characterEntity);
    characterMeshEntity = nullptr;
    weaponEntity = nullptr;
    shootEffect = nullptr;

    controllerComponent = nullptr;
    characterMotionComponent = nullptr;
    characterSkeleton = nullptr;

    headJointIndex = DAVA::SkeletonComponent::INVALID_JOINT_INDEX;
    weaponPointJointIndex = DAVA::SkeletonComponent::INVALID_JOINT_INDEX;

    if (entity != nullptr)
    {
        characterEntity = SafeRetain(entity);
        characterMeshEntity = entity->FindByName("Character");
        DVASSERT(characterMeshEntity != nullptr);

        weaponEntity = entity->FindByName("Weapon");
        if (weaponEntity != nullptr)
            shootEffect = weaponEntity->FindByName("shot_auto");

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
        controllerComponent = PhysicsUtils::GetCharacterControllerComponent(entity);
#endif

        characterSkeleton = GetSkeletonComponent(characterMeshEntity);
        DVASSERT(characterSkeleton != nullptr);

        headJointIndex = characterSkeleton->GetJointIndex(FastName("node-Head"));

        weaponPointJointIndex = characterSkeleton->GetJointIndex(FastName("node-RH_WP"));
        if (weaponPointJointIndex == SkeletonComponent::INVALID_JOINT_INDEX)
            weaponPointJointIndex = characterSkeleton->GetJointIndex(FastName("node-Weapon_Primary"));

        DVASSERT(headJointIndex != SkeletonComponent::INVALID_JOINT_INDEX);
        DVASSERT(weaponPointJointIndex != SkeletonComponent::INVALID_JOINT_INDEX);

        characterMotionComponent = GetMotionComponent(characterMeshEntity);
    }
}

void TestCharacterControllerSystem::PrepareForRemove()
{
    SetCharacterEntity(nullptr);
}

void TestCharacterControllerSystem::Process(float32 timeElapsed)
{
    if (characterMotionComponent == nullptr)
        return;

    timeElapsed *= characterMotionComponent->GetPlaybackRate();

    Quaternion characterOrientation;
    characterOrientation.Construct(-Vector3::UnitY, characterForward);
    TransformComponent* tc = characterMeshEntity->GetComponent<TransformComponent>();
    tc->SetLocalRotation(characterOrientation);

    //////////////////////////////////////////////////////////////////////////
    //Calculate motion params

    Keyboard* keyboard = GetEngineContext()->deviceManager->GetKeyboard();
    Mouse* mouse = GetEngineContext()->deviceManager->GetMouse();

    Vector2 moveDirectionTarget = inputJoypadDirection;
    if (keyboard != nullptr)
    {
        if (keyboard->GetKeyState(eInputElements::KB_W).IsPressed() || keyboard->GetKeyState(eInputElements::KB_UP).IsPressed())
            moveDirectionTarget.y += 1.f;

        if (keyboard->GetKeyState(eInputElements::KB_S).IsPressed() || keyboard->GetKeyState(eInputElements::KB_DOWN).IsPressed())
            moveDirectionTarget.y -= 1.f;

        if (keyboard->GetKeyState(eInputElements::KB_A).IsPressed() || keyboard->GetKeyState(eInputElements::KB_LEFT).IsPressed())
            moveDirectionTarget.x -= 1.f;

        if (keyboard->GetKeyState(eInputElements::KB_D).IsPressed() || keyboard->GetKeyState(eInputElements::KB_RIGHT).IsPressed())
            moveDirectionTarget.x += 1.f;
    }

    float32 directionDt = timeElapsed * 5.f;

    if (directionParam.x > moveDirectionTarget.x)
        directionParam.x -= directionDt;
    if (directionParam.x < moveDirectionTarget.x)
        directionParam.x += directionDt;

    if (directionParam.y > moveDirectionTarget.y)
        directionParam.y -= directionDt;
    if (directionParam.y < moveDirectionTarget.y)
        directionParam.y += directionDt;

    if (Abs(directionParam.x) < directionDt)
        directionParam.x = 0.f;
    if (Abs(directionParam.y) < directionDt)
        directionParam.y = 0.f;

    directionParam.x = Clamp(directionParam.x, -1.f, 1.f);
    directionParam.y = Clamp(directionParam.y, -1.f, 1.f);

    isMoving = (directionParam.SquareLength() > EPSILON || !moveDirectionTarget.IsZero());
    isCrouching = (keyboard != nullptr) && keyboard->GetKeyState(eInputElements::KB_LCTRL).IsPressed();
    isRun = isMoving && !isCrouching && !waitReloadEnd && (keyboard != nullptr) && keyboard->GetKeyState(eInputElements::KB_LSHIFT).IsPressed();
    isZooming = !isRun && ((mouse != nullptr && mouse->GetRightButtonState().IsPressed()) || doubleTapped);

    runningParam += (isRun ? timeElapsed : -timeElapsed) * 3.f;
    runningParam = Clamp(runningParam, 0.f, 1.f);

    crouchingParam += (isCrouching ? timeElapsed : -timeElapsed) * 3.f;
    crouchingParam = Clamp(crouchingParam, 0.f, 1.f);

    zoomFactor += (isZooming ? timeElapsed : -timeElapsed) * 3.f;
    zoomFactor = Clamp(zoomFactor, 0.f, 1.f);

    aimAngleParam = RadToDeg(-cameraAngle);

    //////////////////////////////////////////////////////////////////////////

    const static FastName WEAPON_MOTION_LAYER_ID("weapon-layer");
    const static FastName WEAPON_RELOAD_MOTION_ID("reload");
    const static FastName WEAPON_SHOOT_MOTION_ID("shoot");
    const static FastName WEAPON_SHOOT_MARKER("shoot");

    MotionSingleComponent* msc = GetScene()->motionSingleComponent;
    if (waitReloadEnd)
    {
        MotionSingleComponent::AnimationInfo reloadAnimationInfo(characterMotionComponent, WEAPON_MOTION_LAYER_ID, WEAPON_RELOAD_MOTION_ID);
        waitReloadEnd = (msc->animationEnd.count(reloadAnimationInfo) == 0);
    }

    if (shootEffect != nullptr)
    {
        const static MotionSingleComponent::AnimationInfo shootMarkerInfo(characterMotionComponent, WEAPON_MOTION_LAYER_ID, WEAPON_SHOOT_MOTION_ID, WEAPON_SHOOT_MARKER);
        if (msc->animationMarkerReached.count(shootMarkerInfo))
            GetParticleEffectComponent(shootEffect)->Start();
    }

    //////////////////////////////////////////////////////////////////////////
    //Setup motion animation

    const static FastName MOTION_PARAM_RUNNING("running");
    const static FastName MOTION_PARAM_CROUCHING("crouching");
    const static FastName MOTION_PARAM_AIM_ANGLE("aim-angle");
    const static FastName MOTION_PARAM_DIRECTION_X("direction-x");
    const static FastName MOTION_PARAM_DIRECTION_Y("direction-y");

    const static FastName TRIGGER_MOVE("move");
    const static FastName TRIGGER_STOP("stop");
    const static FastName TRIGGER_WEAPON_SHOOT("shoot");
    const static FastName TRIGGER_WEAPON_RELOAD("reload");
    const static FastName TRIGGER_WEAPON_IDLE("idle");

    characterMotionComponent->SetParameter(MOTION_PARAM_RUNNING, runningParam);
    characterMotionComponent->SetParameter(MOTION_PARAM_CROUCHING, crouchingParam);
    characterMotionComponent->SetParameter(MOTION_PARAM_AIM_ANGLE, aimAngleParam);
    characterMotionComponent->SetParameter(MOTION_PARAM_DIRECTION_X, directionParam.x);
    characterMotionComponent->SetParameter(MOTION_PARAM_DIRECTION_Y, directionParam.y);

    if (!isRun)
    {
        if (keyboard != nullptr && keyboard->GetKeyState(eInputElements::KB_R).IsJustPressed())
        {
            characterMotionComponent->TriggerEvent(TRIGGER_WEAPON_RELOAD);
            waitReloadEnd = true;
        }
        else if (!waitReloadEnd)
        {
            if (mouse != nullptr && mouse->GetLeftButtonState().IsPressed())
            {
                characterMotionComponent->TriggerEvent(TRIGGER_WEAPON_SHOOT);
            }
            else
            {
                characterMotionComponent->TriggerEvent(TRIGGER_WEAPON_IDLE);
            }
        }
    }
    else
    {
        characterMotionComponent->TriggerEvent(TRIGGER_WEAPON_IDLE);
    }

    if (isMoving)
    {
        characterMotionComponent->TriggerEvent(TRIGGER_MOVE);
    }
    else
    {
        characterMotionComponent->TriggerEvent(TRIGGER_STOP);
    }
}

bool TestCharacterControllerSystem::Input(UIEvent* uiEvent)
{
    const static float32 MOUSE_SENSITIVITY_X = 2.f;
    const static float32 MOUSE_SENSITIVITY_Y = 1.4f;
    const static float32 TOUCH_SENSITIVITY_X = 4.f;
    const static float32 TOUCH_SENSITIVITY_Y = 2.8f;

    float32 relativeX = 0.f;
    float32 relativeY = 0.f;

    Size2f wndSize = uiEvent->window->GetSize();

    if (uiEvent->device == eInputDevices::MOUSE && (uiEvent->phase == UIEvent::Phase::MOVE || uiEvent->phase == UIEvent::Phase::DRAG) && uiEvent->isRelative)
    {
        relativeX = uiEvent->point.x / wndSize.dx * MOUSE_SENSITIVITY_X;
        relativeY = uiEvent->point.y / wndSize.dy * MOUSE_SENSITIVITY_Y;
    }

    if (uiEvent->device == eInputDevices::TOUCH_SURFACE)
    {
        if (uiEvent->phase == UIEvent::Phase::BEGAN)
        {
            inputBeginPosition = uiEvent->point;
            inputEndPosition = uiEvent->point;

            if (uiEvent->tapCount > 1)
                doubleTapped = !doubleTapped;
        }
        else if (uiEvent->phase == UIEvent::Phase::DRAG || uiEvent->phase == UIEvent::Phase::ENDED)
        {
            inputBeginPosition = inputEndPosition;
            inputEndPosition = uiEvent->point;
        }

        relativeX = (inputEndPosition.x - inputBeginPosition.x) / wndSize.dx * TOUCH_SENSITIVITY_X;
        relativeY = (inputEndPosition.y - inputBeginPosition.y) / wndSize.dy * TOUCH_SENSITIVITY_Y;
    }

    cameraAngle += relativeY;
    cameraAngle = Clamp(cameraAngle, DegToRad(-45.f), DegToRad(45.f));

    characterForward = Quaternion::MakeRotationFastZ(-relativeX).ApplyToVectorFast(characterForward);
    characterForward.Normalize();

    characterLeft = Vector3::UnitZ.CrossProduct(characterForward);
    cameraDirection = Quaternion::MakeRotation(characterLeft, cameraAngle).ApplyToVectorFast(characterForward);

    return false;
}

void TestCharacterControllerSystem::SetJoypadDirection(const DAVA::Vector2& direction)
{
    inputJoypadDirection.x = Clamp(direction.x, -1.f, 1.f);
    inputJoypadDirection.y = Clamp(-direction.y, -1.f, 1.f);
}

//////////////////////////////////////////////////////////////////////////

TestCharacterMoveSystem::TestCharacterMoveSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
    controllerSystem = scene->GetSystem<TestCharacterControllerSystem>();
}

void TestCharacterMoveSystem::PrepareForRemove()
{
    controllerSystem = nullptr;
}

void TestCharacterMoveSystem::Process(DAVA::float32 timeElapsed)
{
    if (controllerSystem->characterMotionComponent == nullptr)
        return;

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
    const Vector3& characterForward = controllerSystem->characterForward;
    const Vector3& characterLeft = controllerSystem->characterLeft;
    const Vector3& rootOffsetDelta = controllerSystem->characterMotionComponent->GetRootOffsetDelta();

    Vector3 moveDisplacement = -characterForward * rootOffsetDelta.y + characterLeft * rootOffsetDelta.x;
    controllerSystem->controllerComponent->Move(moveDisplacement);
#endif
}

//////////////////////////////////////////////////////////////////////////

TestCharacterWeaponSystem::TestCharacterWeaponSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
    controllerSystem = scene->GetSystem<TestCharacterControllerSystem>();
}

void TestCharacterWeaponSystem::PrepareForRemove()
{
    controllerSystem = nullptr;
}

void TestCharacterWeaponSystem::Process(DAVA::float32 timeElapsed)
{
    if (controllerSystem->weaponEntity == nullptr)
        return;

    SkeletonComponent* skeleton = controllerSystem->characterSkeleton;
    Vector3 weaponPointPosition = skeleton->GetJointObjectSpaceTransform(controllerSystem->weaponPointJointIndex).GetPosition();
    Quaternion weaponPointOrientation = skeleton->GetJointObjectSpaceTransform(controllerSystem->weaponPointJointIndex).GetOrientation();

    Quaternion weaponTransform = Quaternion::MakeRotation(Vector3::UnitX, DegToRad(90.f));

    TransformComponent* tc = controllerSystem->weaponEntity->GetComponent<TransformComponent>();
    tc->SetLocalRotation(weaponTransform * weaponPointOrientation);
    tc->SetLocalTranslation(weaponPointPosition);
}

//////////////////////////////////////////////////////////////////////////

TestCharacterCameraSystem::TestCharacterCameraSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
    controllerSystem = scene->GetSystem<TestCharacterControllerSystem>();
}

void TestCharacterCameraSystem::PrepareForRemove()
{
    controllerSystem = nullptr;
}

void TestCharacterCameraSystem::Process(DAVA::float32 timeElapsed)
{
    if (controllerSystem->characterSkeleton == nullptr || controllerSystem->characterMeshEntity == nullptr)
        return;

    const Vector3& cameraDirection = controllerSystem->cameraDirection;
    const Vector3& characterLeft = controllerSystem->characterLeft;
    const float32 zoomFactor = controllerSystem->zoomFactor;
    const float32 crouchingParam = controllerSystem->crouchingParam;
    const uint32 headJointIndex = controllerSystem->headJointIndex;

    Camera* camera = GetScene()->GetCurrentCamera();
    Entity* characterMeshEntity = controllerSystem->characterMeshEntity;
    SkeletonComponent* characterSkeleton = controllerSystem->characterSkeleton;

    //////////////////////////////////////////////////////////////////////////

    Vector3 headJointPosition = characterSkeleton->GetJointObjectSpaceTransform(headJointIndex).GetPosition();
    headJointPosition.x = 0.f;
    headJointPosition.z = Lerp(Lerp(1.75f, 1.65f, crouchingParam), headJointPosition.z + 0.1f, zoomFactor);

    TransformComponent* tc = characterMeshEntity->GetComponent<TransformComponent>();
    Vector3 headPosition = headJointPosition * tc->GetWorldTransform();

    Vector3 normalCameraOffset = -2.4f * cameraDirection - 0.1f * characterLeft;
    Vector3 zoomCameraOffset = -1.4f * cameraDirection - 0.35f * characterLeft;

    Vector3 cameraOffset = Lerp(normalCameraOffset, zoomCameraOffset, zoomFactor);
    float32 fov = Lerp(70.f, 55.f, zoomFactor);

    camera->SetPosition(headPosition + cameraOffset);
    camera->SetDirection(cameraDirection);
    camera->SetUp(cameraDirection.CrossProduct(characterLeft));
    camera->SetFOV(fov);
}
};
