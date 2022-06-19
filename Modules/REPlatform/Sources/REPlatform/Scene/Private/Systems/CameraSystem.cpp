#include "REPlatform/Scene/Systems/CameraSystem.h"

#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Systems/HoodSystem.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"

#include "REPlatform/Commands/AddComponentCommand.h"
#include "REPlatform/Commands/EntityRemoveCommand.h"
#include "REPlatform/Commands/RemoveComponentCommand.h"
#include "REPlatform/DataNodes/SceneData.h"
#include "REPlatform/DataNodes/SelectableGroup.h"
#include "REPlatform/DataNodes/Settings/GlobalSceneSettings.h"
#include "REPlatform/DataNodes/Settings/RESettings.h"
#include "REPlatform/Global/StringConstants.h"

#include <TArc/Core/Deprecated.h>

#include <DeviceManager/DeviceManager.h>
#include <Engine/Engine.h>
#include <Input/InputSystem.h>
#include <Input/Keyboard.h>
#include <Render/RenderHelper.h>
#include <Scene3D/Components/CameraComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/Controller/RotationControllerComponent.h>
#include <Scene3D/Components/Controller/SnapToLandscapeControllerComponent.h>
#include <Scene3D/Components/Controller/WASDControllerComponent.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/Controller/RotationControllerSystem.h>
#include <Scene3D/Systems/Controller/WASDControllerSystem.h>
#include <Utils/Utils.h>

#include <memory>

