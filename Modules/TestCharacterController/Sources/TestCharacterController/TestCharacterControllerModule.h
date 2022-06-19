#pragma once

#include "Base/BaseMath.h"
#include "Entity/SceneSystem.h"

#include <ModuleManager/IModule.h>
#include <ModuleManager/ModuleManager.h>

namespace DAVA
{
class Entity;
class TestCharacterMoveSystem;
class TestCharacterWeaponSystem;
class TestCharacterCameraSystem;
class TestCharacterControllerSystem;

class TestCharacterControllerModule : public IModule
{
public:
    TestCharacterControllerModule(Engine* engine);

    void Shutdown() override;

    bool EnableController(DAVA::Scene* scene, const DAVA::Vector3& spawnPoint = DAVA::Vector3(0.f, 0.f, 100.f));
    bool DisableController(DAVA::Scene* scene);

    TestCharacterControllerSystem* GetCharacterControllerSystem(DAVA::Scene* scene) const;

protected:
    struct SceneContext
    {
        TestCharacterControllerSystem* characterControllerSystem = nullptr;
        TestCharacterMoveSystem* characterMoveSystem = nullptr;
        TestCharacterWeaponSystem* characterWeaponSystem = nullptr;
        TestCharacterCameraSystem* characterCameraSystem = nullptr;

        DAVA::Entity* characterEntity = nullptr;
    };

    void CheckCharacterResources();

    DAVA::Map<DAVA::Scene*, SceneContext> activeControllers;
    DAVA::Entity* testCharacterEntity = nullptr;

    DAVA_VIRTUAL_REFLECTION(TestCharacterControllerModule, IModule);
};
};
