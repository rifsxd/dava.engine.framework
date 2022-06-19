#include "REPlatform/Commands/ParticleEditorCommands.h"
#include "REPlatform/Deprecated/ParticlesEditorNodeNameHelper.h"

#include "REPlatform/Global/StringConstants.h"

#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>
#include <Math/Vector.h>
#include <Particles/ParticleEmitter.h>
#include <Particles/ParticleEmitterInstance.h>
#include <Particles/ParticleForceSimplified.h>
#include <Particles/ParticlePropertyLine.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Scene3D/Systems/ParticlesQualitySettings.h>
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
namespace ParticleEditorCommandsDetail
{
using ForceType = ParticleForce::eType;
Map<ForceType, String> forceNames =
{
  { ForceType::DRAG_FORCE, "Drag" },
  { ForceType::WIND, "Wind" },
  { ForceType::VORTEX, "Vortex" },
  { ForceType::GRAVITY, "Gravity" },
  { ForceType::POINT_GRAVITY, "Point Gravity" },
  { ForceType::PLANE_COLLISION, "Plane Collision" }
};

void AddNewForceToLayer(ParticleLayer* layer, ParticleForce::eType forceType)
{
    if (layer == nullptr)
        return;
    ScopedPtr<ParticleForce> newForce(new ParticleForce(layer));
    newForce->type = forceType;
    newForce->forceName = ParticleEditorCommandsDetail::forceNames[forceType];

    layer->AddForce(newForce);
}
}

CommandUpdateEffect::CommandUpdateEffect(ParticleEffectComponent* effect)
    : CommandAction()
    , particleEffect(effect)
{
}

void CommandUpdateEffect::Init(float32 speed)
{
    playbackSpeed = speed;
}

void CommandUpdateEffect::Redo()
{
    DVASSERT(particleEffect);
    particleEffect->SetPlaybackSpeed(playbackSpeed);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandUpdateEffect)
{
    ReflectionRegistrator<CommandUpdateEffect>::Begin()
    .End();
}

CommandUpdateEmitter::CommandUpdateEmitter(ParticleEmitterInstance* emitter_)
    : CommandAction()
    , instance(emitter_)
{
}

void CommandUpdateEmitter::Init(const FastName& name,
                                ParticleEmitter::eType emitterType,
                                RefPtr<PropertyLine<float32>> emissionRange,
                                RefPtr<PropertyLine<Vector3>> emissionVector,
                                RefPtr<PropertyLine<Vector3>> emissionVelocityVector,
                                RefPtr<PropertyLine<float32>> radius,
                                RefPtr<PropertyLine<float32>> innerRadius,
                                RefPtr<PropertyLine<float32>> emissionAngle,
                                RefPtr<PropertyLine<float32>> emissionAngleVariation,
                                RefPtr<PropertyLine<Color>> colorOverLife,
                                RefPtr<PropertyLine<Vector3>> size,
                                float32 life,
                                bool isShortEffect,
                                bool generateOnSurface,
                                DAVA::ParticleEmitter::eShockwaveMode shockwaveMode)
{
    this->name = name;
    this->emitterType = emitterType;
    this->emissionRange = emissionRange;
    this->emissionVector = emissionVector;
    this->emissionVelocityVector = emissionVelocityVector;
    this->radius = radius;
    this->innerRadius = innerRadius;
    this->emissionAngle = emissionAngle;
    this->emissionAngleVariation = emissionAngleVariation;
    this->colorOverLife = colorOverLife;
    this->size = size;
    this->life = life;
    this->isShortEffect = isShortEffect;
    this->generateOnSurface = generateOnSurface;
    this->shockwaveMode = shockwaveMode;
}

void CommandUpdateEmitter::Redo()
{
    DVASSERT(instance);
    auto emitter = instance->GetEmitter();
    emitter->name = name;
    emitter->emitterType = emitterType;
    PropertyLineHelper::SetValueLine(emitter->emissionRange, emissionRange);
    PropertyLineHelper::SetValueLine(emitter->emissionVector, emissionVector);
    PropertyLineHelper::SetValueLine(emitter->emissionVelocityVector, emissionVelocityVector);
    PropertyLineHelper::SetValueLine(emitter->radius, radius);
    PropertyLineHelper::SetValueLine(emitter->innerRadius, innerRadius);
    PropertyLineHelper::SetValueLine(emitter->colorOverLife, colorOverLife);
    PropertyLineHelper::SetValueLine(emitter->size, size);
    PropertyLineHelper::SetValueLine(emitter->emissionAngle, emissionAngle);
    PropertyLineHelper::SetValueLine(emitter->emissionAngleVariation, emissionAngleVariation);
    emitter->shortEffect = isShortEffect;
    emitter->generateOnSurface = generateOnSurface;
    emitter->shockwaveMode = shockwaveMode;
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandUpdateEmitter)
{
    ReflectionRegistrator<CommandUpdateEmitter>::Begin()
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandUpdateParticleLayerBase)
{
    ReflectionRegistrator<CommandUpdateParticleLayerBase>::Begin()
    .End();
}

