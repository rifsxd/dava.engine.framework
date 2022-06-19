#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsModule.h"
#include "Physics/PhysicsConfigs.h"
#include "Physics/PhysicsComponent.h"
#include "Physics/CollisionShapeComponent.h"
#include "Physics/BoxShapeComponent.h"
#include "Physics/CapsuleShapeComponent.h"
#include "Physics/SphereShapeComponent.h"
#include "Physics/PlaneShapeComponent.h"
#include "Physics/StaticBodyComponent.h"
#include "Physics/DynamicBodyComponent.h"
#include "Physics/PhysicsGeometryCache.h"
#include "Physics/PhysicsUtils.h"
#include "Physics/BoxCharacterControllerComponent.h"
#include "Physics/CapsuleCharacterControllerComponent.h"
#include "Physics/CollisionSingleComponent.h"
#include "Physics/StaticBodyComponent.h"
#include "Physics/DynamicBodyComponent.h"
#include "Physics/ConvexHullShapeComponent.h"
#include "Physics/VehicleCarComponent.h"
#include "Physics/VehicleTankComponent.h"
#include "Physics/MeshShapeComponent.h"
#include "Physics/HeightFieldShapeComponent.h"

#include "Physics/Private/PhysicsMath.h"
#include "Physics/PhysicsVehiclesSubsystem.h"

#include <Scene3D/Entity.h>
#include <Entity/Component.h>

#include <Base/Type.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <ModuleManager/ModuleManager.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/SwitchComponent.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Render/RenderHelper.h>
#include <FileSystem/KeyedArchive.h>
#include <Utils/Utils.h>

#include <physx/PxScene.h>
#include <physx/PxRigidActor.h>
#include <physx/PxRigidDynamic.h>
#include <physx/common/PxRenderBuffer.h>
#include <PxShared/foundation/PxAllocatorCallback.h>
#include <PxShared/foundation/PxFoundation.h>

#include <functional>

namespace DAVA
{
namespace PhysicsSystemDetail
{
template <typename T>
void EraseComponent(T* component, Vector<T*>& pendingComponents, Vector<T*>& components)
{
    auto addIter = std::find(pendingComponents.begin(), pendingComponents.end(), component);
    if (addIter != pendingComponents.end())
    {
        RemoveExchangingWithLast(pendingComponents, std::distance(pendingComponents.begin(), addIter));
    }
    else
    {
        auto iter = std::find(components.begin(), components.end(), component);
        if (iter != components.end())
        {
            RemoveExchangingWithLast(components, std::distance(components.begin(), iter));
        }
    }
}

void UpdateGlobalPose(physx::PxRigidActor* actor, const Matrix4& m, Vector3& scale)
{
    Vector3 position;
    Quaternion rotation;
    m.Decomposition(position, scale, rotation);
    actor->setGlobalPose(physx::PxTransform(PhysicsMath::Vector3ToPxVec3(position), PhysicsMath::QuaternionToPxQuat(rotation)));
}

void UpdateShapeLocalPose(physx::PxShape* shape, const Matrix4& m)
{
    Vector3 position;
    Vector3 scale;
    Quaternion rotation;
    m.Decomposition(position, scale, rotation);
    shape->setLocalPose(physx::PxTransform(PhysicsMath::Vector3ToPxVec3(position), PhysicsMath::QuaternionToPxQuat(rotation)));
}

bool IsCollisionShapeType(const Type* componentType)
{
    PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    const Vector<const Type*>& shapeComponents = module->GetShapeComponentTypes();
    return std::any_of(shapeComponents.begin(), shapeComponents.end(), [componentType](const Type* type) {
        return componentType == type;
    });
}

bool IsCharacterControllerType(const Type* componentType)
{
    return componentType->Is<BoxCharacterControllerComponent>() || componentType->Is<CapsuleCharacterControllerComponent>();
}

Vector3 AccumulateMeshInfo(Entity* e, Vector<PolygonGroup*>& groups)
{
    RenderObject* ro = GetRenderObject(e);
    if (ro != nullptr)
    {
        uint32 batchesCount = ro->GetRenderBatchCount();
        int32 maxLod = ro->GetMaxLodIndex();
        for (uint32 i = 0; i < batchesCount; ++i)
        {
            int32 lodIndex = -1;
            int32 switchIndex = -1;
            RenderBatch* batch = ro->GetRenderBatch(i, lodIndex, switchIndex);
            if (lodIndex == maxLod)
            {
                PolygonGroup* group = batch->GetPolygonGroup();
                if (group != nullptr)
                {
                    groups.push_back(group);
                }
            }
        }
    }

    return GetTransformComponent(e)->GetWorldTransform().GetScale();
}

PhysicsComponent* GetParentPhysicsComponent(Entity* entity)
{
    PhysicsComponent* physicsComponent = static_cast<PhysicsComponent*>(entity->GetComponent<StaticBodyComponent>());
    if (physicsComponent == nullptr)
    {
        physicsComponent = static_cast<PhysicsComponent*>(entity->GetComponent<DynamicBodyComponent>());
    }

    if (physicsComponent != nullptr)
    {
        return physicsComponent;
    }
    else
    {
        // Move up in the hierarchy
        Entity* parent = entity->GetParent();
        if (parent != nullptr)
        {
            return GetParentPhysicsComponent(parent);
        }
        else
        {
            return nullptr;
        }
    }
}

const uint32 DEFAULT_SIMULATION_BLOCK_SIZE = 16 * 1024 * 512;
} // namespace

physx::PxFilterFlags FilterShader(physx::PxFilterObjectAttributes attributes0,
                                  physx::PxFilterData filterData0,
                                  physx::PxFilterObjectAttributes attributes1,
                                  physx::PxFilterData filterData1,
                                  physx::PxPairFlags& pairFlags,
                                  const void* constantBlock,
                                  physx::PxU32 constantBlockSize)
{
    PX_UNUSED(attributes0);
    PX_UNUSED(attributes1);
    PX_UNUSED(constantBlockSize);
    PX_UNUSED(constantBlock);

    // PxFilterData for a shape is used this way:
    // - PxFilterData.word0 is used for engine-specific features (i.e. for CCD)
    // - PxFilterData.word1 is a bitmask for encoding type of object
    // - PxFilterData.word2 is a bitmask for encoding types of objects this object collides with
    // - PxFilterData.word3 is not used right now
    // Type of a shape and types it collides with can be set using CollisionShapeComponent::SetTypeMask and CollisionShapeComponent::SetTypeMaskToCollideWith methods

    if ((filterData0.word1 & filterData1.word2) == 0 &&
        (filterData1.word1 & filterData0.word2) == 0)
    {
        // If these types of objects do not collide, ignore this pair unless filter data for either of them changes
        return physx::PxFilterFlag::eSUPPRESS;
    }

    pairFlags =
    physx::PxPairFlag::eCONTACT_DEFAULT | // default collision processing
    physx::PxPairFlag::eNOTIFY_TOUCH_FOUND | // notify about a first contact
    physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS | // notify about ongoing contacts
    physx::PxPairFlag::eNOTIFY_CONTACT_POINTS; // report contact points

    if (CollisionShapeComponent::IsCCDEnabled(filterData0) || CollisionShapeComponent::IsCCDEnabled(filterData1))
    {
        pairFlags |= physx::PxPairFlag::eDETECT_CCD_CONTACT; // report continuous collision detection contacts
    }

    return physx::PxFilterFlag::eDEFAULT;
}

PhysicsSystem::SimulationEventCallback::SimulationEventCallback(DAVA::CollisionSingleComponent* targetCollisionSingleComponent)
    : targetCollisionSingleComponent(targetCollisionSingleComponent)
{
    DVASSERT(targetCollisionSingleComponent != nullptr);
}

void PhysicsSystem::SimulationEventCallback::onConstraintBreak(physx::PxConstraintInfo*, physx::PxU32)
{
}

void PhysicsSystem::SimulationEventCallback::onWake(physx::PxActor**, physx::PxU32)
{
}

void PhysicsSystem::SimulationEventCallback::onSleep(physx::PxActor**, physx::PxU32)
{
}

void PhysicsSystem::SimulationEventCallback::onTrigger(physx::PxTriggerPair*, physx::PxU32)
{
}

void PhysicsSystem::SimulationEventCallback::onAdvance(const physx::PxRigidBody* const*, const physx::PxTransform*, const physx::PxU32)
{
}

void PhysicsSystem::SimulationEventCallback::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
    // Buffer for extracting physx contact points
    static const size_t MAX_CONTACT_POINTS_COUNT = 10;
    static physx::PxContactPairPoint physxContactPoints[MAX_CONTACT_POINTS_COUNT];

