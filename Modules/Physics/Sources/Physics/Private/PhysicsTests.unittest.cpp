#include "UnitTests/UnitTests.h"
#include "Physics/PhysicsModule.h"
#include "Physics/StaticBodyComponent.h"
#include "Physics/DynamicBodyComponent.h"
#include "Physics/CollisionShapeComponent.h"
#include "Physics/BoxShapeComponent.h"
#include "Physics/Private/PhysicsSystemPrivate.h"

#include <Engine/Engine.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Entity/Component.h>
#include <Concurrency/Thread.h>

#include <physx/PxScene.h>
#include <physx/PxActor.h>
#include <physx/PxRigidStatic.h>
#include <physx/PxRigidDynamic.h>
#include <PxShared/foundation/PxFlags.h>

using namespace DAVA;

namespace PhysicsTestDetils
{
struct SceneInfo
{
    ScopedPtr<Scene> scene;
    ScopedPtr<Entity> entity;
};

SceneInfo CreateScene()
{
    SceneInfo info;
    info.scene.reset(new Scene());
    info.entity.reset(new Entity());

    info.scene->AddNode(info.entity);

    return info;
}

void Frame(SceneInfo& info)
{
    info.scene->Update(0.16f);
    while (PhysicsSystemPrivate::HasPendingComponents(info.scene->physicsSystem))
    {
        Thread::Sleep(16);
        info.scene->Update(0.16f);
    }
}

template <typename T>
T* AttachComponent(SceneInfo& info)
{
    T* component = new T();
    info.entity->AddComponent(component);
    return component;
}

void RemoveComponent(SceneInfo& info, Component* component)
{
    info.entity->RemoveComponent(component);
}

physx::PxScene* ExtractPxScene(const SceneInfo& info)
{
    return PhysicsSystemPrivate::GetPxScene(info.scene->physicsSystem);
}

template <typename T>
physx::PxShape* ExtractPxShape(const SceneInfo& info)
{
    CollisionShapeComponent* component = static_cast<CollisionShapeComponent*>(info.entity->GetComponent(Type::Instance<T>()));
    TEST_VERIFY(component != nullptr);
    return component->GetPxShape();
}

} // namespace PhysicsTestDetils

