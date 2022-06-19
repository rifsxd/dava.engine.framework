#include "REPlatform/Scene/BaseTransformProxies.h"
#include "REPlatform/Scene/SceneHelper.h"
#include "REPlatform/Scene/Systems/EditorParticlesSystem.h"
#include "REPlatform/DataNodes/SceneData.h"

#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Core/Deprecated.h>

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Math/Transform.h>
#include <Particles/ParticleEmitterInstance.h>
#include <Particles/ParticleForce.h>
#include <Particles/ParticleLayer.h>
#include <Physics/PhysicsModule.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Scene3D/Components/TransformComponent.h>
/*
 * EntityTransformProxy
 */
namespace DAVA
{
const Transform& EntityTransformProxy::GetWorldTransform(const Any& object)
{
    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<Entity>());

    TransformComponent* tc = obj.Cast<Entity>()->GetComponent<TransformComponent>();
    return tc->GetWorldTransform();
}

const Transform& EntityTransformProxy::GetLocalTransform(const Any& object)
{
    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<Entity>());
    TransformComponent* tc = obj.Cast<Entity>()->GetComponent<TransformComponent>();
    return tc->GetLocalTransform();
}

void EntityTransformProxy::SetLocalTransform(Any& object, const Transform& transform)
{
    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<Entity>());
    TransformComponent* tc = obj.Cast<Entity>()->GetComponent<TransformComponent>();
    return tc->SetLocalTransform(transform);
}

bool EntityTransformProxy::SupportsTransformType(const Any& object, Selectable::TransformType type) const
{
    using namespace DAVA;

    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<Entity>());
    Entity* entity = obj.Cast<Entity>();

    if (type == Selectable::TransformType::Scale)
    {
        PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();

        Vector<const Type*> physicsComponents = module->GetBodyComponentTypes();
        const Vector<const Type*>& shapeComponents = module->GetShapeComponentTypes();
        const Vector<const Type*>& vehicleComponents = module->GetVehicleComponentTypes();

        physicsComponents.insert(physicsComponents.end(), shapeComponents.begin(), shapeComponents.end());
        physicsComponents.insert(physicsComponents.end(), vehicleComponents.begin(), vehicleComponents.end());

        for (const Type* type : physicsComponents)
        {
            if (entity->GetComponentCount(type))
            {
                return false;
            }
        }
    }

    return (type == Selectable::TransformType::Disabled) || (entity->GetLocked() == false);
}

bool EntityTransformProxy::TransformDependsFromObject(const Any& dependant, const Any& dependsOn) const
{
    Selectable dependsOnSelectable(dependsOn);
    if (dependsOnSelectable.CanBeCastedTo<Entity>() == false)
    {
        return false;
    }
    Entity* dependsOnEntity = dependsOnSelectable.Cast<Entity>();
    DVASSERT(dependsOnEntity != nullptr);

    Selectable dependantSelectable(dependant);
    DVASSERT(dependantSelectable.CanBeCastedTo<Entity>());
    Entity* dependantEntity = dependantSelectable.Cast<Entity>();
    DVASSERT(dependantEntity != dependsOnEntity, "[TransformDependsFromObject] One object provided to both parameters");

    return SceneHelper::IsEntityChildRecursive(dependsOnEntity, dependantEntity);
}

/*
 * EmitterTransformProxy
 */
const Transform& EmitterTransformProxy::GetWorldTransform(const Any& object)
{
    static Transform currentMatrix;
    currentMatrix = TransformUtils::IDENTITY;

    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<ParticleEmitterInstance>());
    ParticleEmitterInstance* emitterInstance = obj.Cast<ParticleEmitterInstance>();

    auto ownerComponent = emitterInstance->GetOwner();
    if ((ownerComponent == nullptr) || (ownerComponent->GetEntity() == nullptr))
    {
        currentMatrix.SetTranslation(emitterInstance->GetSpawnPosition());
    }
    else
    {
        TransformComponent* tc = ownerComponent->GetEntity()->GetComponent<TransformComponent>();
        const Transform& entityTransform = tc->GetWorldTransform();
        Vector3 center = emitterInstance->GetSpawnPosition();
        TransformPerserveLength(center, Matrix3(TransformUtils::ToMatrix(entityTransform)));
        currentMatrix.SetTranslation(center + entityTransform.GetTranslation());
    }
    return currentMatrix;
}

const Transform& EmitterTransformProxy::GetLocalTransform(const Any& object)
{
    static Transform currentTransform;
    currentTransform = TransformUtils::IDENTITY;

    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<ParticleEmitterInstance>());
    ParticleEmitterInstance* emitterInstance = obj.Cast<ParticleEmitterInstance>();
    currentTransform.SetTranslation(emitterInstance->GetSpawnPosition());
    return currentTransform;
}

void EmitterTransformProxy::SetLocalTransform(Any& object, const Transform& transform)
{
    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<ParticleEmitterInstance>());
    ParticleEmitterInstance* emitterInstance = obj.Cast<ParticleEmitterInstance>();
    emitterInstance->SetSpawnPosition(transform.GetTranslation());
}

bool EmitterTransformProxy::SupportsTransformType(const Any& object, Selectable::TransformType type) const
{
    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<ParticleEmitterInstance>());
    ParticleEmitterInstance* emitterInstance = obj.Cast<ParticleEmitterInstance>();
    if (emitterInstance->GetOwner() == nullptr)
        return false;

    return (type == Selectable::TransformType::Disabled) || (type == Selectable::TransformType::Translation);
}