    for (physx::PxU32 i = 0; i < nbPairs; ++i)
    {
        const physx::PxContactPair& pair = pairs[i];

        if (pair.contactCount > 0)
        {
            if ((pair.flags & physx::PxContactPairFlag::eREMOVED_SHAPE_0) ||
                (pair.flags & physx::PxContactPairFlag::eREMOVED_SHAPE_1))
            {
                // Either first or second shape has been removed from the scene
                // Do not report such contacts
                continue;
            }

            // Extract physx points
            const physx::PxU32 contactPointsCount = pair.extractContacts(&physxContactPoints[0], MAX_CONTACT_POINTS_COUNT);
            DVASSERT(contactPointsCount > 0);

            Vector<CollisionPoint> davaContactPoints(contactPointsCount);

            // Convert each contact point from physx structure to engine structure
            for (size_t j = 0; j < contactPointsCount; ++j)
            {
                CollisionPoint& davaPoint = davaContactPoints[j];
                physx::PxContactPairPoint& physxPoint = physxContactPoints[j];

                davaPoint.position = PhysicsMath::PxVec3ToVector3(physxPoint.position);
                davaPoint.impulse = PhysicsMath::PxVec3ToVector3(physxPoint.impulse);
            }

            CollisionShapeComponent* firstCollisionComponent = CollisionShapeComponent::GetComponent(pair.shapes[0]);
            DVASSERT(firstCollisionComponent != nullptr);

            CollisionShapeComponent* secondCollisionComponent = CollisionShapeComponent::GetComponent(pair.shapes[1]);
            DVASSERT(secondCollisionComponent != nullptr);

            Entity* firstEntity = firstCollisionComponent->GetEntity();
            DVASSERT(firstEntity != nullptr);

            Entity* secondEntity = secondCollisionComponent->GetEntity();
            DVASSERT(secondEntity != nullptr);

            // Register collision
            CollisionInfo collisionInfo;
            collisionInfo.first = firstEntity;
            collisionInfo.second = secondEntity;
            collisionInfo.points = std::move(davaContactPoints);
            targetCollisionSingleComponent->collisions.push_back(collisionInfo);
        }
    }
}

PhysicsSystem::PhysicsSystem(Scene* scene)
    : SceneSystem(scene)
    , simulationEventCallback(scene->collisionSingleComponent)
{
    Engine* engine = Engine::Instance();
    uint32 threadCount = 2;
    Vector3 gravity(0.0, 0.0, -9.81f);
    simulationBlockSize = PhysicsSystemDetail::DEFAULT_SIMULATION_BLOCK_SIZE;
    if (engine != nullptr)
    {
        const KeyedArchive* options = Engine::Instance()->GetOptions();

        simulationBlockSize = options->GetUInt32("physics.simulationBlockSize", PhysicsSystemDetail::DEFAULT_SIMULATION_BLOCK_SIZE);
        DVASSERT((simulationBlockSize % (16 * 1024)) == 0); // simulationBlockSize must be 16K multiplier

        gravity = options->GetVector3("physics.gravity", gravity);
        threadCount = options->GetUInt32("physics.threadCount", threadCount);
    }

    const EngineContext* ctx = GetEngineContext();
    PhysicsModule* physics = ctx->moduleManager->GetModule<PhysicsModule>();
    simulationBlock = physics->Allocate(simulationBlockSize, "SimulationBlock", __FILE__, __LINE__);

    PhysicsSceneConfig sceneConfig;
    sceneConfig.gravity = gravity;
    sceneConfig.threadCount = threadCount;

    geometryCache = new PhysicsGeometryCache();

    physicsScene = physics->CreateScene(sceneConfig, FilterShader, &simulationEventCallback);

    vehiclesSubsystem = new PhysicsVehiclesSubsystem(scene, physicsScene);
    controllerManager = PxCreateControllerManager(*physicsScene);
}