DAVA_TESTCLASS (PhysicsTest)
{
    DAVA_TEST (InitTest)
    {
        const ModuleManager& moduleManager = *GetEngineContext()->moduleManager;
        PhysicsModule* physicsModule = moduleManager.GetModule<PhysicsModule>();
        TEST_VERIFY(physicsModule->IsInitialized());
    }

    DAVA_TEST (AddStaticBodyTest)
    {
        using namespace PhysicsTestDetils;
        SceneInfo info = CreateScene();
        AttachComponent<StaticBodyComponent>(info);

        physx::PxScene* pxScene = ExtractPxScene(info);
        TEST_VERIFY(pxScene != nullptr);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_STATIC) == 0);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC) == 0);

        Frame(info);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_STATIC) == 1);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC) == 0);

        physx::PxActor* actor = nullptr;
        pxScene->getActors(physx::PxActorTypeFlag::eRIGID_STATIC, &actor, 1);
        TEST_VERIFY(actor != nullptr);
        physx::PxRigidStatic* staticActor = actor->is<physx::PxRigidStatic>();
        TEST_VERIFY(staticActor != nullptr);
        TEST_VERIFY(staticActor->getNbShapes() == 0);

        AttachComponent<BoxShapeComponent>(info);
        TEST_VERIFY(staticActor->getNbShapes() == 0);

        Frame(info);
        TEST_VERIFY(staticActor->getNbShapes() == 1);
        physx::PxShape* shape = nullptr;
        staticActor->getShapes(&shape, 1);
        TEST_VERIFY(shape != nullptr);
    }

    DAVA_TEST (CollisionFirstTest)
    {
        using namespace PhysicsTestDetils;
        SceneInfo info = CreateScene();
        AttachComponent<BoxShapeComponent>(info);

        physx::PxScene* pxScene = ExtractPxScene(info);
        TEST_VERIFY(pxScene != nullptr);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_STATIC) == 0);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC) == 0);

        Frame(info);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_STATIC) == 0);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC) == 0);

        physx::PxShape* shape = ExtractPxShape<BoxShapeComponent>(info);
        TEST_VERIFY(shape != nullptr);

        AttachComponent<StaticBodyComponent>(info);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_STATIC) == 0);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC) == 0);

        Frame(info);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_STATIC) == 1);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC) == 0);

        physx::PxActor* actor = nullptr;
        pxScene->getActors(physx::PxActorTypeFlag::eRIGID_STATIC, &actor, 1);
        TEST_VERIFY(actor != nullptr);
        physx::PxRigidStatic* staticActor = actor->is<physx::PxRigidStatic>();
        TEST_VERIFY(staticActor != nullptr);
        TEST_VERIFY(staticActor->getNbShapes() == 1);

        physx::PxShape* attachedShape = nullptr;
        staticActor->getShapes(&attachedShape, 1);
        TEST_VERIFY(attachedShape != nullptr);
        TEST_VERIFY(attachedShape == shape);
    }

    DAVA_TEST (DynamicBodyTest)
    {
        using namespace PhysicsTestDetils;
        SceneInfo info = CreateScene();
        AttachComponent<DynamicBodyComponent>(info);
        AttachComponent<BoxShapeComponent>(info);
        Frame(info);

        physx::PxScene* pxScene = ExtractPxScene(info);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC) == 1);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_STATIC) == 0);

        physx::PxActor* actor = nullptr;
        pxScene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC, &actor, 1);
        TEST_VERIFY(actor != nullptr);
        physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
        TEST_VERIFY(dynamicActor != nullptr);

        TEST_VERIFY(dynamicActor->isSleeping() == false);
        TEST_VERIFY(dynamicActor->getNbShapes() == 1);
    }

    DAVA_TEST (ReplaceBodyComponent)
    {
        using namespace PhysicsTestDetils;
        SceneInfo info = CreateScene();
        Component* componentToRemove = AttachComponent<DynamicBodyComponent>(info);
        AttachComponent<BoxShapeComponent>(info);
        Frame(info);

        RemoveComponent(info, componentToRemove);
        Frame(info);

        physx::PxScene* pxScene = ExtractPxScene(info);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC) == 0);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_STATIC) == 0);

        physx::PxShape* shape = ExtractPxShape<BoxShapeComponent>(info);
        TEST_VERIFY(shape != nullptr);

        AttachComponent<StaticBodyComponent>(info);
        Frame(info);

        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC) == 0);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_STATIC) == 1);

        physx::PxActor* actor = nullptr;
        pxScene->getActors(physx::PxActorTypeFlag::eRIGID_STATIC, &actor, 1);
        TEST_VERIFY(actor != nullptr);
        physx::PxRigidStatic* staticActor = actor->is<physx::PxRigidStatic>();
        TEST_VERIFY(staticActor != nullptr);
        TEST_VERIFY(staticActor->getNbShapes() == 1);
        physx::PxShape* attachedShape = nullptr;
        staticActor->getShapes(&attachedShape, 1);
        TEST_VERIFY(shape == attachedShape);
    }

    DAVA_TEST (RemoveCollisionTest)
    {
        using namespace PhysicsTestDetils;
        SceneInfo info = CreateScene();
        AttachComponent<DynamicBodyComponent>(info);
        Component* componentToRemove = AttachComponent<BoxShapeComponent>(info);
        Frame(info);

        RemoveComponent(info, componentToRemove);

        physx::PxScene* pxScene = ExtractPxScene(info);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC) == 1);
        TEST_VERIFY(pxScene->getNbActors(physx::PxActorTypeFlag::eRIGID_STATIC) == 0);

        physx::PxActor* actor = nullptr;
        TEST_VERIFY(pxScene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC, &actor, 1) == 1);
        physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();
        TEST_VERIFY(dynamicActor != nullptr);

        TEST_VERIFY(dynamicActor->getNbShapes() == 0);
    }

    DAVA_TEST (RaycastTest)
    {
        using namespace PhysicsTestDetils;
        SceneInfo info = CreateScene();
        Matrix4 localTransform = Matrix4::MakeTranslation(Vector3(90.0f, 0.0f, 0.0f));
        info.entity->GetComponent<TransformComponent>()->SetLocalMatrix(localTransform);

        StaticBodyComponent* bodyComponent = AttachComponent<StaticBodyComponent>(info);
        BoxShapeComponent* boxComponent = AttachComponent<BoxShapeComponent>(info);
        boxComponent->SetHalfSize(Vector3(5.0f, 5.0f, 5.0f));
        info.scene->transformSystem->Process(0.0f);
        Frame(info);

        {
            physx::PxRaycastBuffer hitBuffer;
            bool objectHit = info.scene->physicsSystem->Raycast(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), std::numeric_limits<float32>::max(), hitBuffer);
            TEST_VERIFY(objectHit == true);
            TEST_VERIFY(bodyComponent == PhysicsComponent::GetComponent(hitBuffer.block.actor));
            TEST_VERIFY(boxComponent == CollisionShapeComponent::GetComponent(hitBuffer.block.shape));
        }

        {
            physx::PxRaycastBuffer hitBuffer;
            bool objectHit = info.scene->physicsSystem->Raycast(Vector3(90.0f, 90.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f), std::numeric_limits<float32>::max(), hitBuffer);
            TEST_VERIFY(objectHit == true);
            TEST_VERIFY(bodyComponent == PhysicsComponent::GetComponent(hitBuffer.block.actor));
            TEST_VERIFY(boxComponent == CollisionShapeComponent::GetComponent(hitBuffer.block.shape));
        }

        {
            physx::PxRaycastBuffer hitBuffer;
            bool objectHit = info.scene->physicsSystem->Raycast(Vector3(0.0f, 00.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), 75.0f, hitBuffer);
            TEST_VERIFY(objectHit == false);
        }
    }
};