bool EmitterTransformProxy::TransformDependsFromObject(const Any& dependant, const Any& dependsOn) const
{
    DVASSERT(dependant != dependsOn, "[TransformDependsFromObject] One object provided to both parameters");

    Selectable dependesOnSelectable(dependsOn);
    if (dependesOnSelectable.CanBeCastedTo<Entity>() == false)
    {
        return false;
    }

    Entity* dependsOnEntity = dependesOnSelectable.Cast<Entity>();
    if (dependsOnEntity == nullptr)
        return false;

    Selectable dependantObj(dependant);
    DVASSERT(dependantObj.CanBeCastedTo<ParticleEmitterInstance>());
    ParticleEmitterInstance* emitter = dependantObj.Cast<ParticleEmitterInstance>();
    // check if emitter instance contained inside entity
    ParticleEffectComponent* component = dependsOnEntity->GetComponent<ParticleEffectComponent>();
    if (component != nullptr)
    {
        for (uint32 i = 0, e = component->GetEmittersCount(); i < e; ++i)
        {
            if (component->GetEmitterInstance(i) == emitter)
                return true;
        }
    }

    // or it's children
    for (int32 i = 0, e = dependsOnEntity->GetChildrenCount(); i < e; ++i)
    {
        if (TransformDependsFromObject(dependant, dependsOnEntity->GetChild(i)))
            return true;
    }

    return false;
}

/*
* ForceTransformProxy
*/
const Transform& ParticleForceTransformProxy::GetWorldTransform(const Any& object)
{
    static Transform currentMatrix;
    currentMatrix = TransformUtils::IDENTITY;

    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<ParticleForce>());
    ParticleForce* force = obj.Cast<ParticleForce>();

    ParticleEmitterInstance* emitterInstance = GetEmitterInstance(force);
    if (emitterInstance == nullptr)
        return currentMatrix;

    ParticleEffectComponent* ownerComponent = emitterInstance->GetOwner();
    if ((ownerComponent == nullptr) || (ownerComponent->GetEntity() == nullptr))
    {
        currentMatrix.SetTranslation(force->position);
    }
    else
    {
        TransformComponent* tc = ownerComponent->GetEntity()->GetComponent<TransformComponent>();
        if (force->worldAlign)
        {
            currentMatrix.SetTranslation(force->position + tc->GetWorldTransform().GetTranslation());
        }
        else
        {
            const Transform& entityTransform = tc->GetWorldTransform();
            Vector3 center = force->position;
            TransformPerserveLength(center, Matrix3(TransformUtils::ToMatrix(entityTransform)));
            currentMatrix.SetTranslation(center + entityTransform.GetTranslation());
        }
    }
    return currentMatrix;
}

const Transform& ParticleForceTransformProxy::GetLocalTransform(const Any& object)
{
    static Transform currentMatrix;
    currentMatrix = TransformUtils::IDENTITY;

    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<ParticleForce>());
    ParticleForce* force = obj.Cast<ParticleForce>();

    currentMatrix.SetTranslation(force->position);
    return currentMatrix;
}

void ParticleForceTransformProxy::SetLocalTransform(Any& object, const Transform& transform)
{
    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<ParticleForce>());
    ParticleForce* force = obj.Cast<ParticleForce>();

    force->position = transform.GetTranslation();
}

bool ParticleForceTransformProxy::SupportsTransformType(const Any& object, Selectable::TransformType type) const
{
    return (type == Selectable::TransformType::Disabled) || (type == Selectable::TransformType::Translation);
}

bool ParticleForceTransformProxy::TransformDependsFromObject(const Any& dependant, const Any& dependsOn) const
{
    DVASSERT(dependant != dependsOn, "[TransformDependsFromObject] One object provided to both parameters");

    Entity* asEntity = Selectable(dependsOn).AsEntity();
    if (asEntity == nullptr)
        return false;

    Selectable dependantObj(dependant);
    ParticleForce* forceObj = dependantObj.Cast<ParticleForce>();
    // check if force contained inside entity
    ParticleEffectComponent* component = asEntity->GetComponent<ParticleEffectComponent>();
    if (component != nullptr)
    {
        for (uint32 i = 0, e = component->GetEmittersCount(); i < e; ++i)
        {
            ParticleEmitter* emitter = component->GetEmitterInstance(i)->GetEmitter();
            for (ParticleLayer* layer : emitter->layers)
            {
                for (ParticleForce* force : layer->GetParticleForces())
                {
                    if (force == forceObj)
                        return true;
                }
            }
        }
    }

    // or it's children
    for (int32 i = 0, e = asEntity->GetChildrenCount(); i < e; ++i)
    {
        if (TransformDependsFromObject(dependant, asEntity->GetChild(i)))
            return true;
    }

    return true;
}

ParticleEmitterInstance* ParticleForceTransformProxy::GetEmitterInstance(ParticleForce* force) const
{
    DataContext* context = Deprecated::GetActiveContext();
    if (context == nullptr)
        return nullptr;
    EditorParticlesSystem* particleSystem = context->GetData<SceneData>()->GetScene()->GetSystem<EditorParticlesSystem>();
    ParticleLayer* layer = particleSystem->GetForceOwner(force);
    return particleSystem->GetRootEmitterLayerOwner(layer);
}
} // namespace DAVA