PhysicsSystem::~PhysicsSystem()
{
    if (isSimulationRunning)
    {
        FetchResults(true);
    }

    SafeDelete(vehiclesSubsystem);

    DVASSERT(simulationBlock != nullptr);
    SafeDelete(geometryCache);

    controllerManager->release();

    const EngineContext* ctx = GetEngineContext();
    PhysicsModule* physics = ctx->moduleManager->GetModule<PhysicsModule>();
    physics->Deallocate(simulationBlock);
    simulationBlock = nullptr;
    physicsScene->release();
}

void PhysicsSystem::RegisterEntity(Entity* entity)
{
    auto processEntity = [this](Entity* entity, const Type* componentType)
    {
        for (uint32 i = 0; i < entity->GetComponentCount(componentType); ++i)
        {
            RegisterComponent(entity, entity->GetComponent(componentType, i));
        }
    };

    PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    const Vector<const Type*>& bodyComponents = module->GetBodyComponentTypes();
    const Vector<const Type*>& shapeComponents = module->GetShapeComponentTypes();
    const Vector<const Type*>& characterControllerComponents = module->GetCharacterControllerComponentTypes();

    for (const Type* type : bodyComponents)
    {
        processEntity(entity, type);
    }

    for (const Type* type : shapeComponents)
    {
        processEntity(entity, type);
    }

    for (const Type* type : characterControllerComponents)
    {
        processEntity(entity, type);
    }

    processEntity(entity, Type::Instance<RenderComponent>());

    vehiclesSubsystem->RegisterEntity(entity);
}

void PhysicsSystem::UnregisterEntity(Entity* entity)
{
    auto processEntity = [this](Entity* entity, const Type* componentType)
    {
        for (uint32 i = 0; i < entity->GetComponentCount(componentType); ++i)
        {
            UnregisterComponent(entity, entity->GetComponent(componentType, i));
        }
    };

    PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    const Vector<const Type*>& bodyComponents = module->GetBodyComponentTypes();
    const Vector<const Type*>& shapeComponents = module->GetShapeComponentTypes();
    const Vector<const Type*>& characterControllerComponents = module->GetCharacterControllerComponentTypes();

    for (const Type* type : bodyComponents)
    {
        processEntity(entity, type);
    }

    for (const Type* type : shapeComponents)
    {
        processEntity(entity, type);
    }

    for (const Type* type : characterControllerComponents)
    {
        processEntity(entity, type);
    }

    processEntity(entity, Type::Instance<RenderComponent>());

    vehiclesSubsystem->UnregisterEntity(entity);
}

void PhysicsSystem::RegisterComponent(Entity* entity, Component* component)
{
    const Type* componentType = component->GetType();
    if (componentType->Is<StaticBodyComponent>() || componentType->Is<DynamicBodyComponent>())
    {
        pendingAddPhysicsComponents.push_back(static_cast<PhysicsComponent*>(component));
    }

    using namespace PhysicsSystemDetail;
    if (IsCollisionShapeType(componentType))
    {
        pendingAddCollisionComponents.push_back(static_cast<CollisionShapeComponent*>(component));
    }

    if (IsCharacterControllerType(componentType))
    {
        pendingAddCharacterControllerComponents.push_back(static_cast<CharacterControllerComponent*>(component));
    }

    if (componentType->Is<RenderComponent>())
    {
        auto iter = waitRenderInfoComponents.find(entity);
        if (iter != waitRenderInfoComponents.end())
        {
            pendingAddCollisionComponents.insert(pendingAddCollisionComponents.end(), iter->second.begin(), iter->second.end());
            waitRenderInfoComponents.erase(iter);
        }
    }

    vehiclesSubsystem->RegisterComponent(entity, component);
}

void PhysicsSystem::UnregisterComponent(Entity* entity, Component* component)
{
    const Type* componentType = component->GetType();
    if (componentType->Is<StaticBodyComponent>() || componentType->Is<DynamicBodyComponent>())
    {
        PhysicsComponent* physicsComponent = static_cast<PhysicsComponent*>(component);
        PhysicsSystemDetail::EraseComponent(physicsComponent, pendingAddPhysicsComponents, physicsComponents);

        physx::PxActor* actor = physicsComponent->GetPxActor();
        if (actor != nullptr)
        {
            physx::PxRigidActor* rigidActor = actor->is<physx::PxRigidActor>();
            if (rigidActor != nullptr)
            {
                physx::PxU32 shapesCount = rigidActor->getNbShapes();
                Vector<physx::PxShape*> shapes(shapesCount, nullptr);
                rigidActor->getShapes(shapes.data(), shapesCount);

                for (physx::PxShape* shape : shapes)
                {
                    DVASSERT(shape != nullptr);
                    rigidActor->detachShape(*shape);
                }
            }
            physicsScene->removeActor(*actor);
            physicsComponent->ReleasePxActor();
        }

        if (componentType->Is<DynamicBodyComponent>())
        {
            size_t index = 0;
            while (index < forces.size())
            {
                PendingForce& force = forces[index];
                if (force.component == component)
                {
                    RemoveExchangingWithLast(forces, index);
                }
                else
                {
                    ++index;
                }
            }
        }
    }

    using namespace PhysicsSystemDetail;
    if (IsCollisionShapeType(componentType))
    {
        CollisionShapeComponent* collisionComponent = static_cast<CollisionShapeComponent*>(component);
        PhysicsSystemDetail::EraseComponent(collisionComponent, pendingAddCollisionComponents, collisionComponents);

        auto iter = waitRenderInfoComponents.find(entity);
        if (iter != waitRenderInfoComponents.end())
        {
            FindAndRemoveExchangingWithLast(iter->second, collisionComponent);
        }

        ReleaseShape(collisionComponent);
    }

    if (IsCharacterControllerType(componentType))
    {
        CharacterControllerComponent* controllerComponent = static_cast<CharacterControllerComponent*>(component);
        PhysicsSystemDetail::EraseComponent(controllerComponent, pendingAddCharacterControllerComponents, characterControllerComponents);

        if (controllerComponent->controller != nullptr)
        {
            controllerComponent->controller->release();
        }
    }

    if (componentType->Is<RenderComponent>())
    {
        Vector<CollisionShapeComponent*>* waitingComponents = nullptr;
        auto collisionProcess = [&](const Type* componentType)
        {
            for (uint32 i = 0; i < entity->GetComponentCount(componentType); ++i)
            {
                CollisionShapeComponent* component = static_cast<CollisionShapeComponent*>(entity->GetComponent(componentType, i));
                auto iter = std::find(collisionComponents.begin(), collisionComponents.end(), component);
                if (iter == collisionComponents.end())
                {
                    continue;
                }

                if (waitingComponents == nullptr)
                {
                    waitingComponents = &waitRenderInfoComponents[entity];
                }

                waitingComponents->push_back(component);
                ReleaseShape(component);
                PhysicsSystemDetail::EraseComponent(component, pendingAddCollisionComponents, collisionComponents);
            }
        };

        collisionProcess(Type::Instance<ConvexHullShapeComponent>());
        collisionProcess(Type::Instance<MeshShapeComponent>());
        collisionProcess(Type::Instance<HeightFieldShapeComponent>());
    }

    vehiclesSubsystem->UnregisterComponent(entity, component);
}

