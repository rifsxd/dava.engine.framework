#include "Physics/WASDPhysicsControllerSystem.h"
#include "Physics/WASDPhysicsControllerComponent.h"
#include "Physics/CharacterControllerComponent.h"
#include "Physics/PhysicsUtils.h"

#include <DeviceManager/DeviceManager.h>
#include <Engine/Engine.h>
#include <Input/Keyboard.h>
#include <Math/Transform.h>
#include <Render/Highlevel/Camera.h>
#include <Scene3D/Components/CameraComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
WASDPhysicsControllerSystem::WASDPhysicsControllerSystem(Scene* scene)
    : SceneSystem(scene)
{
}

void WASDPhysicsControllerSystem::RegisterEntity(Entity* e)
{
    Component* controllerComponent = e->GetComponent(Type::Instance<WASDPhysicsControllerComponent>());
    if (controllerComponent != nullptr)
    {
        RegisterComponent(e, controllerComponent);
    }
}

void WASDPhysicsControllerSystem::UnregisterEntity(Entity* e)
{
    Component* controllerComponent = e->GetComponent(Type::Instance<WASDPhysicsControllerComponent>());
    if (controllerComponent != nullptr)
    {
        UnregisterComponent(e, controllerComponent);
    }
}

void WASDPhysicsControllerSystem::RegisterComponent(Entity* e, Component* c)
{
    if (c->GetType()->Is<WASDPhysicsControllerComponent>())
    {
        wasdComponents.insert(static_cast<WASDPhysicsControllerComponent*>(c));
    }
}

void WASDPhysicsControllerSystem::UnregisterComponent(Entity* e, Component* c)
{
    if (c->GetType()->Is<WASDPhysicsControllerComponent>())
    {
        wasdComponents.erase(static_cast<WASDPhysicsControllerComponent*>(c));
    }
}

void WASDPhysicsControllerSystem::PrepareForRemove()
{
    wasdComponents.clear();
}

void WASDPhysicsControllerSystem::Process(float32 timeElapsed)
{
    if (wasdComponents.size() == 0)
    {
        return;
    }

    Keyboard* keyboard = GetEngineContext()->deviceManager->GetKeyboard();
    if (keyboard == nullptr)
    {
        return;
    }

    for (WASDPhysicsControllerComponent* wasdComponent : wasdComponents)
    {
        DVASSERT(wasdComponent != nullptr);

        Entity* entity = wasdComponent->GetEntity();
        DVASSERT(entity != nullptr);

        CharacterControllerComponent* characterControllerComponent = PhysicsUtils::GetCharacterControllerComponent(entity);
        if (characterControllerComponent == nullptr)
        {
            continue;
        }

        Vector3 forward = Vector3::UnitX;
        Vector3 right = -Vector3::UnitY;

        // This system also sync entity's camera (if there is any) and uses it's direction for convenience
        CameraComponent* cameraComponent = entity->GetComponent<CameraComponent>();
        if (cameraComponent != nullptr)
        {
            forward = cameraComponent->GetCamera()->GetDirection();
            right = -cameraComponent->GetCamera()->GetLeft();

            const Vector3 direction = cameraComponent->GetCamera()->GetDirection();
            TransformComponent* transform = entity->GetComponent<TransformComponent>();
            cameraComponent->GetCamera()->SetPosition(transform->GetLocalTransform().GetTranslation());
            cameraComponent->GetCamera()->SetDirection(direction);
        }

        forward *= moveSpeedCoeff;
        right *= moveSpeedCoeff;

        if (keyboard->GetKeyState(eInputElements::KB_LSHIFT).IsPressed())
        {
            forward *= 2.0f;
            right *= 2.0f;
        }

        if (keyboard->GetKeyState(eInputElements::KB_W).IsPressed() || keyboard->GetKeyState(eInputElements::KB_UP).IsPressed())
        {
            characterControllerComponent->Move(forward);
        }

        if (keyboard->GetKeyState(eInputElements::KB_S).IsPressed() || keyboard->GetKeyState(eInputElements::KB_DOWN).IsPressed())
        {
            characterControllerComponent->Move(-forward);
        }

        if (keyboard->GetKeyState(eInputElements::KB_D).IsPressed() || keyboard->GetKeyState(eInputElements::KB_RIGHT).IsPressed())
        {
            characterControllerComponent->Move(right);
        }

        if (keyboard->GetKeyState(eInputElements::KB_A).IsPressed() || keyboard->GetKeyState(eInputElements::KB_LEFT).IsPressed())
        {
            characterControllerComponent->Move(-right);
        }
    }
}
}
