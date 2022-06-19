#include "TestCharacterController/TestCharacterControllerModule.h"
#include "TestCharacterController/TestCharacterControllerSystems.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Math/Transform.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/MotionComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/MotionSystem.h>
#include <Scene3D/Systems/RenderUpdateSystem.h>
#include <Scene3D/Systems/TransformSystem.h>

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
#include <Physics/CapsuleCharacterControllerComponent.h>
#include <Physics/PhysicsSystem.h>
#endif

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(TestCharacterControllerModule)
{
    ReflectionRegistrator<TestCharacterControllerModule>::Begin()
    .End();
}

TestCharacterControllerModule::TestCharacterControllerModule(Engine* engine)
    : IModule(engine)
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(TestCharacterControllerModule);
}

void TestCharacterControllerModule::CheckCharacterResources()
{
    if (testCharacterEntity == nullptr)
    {
        ScopedPtr<Scene> characterScene(new Scene());
        characterScene->LoadScene("~res:/TestCharacterControllerModule/character/character_mesh.sc2");

        Entity* characterSourceEntity = characterScene->FindByName("character");
        if (characterSourceEntity != nullptr)
        {
            testCharacterEntity = new Entity();

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
            testCharacterEntity->AddComponent(new CapsuleCharacterControllerComponent());
#endif

            ScopedPtr<Entity> characterMeshEntity(characterSourceEntity->Clone());
            characterMeshEntity->SetName("Character");
            characterMeshEntity->AddComponent(new MotionComponent());
            GetMotionComponent(characterMeshEntity)->SetDescriptorPath("~res:/TestCharacterControllerModule/character/character_motion.yaml");
            testCharacterEntity->AddNode(characterMeshEntity);

            ScopedPtr<Scene> weaponScene(new Scene());
            weaponScene->LoadScene("~res:/TestCharacterControllerModule/character/weapon_mesh.sc2");
            Entity* m4Entity = weaponScene->FindByName("weapon");
            if (m4Entity != nullptr)
            {
                ScopedPtr<Entity> weaponEntity(m4Entity->Clone());
                weaponEntity->SetName("Weapon");
                characterMeshEntity->AddNode(weaponEntity);
            }
        }
    }
}

void TestCharacterControllerModule::Shutdown()
{
    SafeRelease(testCharacterEntity);
}

bool TestCharacterControllerModule::EnableController(DAVA::Scene* scene, const Vector3& spawnPoint)
{
    CheckCharacterResources();

    if (testCharacterEntity == nullptr)
        return false;

    if (activeControllers.count(scene) != 0)
        return false;

    SceneContext& context = activeControllers[scene];

    context.characterControllerSystem = new TestCharacterControllerSystem(scene);
    scene->AddSystem(context.characterControllerSystem, 0, Scene::SCENE_SYSTEM_REQUIRE_PROCESS | Scene::SCENE_SYSTEM_REQUIRE_INPUT, scene->motionSystem);

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
    context.characterMoveSystem = new TestCharacterMoveSystem(scene);
    scene->AddSystem(context.characterMoveSystem, 0, Scene::SCENE_SYSTEM_REQUIRE_PROCESS, scene->physicsSystem);
#endif

    context.characterWeaponSystem = new TestCharacterWeaponSystem(scene);
    scene->AddSystem(context.characterWeaponSystem, 0, Scene::SCENE_SYSTEM_REQUIRE_PROCESS, scene->transformSystem);

    context.characterCameraSystem = new TestCharacterCameraSystem(scene);
    scene->AddSystem(context.characterCameraSystem, 0, Scene::SCENE_SYSTEM_REQUIRE_PROCESS, scene->renderUpdateSystem);

    context.characterEntity = testCharacterEntity->Clone();
    TransformComponent* tc = context.characterEntity->GetComponent<TransformComponent>();
    tc->SetLocalTranslation(spawnPoint);
    scene->AddNode(context.characterEntity);

    context.characterControllerSystem->SetCharacterEntity(context.characterEntity);

    return true;
}

bool TestCharacterControllerModule::DisableController(DAVA::Scene* scene)
{
    if (activeControllers.count(scene) == 0)
        return false;

    SceneContext& context = activeControllers[scene];

    context.characterControllerSystem->SetCharacterEntity(nullptr);

    scene->RemoveSystem(context.characterControllerSystem);
    SafeDelete(context.characterControllerSystem);

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
    scene->RemoveSystem(context.characterMoveSystem);
    SafeDelete(context.characterMoveSystem);
#endif

    scene->RemoveSystem(context.characterWeaponSystem);
    SafeDelete(context.characterWeaponSystem);

    scene->RemoveSystem(context.characterCameraSystem);
    SafeDelete(context.characterCameraSystem);

    if (context.characterEntity != nullptr)
    {
        scene->RemoveNode(context.characterEntity);
        SafeRelease(context.characterEntity);
    }

    activeControllers.erase(scene);

    return true;
}

TestCharacterControllerSystem* TestCharacterControllerModule::GetCharacterControllerSystem(DAVA::Scene* scene) const
{
    auto found = activeControllers.find(scene);
    if (found != activeControllers.end())
        return found->second.characterControllerSystem;
    else
        return nullptr;
}

}; //ns