void PhysicsSystem::PrepareForRemove()
{
    waitRenderInfoComponents.clear();
    physicsComponensUpdatePending.clear();
    collisionComponentsUpdatePending.clear();
    for (CollisionShapeComponent* component : collisionComponents)
    {
        ReleaseShape(component);
    }
    collisionComponents.clear();

    for (CollisionShapeComponent* component : pendingAddCollisionComponents)
    {
        ReleaseShape(component);
    }
    pendingAddCollisionComponents.clear();

    for (PhysicsComponent* component : physicsComponents)
    {
        physx::PxActor* actor = component->GetPxActor();
        if (actor != nullptr)
        {
            physicsScene->removeActor(*actor);
            component->ReleasePxActor();
        }
    }
    physicsComponents.clear();

    for (PhysicsComponent* component : pendingAddPhysicsComponents)
    {
        physx::PxActor* actor = component->GetPxActor();
        if (actor != nullptr)
        {
            physicsScene->removeActor(*actor);
            component->ReleasePxActor();
        }
    }
    pendingAddPhysicsComponents.clear();
}

void PhysicsSystem::Process(float32 timeElapsed)
{
    if (isSimulationRunning == true)
    {
        FetchResults(false);
    }

    if (isSimulationRunning == false)
    {
        InitNewObjects();
        UpdateComponents();

        if (isSimulationEnabled == false)
        {
            SyncTransformToPhysx();
        }
        else
        {
            ApplyForces();
            DrawDebugInfo();

            vehiclesSubsystem->Simulate(timeElapsed);
            MoveCharacterControllers(timeElapsed);
            physicsScene->simulate(timeElapsed, nullptr, simulationBlock, simulationBlockSize);

            isSimulationRunning = true;
        }
    }
}

void PhysicsSystem::SetSimulationEnabled(bool isEnabled)
{
    if (isSimulationEnabled != isEnabled)
    {
        if (isSimulationRunning == true)
        {
            DVASSERT(isSimulationEnabled == true);
            bool success = FetchResults(true);
            DVASSERT(success == true);
        }

        isSimulationEnabled = isEnabled;

        vehiclesSubsystem->OnSimulationEnabled(isSimulationEnabled);
    }
}

bool PhysicsSystem::IsSimulationEnabled() const
{
    return isSimulationEnabled;
}

void PhysicsSystem::SetDebugDrawEnabled(bool drawDebugInfo_)
{
    drawDebugInfo = drawDebugInfo_;
    physx::PxReal enabled = drawDebugInfo == true ? 1.0f : 0.0f;
    physicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, enabled);
    physicsScene->setVisualizationParameter(physx::PxVisualizationParameter::eBODY_MASS_AXES, enabled);
}

bool PhysicsSystem::IsDebugDrawEnabled() const
{
    return drawDebugInfo;
}

bool PhysicsSystem::FetchResults(bool waitForFetchFinish)
{
    DVASSERT(isSimulationRunning);
    bool isFetched = physicsScene->fetchResults(waitForFetchFinish);
    if (isFetched == true)
    {
        isSimulationRunning = false;
        physx::PxU32 actorsCount = 0;
        physx::PxActor** actors = physicsScene->getActiveActors(actorsCount);

        for (physx::PxU32 i = 0; i < actorsCount; ++i)
        {
            physx::PxActor* actor = actors[i];
            PhysicsComponent* component = PhysicsComponent::GetComponent(actor);

            // When character controller is created, actor is created by physx implicitly
            // In this case there is no PhysicsComponent attached to this entity
            if (component != nullptr)
            {
                Entity* entity = component->GetEntity();

                physx::PxRigidActor* rigidActor = actor->is<physx::PxRigidActor>();
                DVASSERT(rigidActor != nullptr);

                // Update entity's transform and its shapes down the hierarchy recursively

                Matrix4 scaleMatrix = Matrix4::MakeScale(component->currentScale);
                TransformComponent* entityTransform = entity->GetComponent<TransformComponent>();
                entityTransform->SetWorldMatrix(scaleMatrix * PhysicsMath::PxMat44ToMatrix4(rigidActor->getGlobalPose()));

                Vector<Entity*> children;
                entity->GetChildEntitiesWithCondition(children, [component](Entity* e) { return PhysicsSystemDetail::GetParentPhysicsComponent(e) == component; });

                for (Entity* child : children)
                {
                    DVASSERT(child != nullptr);

                    Vector<CollisionShapeComponent*> shapes = PhysicsUtils::GetShapeComponents(child);
                    if (shapes.size() > 0)
                    {
                        // Update entity using just first shape for now
                        CollisionShapeComponent* shape = shapes[0];

                        Matrix4 scaleMatrix = Matrix4::MakeScale(shape->scale);
                        TransformComponent* childTransform = child->GetComponent<TransformComponent>();
                        childTransform->SetLocalMatrix(scaleMatrix * PhysicsMath::PxMat44ToMatrix4(shape->GetPxShape()->getLocalPose()));
                    }
                }
            }
        }
    }

    return isFetched;
}