CommandUpdateParticleLayer::CommandUpdateParticleLayer(ParticleEmitterInstance* emitter, ParticleLayer* layer)
    : CommandUpdateParticleLayerBase()
{
    this->emitter = emitter;
    this->layer = layer;
}

void CommandUpdateParticleLayer::Init(const String& layerName,
                                      ParticleLayer::eType layerType,
                                      ParticleLayer::eDegradeStrategy degradeStrategy,
                                      bool isDisabled,
                                      bool inheritPosition,
                                      bool isLong,
                                      float32 scaleVelocityBase,
                                      float32 scaleVelocityFactor,
                                      bool isLooped,
                                      int32 particleOrientation,
                                      RefPtr<PropertyLine<float32>> life,
                                      RefPtr<PropertyLine<float32>> lifeVariation,
                                      RefPtr<PropertyLine<float32>> number,
                                      RefPtr<PropertyLine<float32>> numberVariation,
                                      RefPtr<PropertyLine<Vector2>> size,
                                      RefPtr<PropertyLine<Vector2>> sizeVariation,
                                      RefPtr<PropertyLine<Vector2>> sizeOverLife,
                                      RefPtr<PropertyLine<float32>> velocity,
                                      RefPtr<PropertyLine<float32>> velocityVariation,
                                      RefPtr<PropertyLine<float32>> velocityOverLife,
                                      RefPtr<PropertyLine<float32>> spin,
                                      RefPtr<PropertyLine<float32>> spinVariation,
                                      RefPtr<PropertyLine<float32>> spinOverLife,
                                      bool randomSpinDirection,

                                      RefPtr<PropertyLine<Color>> colorRandom,
                                      RefPtr<PropertyLine<float32>> alphaOverLife,
                                      RefPtr<PropertyLine<Color>> colorOverLife,
                                      RefPtr<PropertyLine<float32>> angle,
                                      RefPtr<PropertyLine<float32>> angleVariation,

                                      float32 startTime,
                                      float32 endTime,
                                      float32 deltaTime,
                                      float32 deltaVariation,
                                      float32 loopEndTime,
                                      float32 loopVariation,
                                      bool frameOverLifeEnabled,
                                      float32 frameOverLifeFPS,
                                      bool randomFrameOnStart,
                                      bool loopSpriteAnimation,
                                      RefPtr<PropertyLine<float32>> animSpeedOverLife,

                                      float32 pivotPointX,
                                      float32 pivotPointY,
                                      bool applyGlobalForces)
{
    this->layerName = layerName;
    this->layerType = layerType;
    this->degradeStrategy = degradeStrategy;
    this->isDisabled = isDisabled;
    this->inheritPosition = inheritPosition;
    this->isLooped = isLooped;
    this->isLong = isLong;
    this->scaleVelocityBase = scaleVelocityBase;
    this->scaleVelocityFactor = scaleVelocityFactor;
    this->life = life;
    this->lifeVariation = lifeVariation;
    this->number = number;
    this->numberVariation = numberVariation;
    this->size = size;
    this->sizeVariation = sizeVariation;
    this->sizeOverLife = sizeOverLife;
    this->velocity = velocity;
    this->velocityVariation = velocityVariation;
    this->velocityOverLife = velocityOverLife;
    this->spin = spin;
    this->spinVariation = spinVariation;
    this->spinOverLife = spinOverLife;
    this->randomSpinDirection = randomSpinDirection;
    this->particleOrientation = particleOrientation;

    this->colorRandom = colorRandom;
    this->alphaOverLife = alphaOverLife;
    this->colorOverLife = colorOverLife;
    this->angle = angle;
    this->angleVariation = angleVariation;

    this->startTime = startTime;
    this->endTime = endTime;
    this->deltaTime = deltaTime;
    this->deltaVariation = deltaVariation;
    this->loopEndTime = loopEndTime;
    this->loopVariation = loopVariation;
    this->frameOverLifeEnabled = frameOverLifeEnabled;
    this->frameOverLifeFPS = frameOverLifeFPS;
    this->randomFrameOnStart = randomFrameOnStart;
    this->loopSpriteAnimation = loopSpriteAnimation;
    this->animSpeedOverLife = animSpeedOverLife;

    this->pivotPointX = pivotPointX;
    this->pivotPointY = pivotPointY;

    this->applyGlobalForces = applyGlobalForces;
}