namespace DAVA
{
namespace CameraSystemDetails
{
const auto wheelAdjust = 0.002;
} // namespace CameraSystemDetails

SceneCameraSystem::SceneCameraSystem(Scene* scene)
    : SceneSystem(scene)
{
}

SceneCameraSystem::~SceneCameraSystem()
{
    SafeRelease(curSceneCamera);
}

void SceneCameraSystem::SaveLocalProperties(PropertiesHolder* holder)
{
    if (curSceneCamera == nullptr || holder == nullptr)
    {
        return;
    }

    PropertiesItem cameraProps = holder->CreateSubHolder("SceneCameraSystem");
    // Debug camera whole object archive
    Camera* debugCam = GetCamera(topCameraEntity);
    RefPtr<KeyedArchive> camArch;
    camArch.ConstructInplace();
    debugCam->SaveObject(camArch.Get());
    cameraProps.Set("archive", camArch);

    // Current active camera name
    FastName curCamName = GetEntityFromCamera(curSceneCamera)->GetName();
    cameraProps.Set("activeCameraName", curCamName);
}

void SceneCameraSystem::LoadLocalProperties(PropertiesHolder* holder, ContextAccessor* accessor)
{
    PropertiesItem cameraProps = holder->CreateSubHolder("SceneCameraSystem");
    Camera* cur = GetCamera(topCameraEntity);

    GlobalSceneSettings* settings = accessor->GetGlobalContext()->GetData<GlobalSceneSettings>();
    RefPtr<KeyedArchive> camArch;
    camArch.ConstructInplace();
    cur->SaveObject(camArch.Get());
    camArch = cameraProps.Get<RefPtr<KeyedArchive>>("archive", camArch);
    if (settings->cameraUseDefaultSettings == false)
    {
        // load all parameters
        cur->LoadObject(camArch.Get());
    }
    else // restore only position
    {
        cur->SetPosition(camArch->GetByteArrayAsType("cam.position", cur->GetPosition()));
        cur->SetTarget(camArch->GetByteArrayAsType("cam.target", cur->GetTarget()));
        cur->SetUp(camArch->GetByteArrayAsType("cam.up", cur->GetUp()));
        cur->SetLeft(camArch->GetByteArrayAsType("cam.left", cur->GetLeft()));
    }

    // set active scene camera
    FastName camName = cameraProps.Get<FastName>("activeCameraName", FastName(ResourceEditor::EDITOR_DEBUG_CAMERA));
    auto camEntityIt = std::find_if(std::begin(sceneCameras), std::end(sceneCameras),
                                    [&camName](Entity* cam)
                                    {
                                        return cam->GetName() == camName;
                                    });
    if (camEntityIt != std::end(sceneCameras))
    {
        cur = GetCamera(*camEntityIt);
        Scene* scene = GetScene();
        scene->SetCurrentCamera(cur);
    }
}

Camera* SceneCameraSystem::GetCurCamera() const
{
    return curSceneCamera;
}

Vector3 SceneCameraSystem::GetPointDirection(const Vector2& point) const
{
    Vector3 dir;

    if (nullptr != curSceneCamera)
    {
        Vector3 pos = curSceneCamera->GetPosition();
        dir = curSceneCamera->UnProject(point.x, point.y, 0, viewportRect);
        dir -= pos;
    }

    return dir;
}

Vector3 SceneCameraSystem::GetCameraPosition() const
{
    Vector3 pos;

    if (nullptr != curSceneCamera)
    {
        pos = curSceneCamera->GetPosition();
    }

    return pos;
}

Vector3 SceneCameraSystem::GetCameraDirection() const
{
    Vector3 dir;

    if (nullptr != curSceneCamera)
    {
        dir = curSceneCamera->GetDirection();
    }

    return dir;
}

float32 SceneCameraSystem::GetMoveSpeed()
{
    float32 speed = 1.0;

    GlobalSceneSettings* settings = Deprecated::GetDataNode<GlobalSceneSettings>();

    switch (activeSpeedIndex)
    {
    case 0:
        speed = settings->cameraSpeed0;
        break;
    case 1:
        speed = settings->cameraSpeed1;
        break;
    case 2:
        speed = settings->cameraSpeed2;
        break;
    case 3:
        speed = settings->cameraSpeed3;
        break;
    }

    return speed;
}

uint32 SceneCameraSystem::GetActiveSpeedIndex()
{
    return activeSpeedIndex;
}

void SceneCameraSystem::SetMoveSpeedArrayIndex(uint32 index)
{
    DVASSERT(index < 4u);
    activeSpeedIndex = index;
}

void SceneCameraSystem::SetViewportRect(const Rect& rect)
{
    viewportRect = rect;
    RecalcCameraAspect();
}

const Rect& SceneCameraSystem::GetViewportRect() const
{
    return viewportRect;
}

Vector2 SceneCameraSystem::GetScreenPos(const Vector3& pos3) const
{
    Vector3 ret3d = GetScreenPosAndDepth(pos3);
    return Vector2(ret3d.x, ret3d.y);
}

Vector3 SceneCameraSystem::GetScreenPosAndDepth(const Vector3& pos3) const
{
    Vector3 ret;

    if (nullptr != curSceneCamera)
    {
        ret = curSceneCamera->GetOnScreenPositionAndDepth(pos3, viewportRect);
    }

    return ret;
}

Vector3 SceneCameraSystem::GetScenePos(const float32 x, const float32 y, const float32 z) const
{
    Vector3 ret;

    if (nullptr != curSceneCamera)
    {
        ret = curSceneCamera->UnProject(x, y, z, viewportRect);
    }

    return ret;
}

void SceneCameraSystem::LookAt(const AABBox3& box)
{
    if (nullptr != curSceneCamera && !box.IsEmpty())
    {
        Vector3 pos = curSceneCamera->GetPosition();
        Vector3 targ = curSceneCamera->GetTarget();
        Vector3 dir = targ - pos;
        dir.Normalize();

        float32 boxSize = ((box.max - box.min).Length());
        const Vector3 c = box.GetCenter();

        pos = c - (dir * (boxSize + curSceneCamera->GetZNear() * 1.5f));
        targ = c;

        MoveTo(pos, targ);
    }
}

void SceneCameraSystem::MoveTo(const Vector3& pos)
{
    if (nullptr != curSceneCamera)
    {
        MoveTo(pos, curSceneCamera->GetTarget());
    }
}

void SceneCameraSystem::MoveTo(const Vector3& pos, const Vector3& target)
{
    if (nullptr != curSceneCamera && !curSceneCamera->GetIsOrtho())
    {
        animateToNewPos = true;
        animateToNewPosTime = 0;

        newPos = pos;
        newTar = target;
    }
}

void SceneCameraSystem::Process(float timeElapsed)
{
    PrecacheSystems();

    if (wasdSystem)
    {
        wasdSystem->SetMoveSpeed((animateToNewPos) ? 0 : GetMoveSpeed());
    }
    if (rotationSystem)
    {
        rotationSystem->SetRotationSpeeed((animateToNewPos) ? 0 : 0.15f);
        if (nullptr != hoodSystem)
        {
            rotationSystem->SetRotationPoint(hoodSystem->GetPosition());
        }
    }

    Camera* camera = GetScene()->GetDrawCamera();

    // is current camera in scene changed?
    if (curSceneCamera != camera)
    {
        // update collision object for last camera
        if (nullptr != curSceneCamera)
        {
            Entity* cameraOwner = GetEntityFromCamera(curSceneCamera);
            if (cameraOwner != nullptr)
            {
                collisionSystem->UpdateCollisionObject(Selectable(cameraOwner));
            }
        }

        // remember current scene camera
        SafeRelease(curSceneCamera);
        curSceneCamera = SafeRetain(camera);

        // Recalc camera aspect
        RecalcCameraAspect();
    }

    // camera move animation
    MoveAnimate(timeElapsed);
}

bool SceneCameraSystem::Input(UIEvent* event)
{
    switch (event->phase)
    {
    case UIEvent::Phase::KEY_DOWN:
    case UIEvent::Phase::KEY_DOWN_REPEAT:
        OnKeyboardInput(event);
        break;
    case UIEvent::Phase::WHEEL:
        ScrollCamera(event->wheelDelta.y);
        break;
    case UIEvent::Phase::GESTURE:
    {
        const UIEvent::Gesture& gesture = event->gesture;
        if (gesture.dy != 0.0f)
        {
            ScrollCamera(gesture.dy);
        }
    }
    break;
    default:
        break;
    }
    return false;
}

void SceneCameraSystem::ScrollCamera(float32 dy)
{
    GeneralSettings* settings = Deprecated::GetDataNode<GeneralSettings>();
    if (settings->wheelMoveCamera == false)
        return;

    int32 reverse = settings->invertWheel ? -1 : 1;
    float32 moveIntence = settings->wheelMoveIntensity;
    int offset = dy * moveIntence;
#ifdef Q_OS_MAC
    offset *= reverse * -1;
#else
    offset *= reverse;
#endif

    MoveToStep(offset);
}

void SceneCameraSystem::PrecacheSystems()
{
    if (wasdSystem == nullptr || rotationSystem == nullptr || hoodSystem == nullptr || collisionSystem == nullptr)
    {
        Scene* scene = GetScene();
        wasdSystem = scene->GetSystem<WASDControllerSystem>();
        rotationSystem = scene->GetSystem<RotationControllerSystem>();
        hoodSystem = scene->GetSystem<HoodSystem>();
        collisionSystem = scene->GetSystem<SceneCollisionSystem>();
    }
}

void SceneCameraSystem::OnKeyboardInput(UIEvent* event)
{
    bool isModificatorPressed = false;

    Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
    if (kb != nullptr)
    {
        isModificatorPressed =
        kb->GetKeyState(eInputElements::KB_LCTRL).IsPressed() ||
        kb->GetKeyState(eInputElements::KB_LALT).IsPressed() ||
        kb->GetKeyState(eInputElements::KB_LSHIFT).IsPressed();
    }

    if (isModificatorPressed)
        return;

    switch (event->key)
    {
    case eInputElements::KB_NUMPAD_PLUS:
    case eInputElements::KB_EQUALS:
    {
        Entity* entity = GetEntityWithEditorCamera();
        SnapToLandscapeControllerComponent* snapComponent = GetSnapToLandscapeControllerComponent(entity);
        if (snapComponent != nullptr)
        {
            GlobalSceneSettings* settings = Deprecated::GetDataNode<GlobalSceneSettings>();
            float32 height = snapComponent->GetHeightOnLandscape() + settings->heightOnLandscapeStep;
            snapComponent->SetHeightOnLandscape(height);
            settings->heightOnLandscape = height;
        }
    }
    break;
    case eInputElements::KB_NUMPAD_MINUS:
    case eInputElements::KB_MINUS:
    {
        auto entity = GetEntityWithEditorCamera();
        auto snapComponent = GetSnapToLandscapeControllerComponent(entity);
        if (snapComponent != nullptr)
        {
            GlobalSceneSettings* settings = Deprecated::GetDataNode<GlobalSceneSettings>();
            float32 height = snapComponent->GetHeightOnLandscape() - settings->heightOnLandscapeStep;
            snapComponent->SetHeightOnLandscape(height);
            settings->heightOnLandscape = height;
        }
    }
    break;

    case eInputElements::KB_T:
        MoveTo(Vector3(0, 0, 200), Vector3(1, 0, 0));
        break;

    case eInputElements::KB_1:
        SetMoveSpeedArrayIndex(0);
        break;
    case eInputElements::KB_2:
        SetMoveSpeedArrayIndex(1);
        break;
    case eInputElements::KB_3:
        SetMoveSpeedArrayIndex(2);
        break;
    case eInputElements::KB_4:
        SetMoveSpeedArrayIndex(3);
        break;

    default:
        break;
    }
}

void SceneCameraSystem::Draw()
{
    PrecacheSystems();
    if (collisionSystem != nullptr)
    {
        for (Entity* entity : sceneCameras)
        {
            DVASSERT(entity != nullptr);
            Camera* camera = GetCamera(entity);
            if (nullptr != camera && camera != curSceneCamera)
            {
                AABBox3 worldBox;
                AABBox3 collBox = collisionSystem->GetBoundingBox(entity);
                DVASSERT(!collBox.IsEmpty());

                Matrix4 transform;
                transform.Identity();
                transform.SetTranslationVector(camera->GetPosition());
                collBox.GetTransformedBox(transform, worldBox);
                GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(worldBox, Color(0, 1.0f, 0, 1.0f), RenderHelper::DRAW_SOLID_DEPTH);
            }
        }
    }
}

void SceneCameraSystem::AddEntity(Entity* entity)
{
    DVASSERT(GetCamera(entity) != nullptr);
    sceneCameras.push_back(entity);
}

void SceneCameraSystem::RemoveEntity(Entity* entity)
{
    FindAndRemoveExchangingWithLast(sceneCameras, entity);
}

void SceneCameraSystem::PrepareForRemove()
{
    sceneCameras.clear();
    wasdSystem = nullptr;
    rotationSystem = nullptr;
    hoodSystem = nullptr;
    collisionSystem = nullptr;
}

void SceneCameraSystem::CreateDebugCameras()
{
    Scene* scene = GetScene();

    // add debug cameras
    // there already can be other cameras in scene
    if (nullptr != scene)
    {
        GlobalSceneSettings* settings = Deprecated::GetDataNode<GlobalSceneSettings>();

        ScopedPtr<Camera> topCamera(new Camera());
        topCamera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        topCamera->SetPosition(Vector3(-50.0f, 0.0f, 50.0f));
        topCamera->SetTarget(Vector3(0.0f, 0.1f, 0.0f));
        float32 cameraFov = settings->cameraFOV;
        float32 cameraNear = settings->cameraNear;
        float32 cameraFar = settings->cameraFar;
        topCamera->SetupPerspective(cameraFov, 320.0f / 480.0f, cameraNear, cameraFar);
        topCamera->SetAspect(1.0f);

        topCameraEntity = new Entity();
        topCameraEntity->SetName(FastName(ResourceEditor::EDITOR_DEBUG_CAMERA));
        topCameraEntity->SetNotRemovable(true);
        topCameraEntity->AddComponent(new CameraComponent(topCamera));
        topCameraEntity->AddComponent(new WASDControllerComponent());
        topCameraEntity->AddComponent(new RotationControllerComponent());
        if (scene->GetChildrenCount() > 0)
        {
            scene->InsertBeforeNode(topCameraEntity, scene->GetChild(0));
        }
        else
        {
            scene->AddNode(topCameraEntity);
        }

        // set current default camera
        if (nullptr == scene->GetCurrentCamera())
        {
            scene->SetCurrentCamera(topCamera);
        }

        scene->AddCamera(topCamera);
    }
}

void SceneCameraSystem::RecalcCameraAspect()
{
    if (nullptr != curSceneCamera)
    {
        float32 aspect = 1.0;

        if (0 != viewportRect.dx && 0 != viewportRect.dy)
        {
            aspect = viewportRect.dx / viewportRect.dy;
        }

        curSceneCamera->SetAspect(aspect);
    }
}

void SceneCameraSystem::MoveAnimate(float32 timeElapsed)
{
    static const float32 animationTime = 3.0f;
    static const float32 animationStopDistance = 1.0f;

    if (nullptr != curSceneCamera && animateToNewPos)
    {
        Vector3 pos = curSceneCamera->GetPosition();
        Vector3 tar = curSceneCamera->GetTarget();
        const float32 animationDistance = (pos - newPos).Length();

        if ((pos != newPos || tar != newTar) && (animateToNewPosTime < animationTime) && (animationDistance > animationStopDistance))
        {
            animateToNewPosTime += timeElapsed;

            float32 fnX = animateToNewPosTime / animationTime;
            float32 fnY = sin(1.57 * fnX);

            Vector3 dPos = newPos - pos;
            Vector3 dTar = newTar - tar;

            if (dPos.Length() > 0.01f)
                dPos = dPos * fnY;
            if (dTar.Length() > 0.01f)
                dTar = dTar * fnY;

            curSceneCamera->SetPosition(pos + dPos);
            curSceneCamera->SetTarget(tar + dTar);
        }
        else
        {
            animateToNewPos = false;
            animateToNewPosTime = 0;

            curSceneCamera->SetTarget(newTar);
            curSceneCamera->SetPosition(newPos);

            rotationSystem->RecalcCameraViewAngles(curSceneCamera);
        }

        UpdateDistanceToCamera();
    }
}

void SceneCameraSystem::UpdateDistanceToCamera()
{
    distanceToCamera = 0.f;

    const Camera* cam = GetScene()->GetCurrentCamera();
    if (cam)
    {
        const SelectableGroup& selection = GetScene()->GetSystem<SelectionSystem>()->GetSelection();
        if (!selection.IsEmpty())
        {
            AABBox3 bbox = selection.GetTransformedBoundingBox();
            if (!bbox.IsEmpty())
            {
                distanceToCamera = ((cam->GetPosition() - bbox.GetCenter()).Length()) * cam->GetZoomFactor();
            }
        }
    }
}

float32 SceneCameraSystem::GetDistanceToCamera() const
{
    return distanceToCamera;
}

Entity* SceneCameraSystem::GetEntityFromCamera(Camera* c) const
{
    for (auto& entity : sceneCameras)
    {
        if (GetCamera(entity) == c)
        {
            return entity;
        }
    }

    return nullptr;
}

void SceneCameraSystem::GetRayTo2dPoint(const Vector2& point, float32 maxRayLen, Vector3& outPointFrom, Vector3& outPointTo) const
{
    if (nullptr != curSceneCamera)
    {
        Vector3 camPos = GetCameraPosition();
        Vector3 camDir = GetPointDirection(point);

        if (curSceneCamera->GetIsOrtho())
        {
            outPointFrom = Vector3(camDir.x, camDir.y, camPos.z);
            outPointTo = Vector3(camDir.x, camDir.y, camPos.z + maxRayLen);
        }
        else
        {
            outPointFrom = camPos;
            outPointTo = outPointFrom + camDir * maxRayLen;
        }
    }
}

Entity* SceneCameraSystem::GetEntityWithEditorCamera() const
{
    int32 cameraCount = GetScene()->GetCameraCount();
    for (int32 i = 0; i < cameraCount; ++i)
    {
        Camera* c = GetScene()->GetCamera(i);
        Entity* e = GetEntityFromCamera(c);
        if (e && e->GetName() == FastName(ResourceEditor::EDITOR_DEBUG_CAMERA))
        {
            return e;
        }
    }

    return nullptr;
}

bool SceneCameraSystem::SnapEditorCameraToLandscape(bool snap)
{
    Entity* entity = GetEntityWithEditorCamera();
    if (!entity)
        return false;

    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());