void PhysicsSystem::DrawDebugInfo()
{
    DVASSERT(isSimulationRunning == false);
    DVASSERT(isSimulationEnabled == true);
    if (IsDebugDrawEnabled() == false)
    {
        return;
    }

    RenderHelper* renderHelper = GetScene()->GetRenderSystem()->GetDebugDrawer();
    const physx::PxRenderBuffer& rb = physicsScene->getRenderBuffer();
    const physx::PxDebugLine* lines = rb.getLines();
    for (physx::PxU32 i = 0; i < rb.getNbLines(); ++i)
    {
        const physx::PxDebugLine& line = lines[i];
        renderHelper->DrawLine(PhysicsMath::PxVec3ToVector3(line.pos0), PhysicsMath::PxVec3ToVector3(line.pos1),
                               PhysicsMath::PxColorToColor(line.color0));
    }

    const physx::PxDebugTriangle* triangles = rb.getTriangles();
    for (physx::PxU32 i = 0; i < rb.getNbTriangles(); ++i)
    {
        const physx::PxDebugTriangle& triangle = triangles[i];
        Polygon3 polygon;
        polygon.AddPoint(PhysicsMath::PxVec3ToVector3(triangle.pos0));
        polygon.AddPoint(PhysicsMath::PxVec3ToVector3(triangle.pos1));
        polygon.AddPoint(PhysicsMath::PxVec3ToVector3(triangle.pos2));
        renderHelper->DrawPolygon(polygon, PhysicsMath::PxColorToColor(triangle.color0), RenderHelper::DRAW_WIRE_DEPTH);
    }

    const physx::PxDebugPoint* points = rb.getPoints();
    for (physx::PxU32 i = 0; i < rb.getNbPoints(); ++i)
    {
        const physx::PxDebugPoint& point = points[i];
        renderHelper->DrawIcosahedron(PhysicsMath::PxVec3ToVector3(point.pos), 5.0f, PhysicsMath::PxColorToColor(point.color), RenderHelper::DRAW_WIRE_DEPTH);
    }
}

void PhysicsSystem::InitNewObjects()
{
    PhysicsModule* physics = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    for (PhysicsComponent* component : pendingAddPhysicsComponents)
    {
        const Type* componentType = component->GetType();
        physx::PxActor* createdActor = nullptr;
        if (componentType->Is<StaticBodyComponent>())
        {
            createdActor = physics->CreateStaticActor();
        }
        else
        {
            DVASSERT(componentType->Is<DynamicBodyComponent>());
            createdActor = physics->CreateDynamicActor();
        }

        component->SetPxActor(createdActor);
        physx::PxRigidActor* rigidActor = createdActor->is<physx::PxRigidActor>();
        Vector3 scale;

        TransformComponent* transform = component->GetEntity()->GetComponent<TransformComponent>();
        PhysicsSystemDetail::UpdateGlobalPose(rigidActor, transform->GetWorldMatrix(), scale);
        component->currentScale = scale;

        Entity* entity = component->GetEntity();
        AttachShapesRecursively(entity, component, scale);

        physicsScene->addActor(*(component->GetPxActor()));
        physicsComponents.push_back(component);
    }
    pendingAddPhysicsComponents.clear();

    for (CollisionShapeComponent* component : pendingAddCollisionComponents)
    {
        physx::PxShape* shape = CreateShape(component, physics);
        if (shape != nullptr)
        {
            Entity* entity = component->GetEntity();
            const Transform& worldTransform = GetTransformComponent(entity)->GetLocalTransform();
            Vector3 scale = worldTransform.GetScale();

            PhysicsComponent* physicsComponent = PhysicsSystemDetail::GetParentPhysicsComponent(entity);
            if (physicsComponent != nullptr)
            {
                AttachShape(physicsComponent, component, scale);

                if (physicsComponent->GetType()->Is<DynamicBodyComponent>())
                {
                    physx::PxRigidDynamic* dynamicActor = physicsComponent->GetPxActor()->is<physx::PxRigidDynamic>();
                    DVASSERT(dynamicActor != nullptr);
                    if (dynamicActor->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_SIMULATION) == false &&
                        dynamicActor->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC) == false)
                    {
                        dynamicActor->wakeUp();
                    }
                }
            }

            collisionComponents.push_back(component);
            shape->release();
        }
    }
    pendingAddCollisionComponents.clear();

    for (CharacterControllerComponent* component : pendingAddCharacterControllerComponents)
    {
        Entity* entity = component->GetEntity();
        DVASSERT(entity != nullptr);

        // Character controllers are only allowed to be root objects
        DVASSERT(entity->GetParent() == entity->GetScene());

        const EngineContext* ctx = GetEngineContext();
        PhysicsModule* physics = ctx->moduleManager->GetModule<PhysicsModule>();

        physx::PxController* controller = nullptr;
        if (component->GetType()->Is<BoxCharacterControllerComponent>())
        {
            BoxCharacterControllerComponent* boxCharacterControllerComponent = static_cast<BoxCharacterControllerComponent*>(component);

            physx::PxBoxControllerDesc desc;
            desc.position = PhysicsMath::Vector3ToPxExtendedVec3(GetTransformComponent(entity)->GetLocalTransform().GetTranslation());
            desc.halfHeight = boxCharacterControllerComponent->GetHalfHeight();
            desc.halfForwardExtent = boxCharacterControllerComponent->GetHalfForwardExtent();
            desc.halfSideExtent = boxCharacterControllerComponent->GetHalfSideExtent();
            desc.upDirection = PhysicsMath::Vector3ToPxVec3(Vector3::UnitZ);
            desc.material = physics->GetMaterial(FastName());
            DVASSERT(desc.isValid());

            controller = controllerManager->createController(desc);
        }
        else if (component->GetType()->Is<CapsuleCharacterControllerComponent>())
        {
            CapsuleCharacterControllerComponent* capsuleCharacterControllerComponent = static_cast<CapsuleCharacterControllerComponent*>(component);

            physx::PxCapsuleControllerDesc desc;
            desc.position = PhysicsMath::Vector3ToPxExtendedVec3(GetTransformComponent(entity)->GetLocalTransform().GetTranslation());
            desc.radius = capsuleCharacterControllerComponent->GetRadius();
            desc.height = capsuleCharacterControllerComponent->GetHeight();
            desc.material = physics->GetMaterial(FastName());
            desc.upDirection = PhysicsMath::Vector3ToPxVec3(Vector3::UnitZ);
            DVASSERT(desc.isValid());

            controller = controllerManager->createController(desc);
        }

        DVASSERT(controller != nullptr);
        component->controller = controller;

        characterControllerComponents.push_back(component);
    }
    pendingAddCharacterControllerComponents.clear();
}

