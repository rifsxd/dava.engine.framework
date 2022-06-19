#include "REPlatform/Scene/Systems/EditorLightSystem.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/Global/StringConstants.h"
#include "REPlatform/Global/Constants.h"

#include <Base/ScopedPtr.h>
#include <Debug/DVAssert.h>
#include <Entity/Component.h>
#include <Entity/ComponentUtils.h>
#include <Render/Highlevel/Camera.h>
#include <Render/Highlevel/Light.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/LightComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
EditorLightSystem::EditorLightSystem(Scene* scene)
    : SceneSystem(scene)
{
    ScopedPtr<Light> light(new Light());
    light->SetType(Light::TYPE_POINT);
    light->SetAmbientColor(Color(0.3f, 0.3f, 0.3f, 1.0f));

    cameraLight = new Entity();
    cameraLight->SetLocked(true);
    cameraLight->SetName(FastName(ResourceEditor::EDITOR_CAMERA_LIGHT));
    cameraLight->AddComponent(new LightComponent(light));

    SetRequiredComponents(ComponentUtils::MakeMask<LightComponent>());

    if (isEnabled)
    {
        AddCameraLightOnScene();
    }
}

EditorLightSystem::~EditorLightSystem()
{
    SafeRelease(cameraLight);
}

void EditorLightSystem::UpdateCameraLightState()
{
    if (isEnabled && lightEntities == 0)
    {
        AddCameraLightOnScene();
    }
    else if (!isEnabled || lightEntities != 0)
    {
        RemoveCameraLightFromScene();
    }
}

void EditorLightSystem::UpdateCameraLightPosition()
{
    if (cameraLight && cameraLight->GetParent())
    {
        Camera* camera = GetScene()->GetCurrentCamera();
        if (!camera)
            return;

        Vector3 newPosition = camera->GetPosition() - camera->GetLeft() * 20.f + camera->GetUp() * 20.f;
        TransformComponent* tc = cameraLight->GetComponent<TransformComponent>();
        if (newPosition != tc->GetLocalTransform().GetTranslation())
        {
            tc->SetLocalTranslation(newPosition);
        }
    }
}

void EditorLightSystem::SetCameraLightEnabled(bool enabled)
{
    if (enabled != isEnabled)
    {
        isEnabled = enabled;
        UpdateCameraLightState();
    }
}

void EditorLightSystem::AddCameraLightOnScene()
{
    SceneEditor2* sc = static_cast<SceneEditor2*>(GetScene());
    if (cameraLight->GetParent() == nullptr)
    {
        sc->AddEditorEntity(cameraLight);
    }
}

void EditorLightSystem::RemoveCameraLightFromScene()
{
    if (cameraLight && cameraLight->GetParent())
    {
        cameraLight->GetParent()->RemoveNode(cameraLight);
    }
}

void EditorLightSystem::SceneDidLoaded()
{
    if (isEnabled && lightEntities == 0)
    {
        AddCameraLightOnScene();
    }
}

void EditorLightSystem::AddEntity(Entity* entity)
{
    DVASSERT(GetLightComponent(entity) != nullptr);
    if (entity == cameraLight)
    {
        return;
    }

    ++lightEntities;
    RemoveCameraLightFromScene();
}

void EditorLightSystem::RemoveEntity(Entity* entity)
{
    if (entity == cameraLight)
    {
        return;
    }

    --lightEntities;

    if (isEnabled && lightEntities == 0)
    {
        AddCameraLightOnScene();
    }
}

void EditorLightSystem::PrepareForRemove()
{
}

void EditorLightSystem::Process(float32 timeElapsed)
{
    if (isEnabled)
    {
        UpdateCameraLightPosition();
    }
}
} // namespace DAVA