    SnapToLandscapeControllerComponent* snapComponent = GetSnapToLandscapeControllerComponent(entity);
    if (snap)
    {
        if (!snapComponent)
        {
            float32 height = Deprecated::GetDataNode<GlobalSceneSettings>()->heightOnLandscape;

            snapComponent = new SnapToLandscapeControllerComponent();
            snapComponent->SetHeightOnLandscape(height);

            scene->Exec(std::make_unique<AddComponentCommand>(entity, snapComponent));
        }
    }
    else if (snapComponent)
    {
        scene->Exec(std::make_unique<RemoveComponentCommand>(entity, snapComponent));
    }

    return true;
}

bool SceneCameraSystem::IsEditorCameraSnappedToLandscape() const
{
    Entity* entity = GetEntityWithEditorCamera();
    return (GetSnapToLandscapeControllerComponent(entity) != nullptr);
}

void SceneCameraSystem::MoveToSelection(const SelectableGroup& objects)
{
    if (!objects.IsEmpty())
    {
        AABBox3 bbox = objects.GetTransformedBoundingBox();
        if (!bbox.IsEmpty())
        {
            LookAt(bbox);
        }
    }
}

void SceneCameraSystem::MoveToStep(int ofs)
{
    const auto pos = GetCameraPosition();
    const auto direction = GetCameraDirection();
    const auto delta = direction * GetMoveSpeed() * ofs * CameraSystemDetails::wheelAdjust;
    const auto dest = pos + delta;
    const auto target = dest + direction;

    MoveTo(dest, target);
}

void SceneCameraSystem::EnableSystem()
{
    EditorSceneSystem::EnableSystem();
    CreateDebugCameras();
}

std::unique_ptr<Command> SceneCameraSystem::PrepareForSave(bool saveForGame)
{
    return std::make_unique<EntityRemoveCommand>(topCameraEntity);
}
} // namespace DAVA