void PhysicsSystem::AttachShape(PhysicsComponent* bodyComponent, CollisionShapeComponent* shapeComponent, const Vector3& scale)
{
    physx::PxActor* actor = bodyComponent->GetPxActor();
    DVASSERT(actor);
    physx::PxRigidActor* rigidActor = actor->is<physx::PxRigidActor>();
    DVASSERT(rigidActor);

    shapeComponent->scale = scale;

    physx::PxShape* shape = shapeComponent->GetPxShape();
    if (shape != nullptr)
    {
        rigidActor->attachShape(*shape);
        ScheduleUpdate(shapeComponent);
        ScheduleUpdate(bodyComponent);
    }
}

void PhysicsSystem::AttachShapesRecursively(Entity* entity, PhysicsComponent* bodyComponent, const Vector3& scale)
{
    for (const Type* type : GetEngineContext()->moduleManager->GetModule<PhysicsModule>()->GetShapeComponentTypes())
    {
        for (uint32 i = 0; i < entity->GetComponentCount(type); ++i)
        {
            AttachShape(bodyComponent, static_cast<CollisionShapeComponent*>(entity->GetComponent(type, i)), scale);
        }
    }

    const int32 childrenCount = entity->GetChildrenCount();
    for (int32 i = 0; i < childrenCount; ++i)
    {
        Entity* child = entity->GetChild(i);
        if (child->GetComponent<DynamicBodyComponent>() || child->GetComponent<StaticBodyComponent>())
        {
            continue;
        }

        AttachShapesRecursively(child, bodyComponent, scale);
    }
}

physx::PxShape* PhysicsSystem::CreateShape(CollisionShapeComponent* component, PhysicsModule* physics)
{
    using namespace PhysicsSystemDetail;
    physx::PxShape* shape = nullptr;

    const Type* componentType = component->GetType();

    if (componentType->Is<BoxShapeComponent>())
    {
        BoxShapeComponent* boxShape = static_cast<BoxShapeComponent*>(component);
        shape = physics->CreateBoxShape(boxShape->GetHalfSize(), component->GetMaterialName());
    }

    else if (componentType->Is<CapsuleShapeComponent>())
    {
        CapsuleShapeComponent* capsuleShape = static_cast<CapsuleShapeComponent*>(component);
        shape = physics->CreateCapsuleShape(capsuleShape->GetRadius(), capsuleShape->GetHalfHeight(), component->GetMaterialName());
    }

    else if (componentType->Is<SphereShapeComponent>())
    {
        SphereShapeComponent* sphereShape = static_cast<SphereShapeComponent*>(component);
        shape = physics->CreateSphereShape(sphereShape->GetRadius(), component->GetMaterialName());
    }

    else if (componentType->Is<PlaneShapeComponent>())
    {
        shape = physics->CreatePlaneShape(component->GetMaterialName());
    }

    else if (componentType->Is<ConvexHullShapeComponent>())
    {
        Vector<PolygonGroup*> groups;
        Entity* entity = component->GetEntity();
        Vector3 scale = AccumulateMeshInfo(entity, groups);
        if (groups.empty() == false)
        {
            shape = physics->CreateConvexHullShape(std::move(groups), scale, component->GetMaterialName(), geometryCache);
        }
        else
        {
            waitRenderInfoComponents[entity].push_back(component);
        }
    }

    else if (componentType->Is<MeshShapeComponent>())
    {
        Vector<PolygonGroup*> groups;
        Entity* entity = component->GetEntity();
        Vector3 scale = AccumulateMeshInfo(entity, groups);
        if (groups.empty() == false)
        {
            shape = physics->CreateMeshShape(std::move(groups), scale, component->GetMaterialName(), geometryCache);
        }
        else
        {
            waitRenderInfoComponents[entity].push_back(component);
        }
    }

    else if (componentType->Is<HeightFieldShapeComponent>())
    {
        Entity* entity = component->GetEntity();
        Landscape* landscape = GetLandscape(entity);
        if (landscape != nullptr)
        {
            Matrix4 localPose;
            shape = physics->CreateHeightField(landscape, component->GetMaterialName(), localPose);
            component->SetLocalPose(localPose);
        }
        else
        {
            waitRenderInfoComponents[entity].push_back(component);
        }
    }

    else
    {
        DVASSERT(false);
    }

    if (shape != nullptr)
    {
        component->SetPxShape(shape);

        // It's user's responsibility to setup drivable surfaces to work with vehicles
        // For simplicity do it for every landscape by default
        if (component->GetType()->Is<HeightFieldShapeComponent>())
        {
            vehiclesSubsystem->SetupDrivableSurface(component);
        }
    }

    return shape;
}