void CommandUpdateParticleLayer::Redo()
{
    layer->layerName = layerName;
    layer->degradeStrategy = degradeStrategy;
    layer->isDisabled = isDisabled;
    layer->SetInheritPosition(inheritPosition);
    layer->isLong = isLong;
    layer->scaleVelocityBase = scaleVelocityBase;
    layer->scaleVelocityFactor = scaleVelocityFactor;
    layer->isLooped = isLooped;
    PropertyLineHelper::SetValueLine(layer->life, life);
    PropertyLineHelper::SetValueLine(layer->lifeVariation, lifeVariation);
    PropertyLineHelper::SetValueLine(layer->number, number);
    PropertyLineHelper::SetValueLine(layer->numberVariation, numberVariation);
    PropertyLineHelper::SetValueLine(layer->size, size);
    PropertyLineHelper::SetValueLine(layer->sizeVariation, sizeVariation);
    PropertyLineHelper::SetValueLine(layer->sizeOverLifeXY, sizeOverLife);
    PropertyLineHelper::SetValueLine(layer->velocity, velocity);
    PropertyLineHelper::SetValueLine(layer->velocityVariation, velocityVariation);
    PropertyLineHelper::SetValueLine(layer->velocityOverLife, velocityOverLife);
    PropertyLineHelper::SetValueLine(layer->spin, spin);
    PropertyLineHelper::SetValueLine(layer->spinVariation, spinVariation);
    PropertyLineHelper::SetValueLine(layer->spinOverLife, spinOverLife);
    layer->randomSpinDirection = randomSpinDirection;
    layer->particleOrientation = particleOrientation;

    PropertyLineHelper::SetValueLine(layer->colorRandom, colorRandom);
    PropertyLineHelper::SetValueLine(layer->alphaOverLife, alphaOverLife);
    PropertyLineHelper::SetValueLine(layer->colorOverLife, colorOverLife);

    layer->frameOverLifeEnabled = frameOverLifeEnabled;
    layer->frameOverLifeFPS = frameOverLifeFPS;
    layer->randomFrameOnStart = randomFrameOnStart;
    layer->loopSpriteAnimation = loopSpriteAnimation;
    PropertyLineHelper::SetValueLine(layer->animSpeedOverLife, animSpeedOverLife);

    PropertyLineHelper::SetValueLine(layer->angle, angle);
    PropertyLineHelper::SetValueLine(layer->angleVariation, angleVariation);

    layer->UpdateLayerTime(startTime, endTime);
    layer->deltaTime = deltaTime;
    layer->deltaVariation = deltaVariation;
    layer->loopEndTime = loopEndTime;
    layer->loopVariation = loopVariation;

    layer->applyGlobalForces = applyGlobalForces;

    layer->SetPivotPoint(Vector2(pivotPointX, pivotPointY));

    // The same is for emitter type.
    if (layer->type != layerType)
    {
        if (layer->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
        {
            deletedEmitter = RefPtr<ParticleEmitterInstance>::ConstructWithRetain(layer->innerEmitter);
            SafeRelease(layer->innerEmitter);
        }
        layer->type = layerType;
        if (layerType == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
        {
            DVASSERT(layer->innerEmitter == nullptr);
            ScopedPtr<ParticleEmitter> emitter(new ParticleEmitter());
            if (!layer->innerEmitterPath.IsEmpty())
            {
                emitter->LoadFromYaml(layer->innerEmitterPath);
            }

            layer->innerEmitter = new ParticleEmitterInstance(emitter);
            createdEmitter = RefPtr<ParticleEmitterInstance>::ConstructWithRetain(layer->innerEmitter);
        }
        //TODO: restart in effect
    }
    layer->isLong = isLong;
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandUpdateParticleLayer)
{
    ReflectionRegistrator<CommandUpdateParticleLayer>::Begin()
    .End();
}

CommandUpdateParticleLayerTime::CommandUpdateParticleLayerTime(ParticleLayer* layer)
    : CommandUpdateParticleLayerBase()
{
    this->layer = layer;
}

void CommandUpdateParticleLayerTime::Init(float32 startTime, float32 endTime)
{
    this->startTime = startTime;
    this->endTime = endTime;
}

void CommandUpdateParticleLayerTime::Redo()
{
    layer->UpdateLayerTime(startTime, endTime);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandUpdateParticleLayerTime)
{
    ReflectionRegistrator<CommandUpdateParticleLayerTime>::Begin()
    .End();
}

CommandUpdateParticleLayerEnabled::CommandUpdateParticleLayerEnabled(ParticleLayer* layer, bool isEnabled)
    : CommandUpdateParticleLayerBase()
{
    this->layer = layer;
    this->isEnabled = isEnabled;
}

void CommandUpdateParticleLayerEnabled::Redo()
{
    if (layer)
    {
        layer->isDisabled = !isEnabled;
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandUpdateParticleLayerEnabled)
{
    ReflectionRegistrator<CommandUpdateParticleLayerEnabled>::Begin()
    .End();
}

CommandUpdateParticleLayerLods::CommandUpdateParticleLayerLods(ParticleLayer* layer, const Vector<bool>& lods)
    : CommandUpdateParticleLayerBase()
{
    this->layer = layer;
    this->lods = lods;
}

void CommandUpdateParticleLayerLods::Redo()
{
    if (layer == nullptr)
        return;

    for (size_type i = 0; i < lods.size(); i++)
    {
        layer->SetLodActive(static_cast<int32>(i), lods[i]);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandUpdateParticleLayerLods)
{
    ReflectionRegistrator<CommandUpdateParticleLayerLods>::Begin()
    .End();
}

CommandUpdateParticleSimplifiedForce::CommandUpdateParticleSimplifiedForce(ParticleLayer* layer, uint32 forceId)
    : CommandAction()
{
    this->layer = layer;
    this->forceId = forceId;
}

void CommandUpdateParticleSimplifiedForce::Init(RefPtr<PropertyLine<Vector3>> force,
                                                RefPtr<PropertyLine<float32>> forcesOverLife)
{
    PropertyLineHelper::SetValueLine(this->force, force);
    PropertyLineHelper::SetValueLine(this->forcesOverLife, forcesOverLife);
}

void CommandUpdateParticleSimplifiedForce::Redo()
{
    layer->GetSimplifiedParticleForces()[forceId]->force = force;
    layer->GetSimplifiedParticleForces()[forceId]->forceOverLife = forcesOverLife;
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandUpdateParticleSimplifiedForce)
{
    ReflectionRegistrator<CommandUpdateParticleSimplifiedForce>::Begin()
    .End();
}

//////////////////////////////////////////////////////////////////////////
CommandUpdateParticleForce::CommandUpdateParticleForce(ParticleLayer* layer_, uint32 forceId_, ForceParams&& params)
    : CommandAction()
    , layer(layer_)
    , forceId(forceId_)
    , newParams(params)
{
    DVASSERT(forceId < static_cast<uint32>(layer->GetParticleForces().size()));
    DVASSERT(layer != nullptr);
    if (layer != nullptr)
    {
        ParticleForce* force = layer->GetParticleForces()[forceId];
        oldParams.isActive = force->isActive;
        oldParams.useInfinityRange = force->isInfinityRange;
        oldParams.killParticles = force->killParticles;
        oldParams.normalAsReflectionVector = force->normalAsReflectionVector;
        oldParams.randomizeReflectionForce = force->randomizeReflectionForce;
        oldParams.worldAlign = force->worldAlign;
        oldParams.pointGravityUseRandomPointsOnSphere = force->pointGravityUseRandomPointsOnSphere;
        oldParams.isGlobal = force->isGlobal;
        oldParams.boxSize = force->GetBoxSize();
        oldParams.radius = force->GetRadius();
        oldParams.forcePower = force->forcePower;
        oldParams.shape = force->GetShape();
        oldParams.forceName = force->forceName;
        oldParams.timingType = force->timingType;
        oldParams.forcePowerLine = force->forcePowerLine;
        oldParams.turbulenceLine = force->turbulenceLine;
        oldParams.direction = force->direction;
        oldParams.windFrequency = force->windFrequency;
        oldParams.windTurbulence = force->windTurbulence;
        oldParams.pointGravityRadius = force->pointGravityRadius;
        oldParams.rndReflectionForceMin = force->rndReflectionForceMin;
        oldParams.rndReflectionForceMax = force->rndReflectionForceMax;
        oldParams.planeScale = force->planeScale;
        oldParams.reflectionChaos = force->reflectionChaos;
        oldParams.windTurbulenceFrequency = force->windTurbulenceFrequency;
        oldParams.windBias = force->windBias;
        oldParams.backwardTurbulenceProbability = force->backwardTurbulenceProbability;
        oldParams.reflectionPercent = force->reflectionPercent;
        oldParams.velocityThreshold = force->velocityThreshold;
        oldParams.startTime = force->startTime;
        oldParams.endTime = force->endTime;
    }
}

void CommandUpdateParticleForce::Redo()
{
    ApplyParams(newParams);
}

void CommandUpdateParticleForce::Undo()
{
    ApplyParams(oldParams);
}

void CommandUpdateParticleForce::ApplyParams(ForceParams& params)
{
    if (layer != nullptr && forceId < layer->GetParticleForces().size())
    {
        ParticleForce* force = layer->GetParticleForces()[forceId];
        force->isActive = params.isActive;
        force->SetBoxSize(params.boxSize);
        force->SetRadius(params.radius);
        force->forcePower = params.forcePower;
        force->SetShape(params.shape);
        force->isInfinityRange = params.useInfinityRange;
        force->killParticles = params.killParticles;
        force->pointGravityUseRandomPointsOnSphere = params.pointGravityUseRandomPointsOnSphere;
        force->isGlobal = params.isGlobal;
        force->forceName = params.forceName;
        force->timingType = params.timingType;
        force->direction = params.direction;
        force->windFrequency = params.windFrequency;
        force->windTurbulence = params.windTurbulence;
        force->windTurbulenceFrequency = params.windTurbulenceFrequency;
        force->windBias = params.windBias;
        force->backwardTurbulenceProbability = params.backwardTurbulenceProbability;
        force->pointGravityRadius = params.pointGravityRadius;
        force->planeScale = params.planeScale;
        force->reflectionChaos = params.reflectionChaos;
        force->normalAsReflectionVector = params.normalAsReflectionVector;
        force->randomizeReflectionForce = params.randomizeReflectionForce;
        force->worldAlign = params.worldAlign;
        force->rndReflectionForceMin = params.rndReflectionForceMin;
        force->rndReflectionForceMax = params.rndReflectionForceMax;
        force->velocityThreshold = params.velocityThreshold;
        force->reflectionPercent = params.reflectionPercent;
        force->startTime = params.startTime;
        force->endTime = params.endTime;
        PropertyLineHelper::SetValueLine(force->forcePowerLine, params.forcePowerLine);
        PropertyLineHelper::SetValueLine(force->turbulenceLine, params.turbulenceLine);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandUpdateParticleForce)
{
    ReflectionRegistrator<CommandUpdateParticleForce>::Begin()
    .End();
}

CommandAddParticleEmitter::CommandAddParticleEmitter(Entity* effect)
    : CommandAction()
    , effectEntity(effect)
{
}

void CommandAddParticleEmitter::Redo()
{
    if (effectEntity == nullptr)
        return;

    ParticleEffectComponent* effectComponent = GetEffectComponent(effectEntity);
    DVASSERT(effectComponent);
    effectComponent->AddEmitterInstance(ScopedPtr<ParticleEmitter>(new ParticleEmitter()));
}

Entity* CommandAddParticleEmitter::GetEntity() const
{
    return effectEntity;
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandAddParticleEmitter)
{
    ReflectionRegistrator<CommandAddParticleEmitter>::Begin()
    .End();
}

CommandStartStopParticleEffect::CommandStartStopParticleEffect(Entity* effect, bool isStart)
    : CommandAction()
{
    this->isStart = isStart;
    this->effectEntity = effect;
}

void CommandStartStopParticleEffect::Redo()
{
    if (effectEntity == nullptr)
        return;

    ParticleEffectComponent* effectComponent = effectEntity->GetComponent<ParticleEffectComponent>();
    DVASSERT(effectComponent);

    if (this->isStart)
    {
        effectComponent->Start();
    }
    else
    {
        effectComponent->Stop();
    }
}

Entity* CommandStartStopParticleEffect::GetEntity() const
{
    return this->effectEntity;
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandStartStopParticleEffect)
{
    ReflectionRegistrator<CommandStartStopParticleEffect>::Begin()
    .End();
}

CommandRestartParticleEffect::CommandRestartParticleEffect(Entity* effect)
    : CommandAction()
{
    this->effectEntity = effect;
}

void CommandRestartParticleEffect::Redo()
{
    if (!effectEntity)
    {
        return;
    }

    ParticleEffectComponent* effectComponent = effectEntity->GetComponent<ParticleEffectComponent>();
    DVASSERT(effectComponent);
    effectComponent->Restart();
}

Entity* CommandRestartParticleEffect::GetEntity() const
{
    return this->effectEntity;
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandRestartParticleEffect)
{
    ReflectionRegistrator<CommandRestartParticleEffect>::Begin()
    .End();
}

CommandAddParticleEmitterLayer::CommandAddParticleEmitterLayer(ParticleEffectComponent* component_, ParticleEmitterInstance* emitter)
    : CommandAction()
    , component(component_)
    , instance(emitter)
{
}

CommandAddParticleEmitterLayer::~CommandAddParticleEmitterLayer()
{
    SafeRelease(createdLayer);
}

void CommandAddParticleEmitterLayer::Redo()
{
    if (instance == nullptr)
        return;

    static const float32 LIFETIME_FOR_NEW_PARTICLE_EMITTER = 4.0f;

    createdLayer = new ParticleLayer();
    createdLayer->startTime = 0;
    createdLayer->endTime = LIFETIME_FOR_NEW_PARTICLE_EMITTER;
    createdLayer->life = new PropertyLineValue<float32>(1.0f);
    createdLayer->layerName = ParticlesEditorNodeNameHelper::GetNewLayerName(ResourceEditor::LAYER_NODE_NAME.c_str(), instance->GetEmitter());

    createdLayer->loopEndTime = instance->GetEmitter()->lifeTime;
    instance->GetEmitter()->AddLayer(createdLayer);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandAddParticleEmitterLayer)
{
    ReflectionRegistrator<CommandAddParticleEmitterLayer>::Begin()
    .End();
}

CommandRemoveParticleEmitterLayer::CommandRemoveParticleEmitterLayer(ParticleEffectComponent* component_, ParticleEmitterInstance* emitter, ParticleLayer* layer)
    : RECommand("Remove Particle Layer")
    , component(component_)
    , instance(emitter)
    , selectedLayer(SafeRetain(layer))
{
    DVASSERT(instance != nullptr);
    DVASSERT(selectedLayer != nullptr);
}

void CommandRemoveParticleEmitterLayer::Redo()
{
    selectedLayerIndex = instance->GetEmitter()->RemoveLayer(selectedLayer);
}

void CommandRemoveParticleEmitterLayer::Undo()
{
    DVASSERT(selectedLayerIndex != -1);
    instance->GetEmitter()->InsertLayer(selectedLayer, selectedLayerIndex);
}

CommandRemoveParticleEmitterLayer::~CommandRemoveParticleEmitterLayer()
{
    SafeRelease(selectedLayer);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandRemoveParticleEmitterLayer)
{
    ReflectionRegistrator<CommandRemoveParticleEmitterLayer>::Begin()
    .End();
}

CommandRemoveParticleEmitter::CommandRemoveParticleEmitter(ParticleEffectComponent* effect, ParticleEmitterInstance* emitter)
    : RECommand()
    , selectedEffect(effect)
    , instance(SafeRetain(emitter))
{
    DVASSERT(selectedEffect != nullptr);
    DVASSERT(instance != nullptr);
}

CommandRemoveParticleEmitter::~CommandRemoveParticleEmitter()
{
    SafeRelease(instance);
}

void CommandRemoveParticleEmitter::Redo()
{
    selectedEffect->RemoveEmitterInstance(instance);
}

void CommandRemoveParticleEmitter::Undo()
{
    selectedEffect->AddEmitterInstance(instance);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandRemoveParticleEmitter)
{
    ReflectionRegistrator<CommandRemoveParticleEmitter>::Begin()
    .End();
}

CommandCloneParticleEmitterLayer::CommandCloneParticleEmitterLayer(ParticleEmitterInstance* emitter, ParticleLayer* layer)
    : CommandAction()
    , instance(emitter)
    , selectedLayer(layer)
{
}

void CommandCloneParticleEmitterLayer::Redo()
{
    if ((selectedLayer == nullptr) || (instance == nullptr))
        return;

    ScopedPtr<ParticleLayer> clonedLayer(selectedLayer->Clone());
    clonedLayer->layerName = selectedLayer->layerName + " Clone";
    instance->GetEmitter()->AddLayer(clonedLayer);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandCloneParticleEmitterLayer)
{
    ReflectionRegistrator<CommandCloneParticleEmitterLayer>::Begin()
    .End();
}

CommandAddParticleEmitterSimplifiedForce::CommandAddParticleEmitterSimplifiedForce(ParticleEffectComponent* component_, ParticleLayer* layer)
    : CommandAction()
    , component(component_)
    , selectedLayer(layer)
{
}

void CommandAddParticleEmitterSimplifiedForce::Redo()
{
    if (selectedLayer == nullptr)
        return;

    // Add the new Force to the Layer.
    ParticleForceSimplified* newForce = new ParticleForceSimplified(RefPtr<PropertyLine<Vector3>>(new PropertyLineValue<Vector3>(Vector3(0, 0, 0))), RefPtr<PropertyLine<float32>>());
    selectedLayer->AddSimplifiedForce(newForce);
    newForce->Release();
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandAddParticleEmitterSimplifiedForce)
{
    ReflectionRegistrator<CommandAddParticleEmitterSimplifiedForce>::Begin()
    .End();
}

CommandRemoveParticleEmitterSimplifiedForce::CommandRemoveParticleEmitterSimplifiedForce(ParticleEffectComponent* component_, ParticleLayer* layer, ParticleForceSimplified* force)
    : RECommand("Remove force")
    , component(component_)
    , selectedLayer(layer)
    , selectedForce(SafeRetain(force))
{
}

CommandRemoveParticleEmitterSimplifiedForce::~CommandRemoveParticleEmitterSimplifiedForce()
{
    SafeRelease(selectedForce);
}

void CommandRemoveParticleEmitterSimplifiedForce::Redo()
{
    if ((selectedLayer == nullptr) || (selectedForce == nullptr))
        return;

    selectedLayer->RemoveSimplifiedForce(selectedForce);
}

void CommandRemoveParticleEmitterSimplifiedForce::Undo()
{
    if ((selectedLayer == nullptr) || (selectedForce == nullptr))
        return;

    selectedLayer->AddSimplifiedForce(selectedForce);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandRemoveParticleEmitterSimplifiedForce)
{
    ReflectionRegistrator<CommandRemoveParticleEmitterSimplifiedForce>::Begin()
    .End();
}

CommandAddParticleDrag::CommandAddParticleDrag(ParticleEffectComponent* component_, ParticleLayer* layer)
    : CommandAction()
    , component(component_)
    , selectedLayer(layer)
{
}

void CommandAddParticleDrag::Redo()
{
    ParticleEditorCommandsDetail::AddNewForceToLayer(selectedLayer, ParticleForce::eType::DRAG_FORCE);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandAddParticleDrag)
{
    ReflectionRegistrator<CommandAddParticleDrag>::Begin()
    .End();
}

CommandAddParticleVortex::CommandAddParticleVortex(ParticleEffectComponent* component_, ParticleLayer* layer)
    : CommandAction()
    , component(component_)
    , selectedLayer(layer)
{
}

void CommandAddParticleVortex::Redo()
{
    ParticleEditorCommandsDetail::AddNewForceToLayer(selectedLayer, ParticleForce::eType::VORTEX);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandAddParticleVortex)
{
    ReflectionRegistrator<CommandAddParticleVortex>::Begin()
    .End();
}

CommandAddParticleGravity::CommandAddParticleGravity(ParticleEffectComponent* component_, ParticleLayer* layer)
    : CommandAction()
    , component(component_)
    , selectedLayer(layer)
{
}

void CommandAddParticleGravity::Redo()
{
    ParticleEditorCommandsDetail::AddNewForceToLayer(selectedLayer, ParticleForce::eType::GRAVITY);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandAddParticleGravity)
{
    ReflectionRegistrator<CommandAddParticleGravity>::Begin()
    .End();
}

CommandAddParticleWind::CommandAddParticleWind(ParticleEffectComponent* component_, ParticleLayer* layer)
    : CommandAction()
    , component(component_)
    , selectedLayer(layer)
{
}

void CommandAddParticleWind::Redo()
{
    ParticleEditorCommandsDetail::AddNewForceToLayer(selectedLayer, ParticleForce::eType::WIND);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandAddParticleWind)
{
    ReflectionRegistrator<CommandAddParticleWind>::Begin()
    .End();
}

CommandAddParticlePointGravity::CommandAddParticlePointGravity(ParticleEffectComponent* component_, ParticleLayer* layer)
    : CommandAction()
    , component(component_)
    , selectedLayer(layer)
{
}

void CommandAddParticlePointGravity::Redo()
{
    ParticleEditorCommandsDetail::AddNewForceToLayer(selectedLayer, ParticleForce::eType::POINT_GRAVITY);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandAddParticlePointGravity)
{
    ReflectionRegistrator<CommandAddParticlePointGravity>::Begin()
    .End();
}

CommandAddParticlePlaneCollision::CommandAddParticlePlaneCollision(ParticleEffectComponent* component_, ParticleLayer* layer)
    : CommandAction()
    , component(component_)
    , selectedLayer(layer)
{
}

void CommandAddParticlePlaneCollision::Redo()
{
    ParticleEditorCommandsDetail::AddNewForceToLayer(selectedLayer, ParticleForce::eType::PLANE_COLLISION);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandAddParticlePlaneCollision)
{
    ReflectionRegistrator<CommandAddParticlePlaneCollision>::Begin()
    .End();
}

CommandRemoveParticleForce::CommandRemoveParticleForce(ParticleEffectComponent* component_, ParticleLayer* layer, ParticleForce* force)
    : RECommand()
    , component(component_)
    , selectedLayer(layer)
    , selectedForce(SafeRetain(force))
{
    DVASSERT(selectedLayer != nullptr);
    DVASSERT(selectedForce != nullptr);
}

void CommandRemoveParticleForce::Redo()
{
    selectedLayer->RemoveForce(selectedForce);
}

void CommandRemoveParticleForce::Undo()
{
    selectedLayer->AddForce(selectedForce);
}

CommandRemoveParticleForce::~CommandRemoveParticleForce()
{
    SafeRelease(selectedForce);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandRemoveParticleForce)
{
    ReflectionRegistrator<CommandRemoveParticleForce>::Begin()
    .End();
}

CommandLoadParticleEmitterFromYaml::CommandLoadParticleEmitterFromYaml(ParticleEffectComponent* effect, ParticleEmitterInstance* emitter, const FilePath& path)
    : CommandAction()
    , selectedEffect(effect)
    , instance(emitter)
    , filePath(path)
{
}

void CommandLoadParticleEmitterFromYaml::Redo()
{
    if ((instance == nullptr) || (selectedEffect == nullptr))
        return;

    auto emitterIndex = selectedEffect->GetEmitterInstanceIndex(instance);
    if (emitterIndex == -1)
        return;

    //TODO: restart effect
    const ParticlesQualitySettings::FilepathSelector* filepathSelector = QualitySettingsSystem::Instance()->GetParticlesQualitySettings().GetOrCreateFilepathSelector();
    FilePath qualityFilepath = filePath;
    if (filepathSelector)
    {
        qualityFilepath = filepathSelector->SelectFilepath(filePath);
    }
    instance->GetEmitter()->LoadFromYaml(qualityFilepath);
    selectedEffect->SetOriginalConfigPath(emitterIndex, filePath);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandLoadParticleEmitterFromYaml)
{
    ReflectionRegistrator<CommandLoadParticleEmitterFromYaml>::Begin()
    .End();
}

CommandSaveParticleEmitterToYaml::CommandSaveParticleEmitterToYaml(ParticleEffectComponent* effect, ParticleEmitterInstance* emitter, const FilePath& path)
    : CommandAction()
    , selectedEffect(effect)
    , instance(emitter)
    , filePath(path)
{
}

void CommandSaveParticleEmitterToYaml::Redo()
{
    if ((instance == nullptr) || (selectedEffect == nullptr))
        return;

    if (selectedEffect->GetEmitterInstanceIndex(instance) != -1)
    {
        instance->GetEmitter()->SaveToYaml(filePath);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandSaveParticleEmitterToYaml)
{
    ReflectionRegistrator<CommandSaveParticleEmitterToYaml>::Begin()
    .End();
}

CommandLoadInnerParticleEmitterFromYaml::CommandLoadInnerParticleEmitterFromYaml(ParticleEmitterInstance* emitter, const FilePath& path)
    : CommandAction()
    , instance(emitter)
    , filePath(path)
{
}

void CommandLoadInnerParticleEmitterFromYaml::Redo()
{
    if (instance == nullptr)
        return;

    //TODO: restart effect
    instance->GetEmitter()->LoadFromYaml(filePath);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandLoadInnerParticleEmitterFromYaml)
{
    ReflectionRegistrator<CommandLoadInnerParticleEmitterFromYaml>::Begin()
    .End();
}

CommandSaveInnerParticleEmitterToYaml::CommandSaveInnerParticleEmitterToYaml(ParticleEmitterInstance* emitter, const FilePath& path)
    : CommandAction()
    , instance(emitter)
    , filePath(path)
{
}

void CommandSaveInnerParticleEmitterToYaml::Redo()
{
    if (instance == nullptr)
        return;

    instance->GetEmitter()->SaveToYaml(filePath);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandSaveInnerParticleEmitterToYaml)
{
    ReflectionRegistrator<CommandSaveInnerParticleEmitterToYaml>::Begin()
    .End();
}

CommandCloneParticleForce::CommandCloneParticleForce(ParticleEffectComponent* component_, ParticleLayer* layer, ParticleForce* force)
    : CommandAction()
    , component(component_)
    , selectedLayer(layer)
    , selectedForce(force)
{
}

void CommandCloneParticleForce::Redo()
{
    if ((selectedLayer == nullptr) || (selectedForce == nullptr))
        return;

    ScopedPtr<ParticleForce> clonedForce(selectedForce->Clone());
    clonedForce->forceName = selectedForce->forceName + " Clone";
    selectedLayer->AddForce(clonedForce);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandCloneParticleForce)
{
    ReflectionRegistrator<CommandCloneParticleForce>::Begin()
    .End();
}

CommandReloadEmitters::CommandReloadEmitters(ParticleEffectComponent* component_)
    : RECommand()
    , component(component_)
{
    int32 emitterCount = component->GetEmittersCount();

    for (int32 i = 0; i < emitterCount; i++)
    {
        ParticleEmitterInstance* instance = component->GetEmitterInstance(i);
        instance->Retain();
        undoParticleEmitterInstance.push_back(instance);
    }
}

CommandReloadEmitters::~CommandReloadEmitters()
{
    auto fRelease = [](auto& array)
    {
        for (DAVA::BaseObject* obj : array)
        {
            obj->Release();
        }
        array.clear();
    };

    fRelease(undoParticleEmitterInstance);
    fRelease(redoParticleEmitterInstance);
}

void CommandReloadEmitters::Redo()
{
    if (undoDataInitialized)
    {
        ReplaceComponentEmitters(redoParticleEmitterInstance);
    }
    else
    {
        int32 emitterCount = component->GetEmittersCount();

        for (int32 i = 0; i < emitterCount; i++)
        {
            ParticleEmitterInstance* undoInstance = component->GetEmitterInstance(i);
            ParticleEmitterInstance* redoInstance = undoInstance->Clone();

            redoParticleEmitterInstance.push_back(redoInstance);
        }

        ReplaceComponentEmitters(redoParticleEmitterInstance);

        component->ReloadEmitters();

        undoDataInitialized = true;
    }
}

void CommandReloadEmitters::Undo()
{
    ReplaceComponentEmitters(undoParticleEmitterInstance);
}

void CommandReloadEmitters::ReplaceComponentEmitters(const DAVA::Vector<DAVA::ParticleEmitterInstance*>& nextParticleEmitterInstances)
{
    int32 emitterCount = component->GetEmittersCount();

    for (int32 i = 0; i < emitterCount; i++)
    {
        ParticleEmitterInstance* instance = component->GetEmitterInstance(0);
        component->RemoveEmitterInstance(instance);
    }

    for (DAVA::ParticleEmitterInstance* instance : nextParticleEmitterInstances)
    {
        component->AddEmitterInstance(instance);
    }
}

ParticleEffectComponent* CommandReloadEmitters::GetComponent() const
{
    return component;
}

DAVA_VIRTUAL_REFLECTION_IMPL(CommandReloadEmitters)
{
    ReflectionRegistrator<CommandReloadEmitters>::Begin()
    .End();
}

} // namespace DAVA