void PhysicsSystem::SyncTransformToPhysx()
{
    TransformSingleComponent* transformSingle = GetScene()->transformSingleComponent;
    for (Entity* entity : transformSingle->localTransformChanged)
    {
        SyncEntityTransformToPhysx(entity);
    }

    for (auto& mapNode : transformSingle->worldTransformChanged.map)
    {
        for (Entity* entity : mapNode.second)
        {
            SyncEntityTransformToPhysx(entity);
        }
    }
}

void PhysicsSystem::SyncEntityTransformToPhysx(Entity* entity)
{
    DVASSERT(isSimulationEnabled == false);
    DVASSERT(isSimulationRunning == false);
    TransformSingleComponent* transformSingle = GetScene()->transformSingleComponent;
    if (transformSingle == nullptr)
    {
        return;
    }

    auto updatePose = [](Entity* e, PhysicsComponent* component)
    {
        if (component != nullptr)
        {
            physx::PxActor* actor = component->GetPxActor();
            DVASSERT(actor != nullptr);
            physx::PxRigidActor* rigidActor = actor->is<physx::PxRigidActor>();
            DVASSERT(rigidActor != nullptr);
            Vector3 scale;
            PhysicsSystemDetail::UpdateGlobalPose(rigidActor, GetTransformComponent(e)->GetLocalMatrix(), scale);

            physx::PxU32 shapesCount = rigidActor->getNbShapes();
            for (physx::PxU32 i = 0; i < shapesCount; ++i)
            {
                physx::PxShape* shape = nullptr;
                rigidActor->getShapes(&shape, 1, i);

                // Update local pose for nested shapes

                CollisionShapeComponent* shapeComponent = CollisionShapeComponent::GetComponent(shape);
                DVASSERT(shapeComponent->GetEntity() != nullptr);
                if (shapeComponent->GetEntity() != e)
                {
                    PhysicsSystemDetail::UpdateShapeLocalPose(shape, GetTransformComponent(shapeComponent->GetEntity())->GetLocalMatrix());
                }

                // Update geometry scale

                physx::PxGeometryHolder geomHolder = shape->getGeometry();

                physx::PxGeometryType::Enum geomType = geomHolder.getType();
                if (geomType == physx::PxGeometryType::eTRIANGLEMESH)
                {
                    physx::PxTriangleMeshGeometry geom;
                    bool extracted = shape->getTriangleMeshGeometry(geom);
                    DVASSERT(extracted);
                    geom.scale.scale = PhysicsMath::Vector3ToPxVec3(scale);
                    shape->setGeometry(geom);
                }
                else if (geomType == physx::PxGeometryType::eCONVEXMESH)
                {
                    physx::PxConvexMeshGeometry geom;
                    bool extracted = shape->getConvexMeshGeometry(geom);
                    DVASSERT(extracted);
                    geom.scale.scale = PhysicsMath::Vector3ToPxVec3(scale);
                    shape->setGeometry(geom);
                }
            }
        }
    };

    PhysicsComponent* staticBodyComponent = static_cast<PhysicsComponent*>(entity->GetComponent<StaticBodyComponent>());
    updatePose(entity, staticBodyComponent);

    PhysicsComponent* dynamicBodyComponent = static_cast<PhysicsComponent*>(entity->GetComponent<DynamicBodyComponent>());
    updatePose(entity, dynamicBodyComponent);

    for (int32 i = 0; i < entity->GetChildrenCount(); ++i)
    {
        SyncEntityTransformToPhysx(entity->GetChild(i));
    }
}

void PhysicsSystem::ReleaseShape(CollisionShapeComponent* component)
{
    physx::PxShape* shape = component->GetPxShape();
    if (shape == nullptr)
    {
        return;
    }
    DVASSERT(shape->isExclusive() == true);

    physx::PxActor* actor = shape->getActor();
    if (actor != nullptr)
    {
        actor->is<physx::PxRigidActor>()->detachShape(*shape);
    }

    component->ReleasePxShape();
}

void PhysicsSystem::ScheduleUpdate(PhysicsComponent* component)
{
    physicsComponensUpdatePending.insert(component);
}

void PhysicsSystem::ScheduleUpdate(CollisionShapeComponent* component)
{
    collisionComponentsUpdatePending.insert(component);
}

void PhysicsSystem::ScheduleUpdate(CharacterControllerComponent* component)
{
    characterControllerComponentsUpdatePending.insert(component);
}

bool PhysicsSystem::Raycast(const Vector3& origin, const Vector3& direction, float32 distance, physx::PxRaycastCallback& callback)
{
    using namespace physx;

    return physicsScene->raycast(PhysicsMath::Vector3ToPxVec3(origin), PhysicsMath::Vector3ToPxVec3(Normalize(direction)),
                                 static_cast<PxReal>(distance), callback);
}

PhysicsVehiclesSubsystem* PhysicsSystem::GetVehiclesSystem()
{
    return vehiclesSubsystem;
}

void PhysicsSystem::UpdateComponents()
{
    PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    for (CollisionShapeComponent* shapeComponent : collisionComponentsUpdatePending)
    {
        shapeComponent->UpdateLocalProperties();
        physx::PxShape* shape = shapeComponent->GetPxShape();
        DVASSERT(shape != nullptr);
        physx::PxMaterial* material = module->GetMaterial(shapeComponent->GetMaterialName());
        shape->setMaterials(&material, 1);
        physx::PxActor* actor = shape->getActor();
        if (actor != nullptr)
        {
            PhysicsComponent* bodyComponent = PhysicsComponent::GetComponent(actor);
            physicsComponensUpdatePending.insert(bodyComponent);
        }
    }

    for (PhysicsComponent* bodyComponent : physicsComponensUpdatePending)
    {
        bodyComponent->UpdateLocalProperties();

        // Recalculate mass
        // Ignore vehicles, VehiclesSubsystem is responsible for setting correct values

        Entity* entity = bodyComponent->GetEntity();
        if (entity->GetComponent<VehicleCarComponent>() == nullptr &&
            entity->GetComponent<VehicleTankComponent>() == nullptr)
        {
            physx::PxRigidDynamic* dynamicActor = bodyComponent->GetPxActor()->is<physx::PxRigidDynamic>();
            if (dynamicActor != nullptr)
            {
                physx::PxU32 shapesCount = dynamicActor->getNbShapes();
                if (shapesCount > 0)
                {
                    Vector<physx::PxShape*> shapes(shapesCount, nullptr);
                    physx::PxU32 extractedShapesCount = dynamicActor->getShapes(shapes.data(), shapesCount);
                    DVASSERT(shapesCount == extractedShapesCount);

                    Vector<physx::PxReal> masses;
                    masses.reserve(shapesCount);

                    for (physx::PxShape* shape : shapes)
                    {
                        CollisionShapeComponent* shapeComponent = CollisionShapeComponent::GetComponent(shape);
                        masses.push_back(shapeComponent->GetMass());
                    }

                    physx::PxRigidBodyExt::setMassAndUpdateInertia(*dynamicActor, masses.data(), static_cast<physx::PxU32>(masses.size()));
                }
            }
        }

        if (bodyComponent->GetType()->Is<DynamicBodyComponent>())
        {
            DynamicBodyComponent* dynamicBody = static_cast<DynamicBodyComponent*>(bodyComponent);
            bool isCCDEnabled = dynamicBody->IsCCDEnabled();

            physx::PxRigidDynamic* actor = dynamicBody->GetPxActor()->is<physx::PxRigidDynamic>();
            DVASSERT(actor != nullptr);

            physx::PxU32 shapesCount = actor->getNbShapes();
            for (physx::PxU32 shapeIndex = 0; shapeIndex < shapesCount; ++shapeIndex)
            {
                physx::PxShape* shape = nullptr;
                actor->getShapes(&shape, 1, shapeIndex);
                CollisionShapeComponent::SetCCDEnabled(shape, isCCDEnabled);
            }
        }
    }

    for (CharacterControllerComponent* controllerComponent : characterControllerComponentsUpdatePending)
    {
        physx::PxController* controller = controllerComponent->controller;
        if (controller != nullptr)
        {
            // Update geometry if needed
            if (controllerComponent->geometryChanged)
            {
                if (controllerComponent->GetType()->Is<BoxCharacterControllerComponent>())
                {
                    BoxCharacterControllerComponent* boxComponent = static_cast<BoxCharacterControllerComponent*>(controllerComponent);
                    physx::PxBoxController* boxController = static_cast<physx::PxBoxController*>(controller);

                    boxController->setHalfHeight(boxComponent->GetHalfHeight());
                    boxController->setHalfForwardExtent(boxComponent->GetHalfForwardExtent());
                    boxController->setHalfSideExtent(boxComponent->GetHalfSideExtent());
                }
                else if (controllerComponent->GetType()->Is<CapsuleCharacterControllerComponent>())
                {
                    CapsuleCharacterControllerComponent* capsuleComponent = static_cast<CapsuleCharacterControllerComponent*>(controllerComponent);
                    physx::PxCapsuleController* capsuleController = static_cast<physx::PxCapsuleController*>(controller);

                    capsuleController->setRadius(capsuleComponent->GetRadius());
                    capsuleController->setHeight(capsuleComponent->GetHeight());
                }
                controllerComponent->geometryChanged = false;
            }

            // Teleport if needed
            if (controllerComponent->teleported)
            {
                controller->setPosition(PhysicsMath::Vector3ToPxExtendedVec3(controllerComponent->teleportDestination));
                controllerComponent->teleported = false;
            }
        }
    }

    collisionComponentsUpdatePending.clear();
    physicsComponensUpdatePending.clear();
    characterControllerComponentsUpdatePending.clear();
}

void PhysicsSystem::MoveCharacterControllers(float32 timeElapsed)
{
    for (CharacterControllerComponent* controllerComponent : characterControllerComponents)
    {
        physx::PxController* controller = controllerComponent->controller;
        if (controller != nullptr)
        {
            // Apply movement
            physx::PxControllerCollisionFlags collisionFlags;
            if (controllerComponent->GetMovementMode() == CharacterControllerComponent::MovementMode::Flying)
            {
                collisionFlags = controller->move(PhysicsMath::Vector3ToPxVec3(controllerComponent->totalDisplacement), 0.0f, timeElapsed, physx::PxControllerFilters());
            }
            else
            {
                DVASSERT(controllerComponent->GetMovementMode() == CharacterControllerComponent::MovementMode::Walking);

                // Ignore displacement along z axis
                Vector3 displacement = controllerComponent->totalDisplacement;
                displacement.z = 0.0f;

                // Apply gravity
                displacement += PhysicsMath::PxVec3ToVector3(physicsScene->getGravity()) * timeElapsed;

                collisionFlags = controller->move(PhysicsMath::Vector3ToPxVec3(displacement), 0.0f, timeElapsed, physx::PxControllerFilters());
            }

            controllerComponent->grounded = (collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN);
            controllerComponent->totalDisplacement = Vector3::Zero;

            // Sync entity's transform

            Entity* entity = controllerComponent->GetEntity();
            DVASSERT(entity != nullptr);

            TransformComponent* transform = entity->GetComponent<TransformComponent>();
            transform->SetLocalTranslation(PhysicsMath::PxExtendedVec3ToVector3(controller->getFootPosition()));
        }
    }
}

void PhysicsSystem::AddForce(DynamicBodyComponent* component, const Vector3& force, physx::PxForceMode::Enum mode)
{
    PendingForce pendingForce;
    pendingForce.component = component;
    pendingForce.force = force;
    pendingForce.mode = mode;

    forces.push_back(pendingForce);
}

void PhysicsSystem::ApplyForces()
{
    for (const PendingForce& force : forces)
    {
        physx::PxActor* actor = force.component->GetPxActor();
        DVASSERT(actor != nullptr);
        physx::PxRigidBody* rigidBody = actor->is<physx::PxRigidBody>();
        DVASSERT(rigidBody != nullptr);

        rigidBody->addForce(PhysicsMath::Vector3ToPxVec3(force.force), force.mode);
    }

    forces.clear();
}

} // namespace DAVA
