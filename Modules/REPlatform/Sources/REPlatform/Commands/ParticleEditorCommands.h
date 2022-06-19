#pragma once

#include "REPlatform/Commands/CommandAction.h"

#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>
#include <FileSystem/FilePath.h>
#include <Math/Vector.h>
#include <Particles/ParticleEmitter.h>
#include <Particles/ParticleLayer.h>
#include <Particles/ParticlePropertyLine.h>
#include <Particles/ParticleForce.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
class ParticleEmitterInstance;
class ParticleEffectComponent;
class ParticleForce;

class CommandAddParticleEmitter : public CommandAction
{
public:
    CommandAddParticleEmitter(Entity* effect);
    void Redo() override;

    Entity* GetEntity() const;

protected:
    Entity* effectEntity = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandAddParticleEmitter, CommandAction);
};

// Start/stop/restart Particle Effect.
class CommandStartStopParticleEffect : public CommandAction
{
public:
    CommandStartStopParticleEffect(Entity* effect, bool isStart);

    Entity* GetEntity() const;
    void Redo() override;

    bool GetStarted() const
    {
        return isStart;
    };

    bool IsClean() const override
    {
        return true;
    }

protected:
    Entity* effectEntity;
    bool isStart = false;

    DAVA_VIRTUAL_REFLECTION(CommandStartStopParticleEffect, CommandAction);
};

class CommandRestartParticleEffect : public CommandAction
{
public:
    CommandRestartParticleEffect(Entity* effect);

    Entity* GetEntity() const;
    void Redo() override;

    bool IsClean() const override
    {
        return true;
    }

protected:
    Entity* effectEntity = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandRestartParticleEffect, CommandAction);
};

// Add new layer to Particle Emitter.
class CommandAddParticleEmitterLayer : public CommandAction
{
public:
    CommandAddParticleEmitterLayer(ParticleEffectComponent* component, ParticleEmitterInstance* emitter);
    ~CommandAddParticleEmitterLayer();

    void Redo() override;

    ParticleLayer* GetCreatedLayer() const
    {
        return createdLayer;
    }

    ParticleEmitterInstance* GetParentEmitter() const
    {
        return instance;
    }

    ParticleEffectComponent* GetEffectComponent() const
    {
        return component;
    }

protected:
    ParticleEffectComponent* component = nullptr;
    ParticleEmitterInstance* instance = nullptr;
    ParticleLayer* createdLayer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandAddParticleEmitterLayer, CommandAction);
};

// Remove a layer from Particle Emitter.
class CommandRemoveParticleEmitterLayer : public RECommand
{
public:
    CommandRemoveParticleEmitterLayer(ParticleEffectComponent* component, ParticleEmitterInstance* emitter, ParticleLayer* layer);
    ~CommandRemoveParticleEmitterLayer();

    void Redo() override;
    void Undo() override;

    ParticleEffectComponent* GetEffectComponent() const
    {
        return component;
    }

    ParticleEmitterInstance* GetEmitterInstance() const
    {
        return instance;
    }

    ParticleLayer* GetLayer() const
    {
        return selectedLayer;
    }

protected:
    ParticleEffectComponent* component = nullptr;
    ParticleEmitterInstance* instance = nullptr;
    ParticleLayer* selectedLayer = nullptr;
    int32 selectedLayerIndex = -1;

    DAVA_VIRTUAL_REFLECTION(CommandRemoveParticleEmitterLayer, RECommand);
};

class CommandRemoveParticleEmitter : public RECommand
{
public:
    CommandRemoveParticleEmitter(ParticleEffectComponent* effect, ParticleEmitterInstance* emitter);
    ~CommandRemoveParticleEmitter();

    void Redo() override;
    void Undo() override;

    ParticleEffectComponent* GetEffect() const;
    ParticleEmitterInstance* GetEmitterInstance() const
    {
        return instance;
    }

protected:
    ParticleEffectComponent* selectedEffect = nullptr;
    ParticleEmitterInstance* instance = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandRemoveParticleEmitter, RECommand);
};

inline ParticleEffectComponent* CommandRemoveParticleEmitter::GetEffect() const
{
    return selectedEffect;
}

// Clone a layer inside Particle Emitter.
class CommandCloneParticleEmitterLayer : public CommandAction
{
public:
    CommandCloneParticleEmitterLayer(ParticleEmitterInstance* emitter, ParticleLayer* layer);
    void Redo() override;

    ParticleEmitterInstance* GetEmitterInstance() const
    {
        return instance;
    }

protected:
    ParticleEmitterInstance* instance = nullptr;
    ParticleLayer* selectedLayer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandCloneParticleEmitterLayer, CommandAction);
};

// Add new force to Particle Emitter layer.
class CommandAddParticleEmitterSimplifiedForce : public CommandAction
{
public:
    CommandAddParticleEmitterSimplifiedForce(ParticleEffectComponent* component, ParticleLayer* layer);
    void Redo() override;

    ParticleEffectComponent* GetEffectComponent() const
    {
        return component;
    }

    ParticleLayer* GetLayer() const
    {
        return selectedLayer;
    }

protected:
    ParticleEffectComponent* component = nullptr;
    ParticleLayer* selectedLayer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandAddParticleEmitterSimplifiedForce, CommandAction);
};

// Remove a force from Particle Emitter layer.
class CommandRemoveParticleEmitterSimplifiedForce : public RECommand
{
public:
    CommandRemoveParticleEmitterSimplifiedForce(ParticleEffectComponent* component, ParticleLayer* layer, ParticleForceSimplified* force);
    ~CommandRemoveParticleEmitterSimplifiedForce();
    void Redo() override;
    void Undo() override;

    ParticleEffectComponent* GetEffectComponent() const
    {
        return component;
    }

    ParticleLayer* GetLayer() const
    {
        return selectedLayer;
    }

    ParticleForceSimplified* GetForce() const
    {
        return selectedForce;
    }

protected:
    ParticleEffectComponent* component = nullptr;
    ParticleLayer* selectedLayer = nullptr;
    ParticleForceSimplified* selectedForce = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandRemoveParticleEmitterSimplifiedForce, RECommand);
};

class CommandAddParticleDrag : public CommandAction
{
public:
    CommandAddParticleDrag(ParticleEffectComponent* component, ParticleLayer* layer);
    void Redo() override;

    ParticleEffectComponent* GetEffectComponent() const
    {
        return component;
    }

    ParticleLayer* GetLayer() const
    {
        return selectedLayer;
    }

protected:
    ParticleEffectComponent* component = nullptr;
    ParticleLayer* selectedLayer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandAddParticleDrag, CommandAction);
};

class CommandAddParticleVortex : public CommandAction
{
public:
    CommandAddParticleVortex(ParticleEffectComponent* component, ParticleLayer* layer);
    void Redo() override;

    ParticleEffectComponent* GetEffectComponent() const
    {
        return component;
    }

    ParticleLayer* GetLayer() const
    {
        return selectedLayer;
    }

protected:
    ParticleEffectComponent* component = nullptr;
    ParticleLayer* selectedLayer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandAddParticleVortex, CommandAction);
};

class CommandAddParticleGravity : public CommandAction
{
public:
    CommandAddParticleGravity(ParticleEffectComponent* component, ParticleLayer* layer);
    void Redo() override;

    ParticleEffectComponent* GetEffectComponent() const
    {
        return component;
    }

    ParticleLayer* GetLayer() const
    {
        return selectedLayer;
    }

protected:
    ParticleEffectComponent* component = nullptr;
    ParticleLayer* selectedLayer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandAddParticleGravity, CommandAction);
};

class CommandAddParticleWind : public CommandAction
{
public:
    CommandAddParticleWind(ParticleEffectComponent* component, ParticleLayer* layer);
    void Redo() override;

    ParticleEffectComponent* GetEffectComponent() const
    {
        return component;
    }

    ParticleLayer* GetLayer() const
    {
        return selectedLayer;
    }

protected:
    ParticleEffectComponent* component = nullptr;
    ParticleLayer* selectedLayer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandAddParticleWind, CommandAction);
};

class CommandAddParticlePointGravity : public CommandAction
{
public:
    CommandAddParticlePointGravity(ParticleEffectComponent* component, ParticleLayer* layer);
    void Redo() override;

    ParticleEffectComponent* GetEffectComponent() const
    {
        return component;
    }

    ParticleLayer* GetLayer() const
    {
        return selectedLayer;
    }

protected:
    ParticleEffectComponent* component = nullptr;
    ParticleLayer* selectedLayer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandAddParticlePointGravity, CommandAction);
};

class CommandAddParticlePlaneCollision : public CommandAction
{
public:
    CommandAddParticlePlaneCollision(ParticleEffectComponent* component, ParticleLayer* layer);
    void Redo() override;

    ParticleEffectComponent* GetEffectComponent() const
    {
        return component;
    }

    ParticleLayer* GetLayer() const
    {
        return selectedLayer;
    }

protected:
    ParticleEffectComponent* component = nullptr;
    ParticleLayer* selectedLayer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandAddParticlePlaneCollision, CommandAction);
};

class CommandRemoveParticleForce : public RECommand
{
public:
    CommandRemoveParticleForce(ParticleEffectComponent* component, ParticleLayer* layer, ParticleForce* force);
    ~CommandRemoveParticleForce();

    void Redo() override;
    void Undo() override;

    ParticleEffectComponent* GetEffectComponent() const
    {
        return component;
    }

    ParticleLayer* GetLayer() const
    {
        return selectedLayer;
    }

    ParticleForce* GetForce() const
    {
        return selectedForce;
    }

protected:
    ParticleEffectComponent* component = nullptr;
    ParticleLayer* selectedLayer = nullptr;
    ParticleForce* selectedForce = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandRemoveParticleForce, RECommand);
};

class CommandCloneParticleForce : public CommandAction
{
public:
    CommandCloneParticleForce(ParticleEffectComponent* component, ParticleLayer* layer, ParticleForce* force);
    void Redo() override;

    ParticleEffectComponent* GetEffectComponent() const
    {
        return component;
    }

    ParticleLayer* GetLayer() const
    {
        return selectedLayer;
    }

    ParticleForce* GetForce() const
    {
        return selectedForce;
    }

protected:
    ParticleEffectComponent* component = nullptr;
    ParticleLayer* selectedLayer = nullptr;
    ParticleForce* selectedForce = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandCloneParticleForce, CommandAction);
};

class CommandUpdateEffect : public CommandAction
{
public:
    CommandUpdateEffect(ParticleEffectComponent* particleEffect);
    void Init(float32 playbackSpeed);
    void Redo() override;

protected:
    ParticleEffectComponent* particleEffect = nullptr;
    float32 playbackSpeed = 1.0f;

    DAVA_VIRTUAL_REFLECTION(CommandUpdateEffect, CommandAction);
};

class CommandUpdateEmitter : public CommandAction
{
public:
    CommandUpdateEmitter(ParticleEmitterInstance* emitter);

    void Init(const FastName& name,
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
              DAVA::ParticleEmitter::eShockwaveMode shockwaveMode);

    ParticleEmitterInstance* GetEmitterInstance() const
    {
        return instance;
    }

    void Redo() override;

protected:
    FastName name;
    ParticleEmitterInstance* instance = nullptr;

    ParticleEmitter::eType emitterType;
    RefPtr<PropertyLine<float32>> emissionRange;
    RefPtr<PropertyLine<float32>> emissionAngle;
    RefPtr<PropertyLine<float32>> emissionAngleVariation;
    RefPtr<PropertyLine<Vector3>> emissionVector;
    RefPtr<PropertyLine<Vector3>> emissionVelocityVector;
    RefPtr<PropertyLine<float32>> radius;
    RefPtr<PropertyLine<float32>> innerRadius;
    RefPtr<PropertyLine<Color>> colorOverLife;
    RefPtr<PropertyLine<Vector3>> size;
    float32 life = 0.0f;
    bool isShortEffect = false;
    bool generateOnSurface = false;
    DAVA::ParticleEmitter::eShockwaveMode shockwaveMode = DAVA::ParticleEmitter::eShockwaveMode::SHOCKWAVE_DISABLED;

    DAVA_VIRTUAL_REFLECTION(CommandUpdateEmitter, CommandAction);
};

class CommandUpdateParticleLayerBase : public CommandAction
{
public:
    CommandUpdateParticleLayerBase()
        : CommandAction()
    {
    }

    ParticleLayer* GetLayer() const
    {
        return layer;
    };

protected:
    ParticleLayer* layer;

    DAVA_VIRTUAL_REFLECTION(CommandUpdateParticleLayerBase, CommandAction);
};

class CommandUpdateParticleLayer : public CommandUpdateParticleLayerBase
{
public:
    CommandUpdateParticleLayer(ParticleEmitterInstance* emitter, ParticleLayer* layer);
    void Init(const String& layerName,
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
              bool applyGlobalForces);

    void Redo() override;

    ParticleEmitterInstance* GetDeletedEmitter() const
    {
        return deletedEmitter.Get();
    }

    ParticleEmitterInstance* GetCreatedEmitter() const
    {
        return createdEmitter.Get();
    }

protected:
    ParticleEmitterInstance* emitter = nullptr;

    String layerName;
    ParticleLayer::eType layerType;
    ParticleLayer::eDegradeStrategy degradeStrategy;
    bool isDisabled;
    bool isLong;
    float32 scaleVelocityBase;
    float32 scaleVelocityFactor;
    bool inheritPosition;
    bool isLooped;
    int32 particleOrientation;
    RefPtr<PropertyLine<float32>> life;
    RefPtr<PropertyLine<float32>> lifeVariation;
    RefPtr<PropertyLine<float32>> number;
    RefPtr<PropertyLine<float32>> numberVariation;
    RefPtr<PropertyLine<Vector2>> size;
    RefPtr<PropertyLine<Vector2>> sizeVariation;
    RefPtr<PropertyLine<Vector2>> sizeOverLife;
    RefPtr<PropertyLine<float32>> velocity;
    RefPtr<PropertyLine<float32>> velocityVariation;
    RefPtr<PropertyLine<float32>> velocityOverLife;
    RefPtr<PropertyLine<float32>> spin;
    RefPtr<PropertyLine<float32>> spinVariation;
    RefPtr<PropertyLine<float32>> spinOverLife;
    bool randomSpinDirection;

    RefPtr<PropertyLine<Color>> colorRandom;
    RefPtr<PropertyLine<float32>> alphaOverLife;
    RefPtr<PropertyLine<Color>> colorOverLife;
    RefPtr<PropertyLine<float32>> angle;
    RefPtr<PropertyLine<float32>> angleVariation;

    float32 startTime;
    float32 endTime;
    float32 deltaTime;
    float32 deltaVariation;
    float32 loopEndTime;
    float32 loopVariation;
    bool frameOverLifeEnabled;
    float32 frameOverLifeFPS;
    bool randomFrameOnStart;
    bool loopSpriteAnimation;
    RefPtr<PropertyLine<float32>> animSpeedOverLife;

    float32 pivotPointX;
    float32 pivotPointY;

    RefPtr<ParticleEmitterInstance> deletedEmitter;
    RefPtr<ParticleEmitterInstance> createdEmitter;
    bool applyGlobalForces;

    DAVA_VIRTUAL_REFLECTION(CommandUpdateParticleLayer, CommandUpdateParticleLayerBase);
};

class CommandUpdateParticleLayerTime : public CommandUpdateParticleLayerBase
{
public:
    CommandUpdateParticleLayerTime(ParticleLayer* layer);
    void Init(float32 startTime, float32 endTime);

    void Redo() override;

protected:
    float32 startTime;
    float32 endTime;

    DAVA_VIRTUAL_REFLECTION(CommandUpdateParticleLayerTime, CommandUpdateParticleLayerBase);
};

class CommandUpdateParticleLayerEnabled : public CommandUpdateParticleLayerBase
{
public:
    CommandUpdateParticleLayerEnabled(ParticleLayer* layer, bool isEnabled);
    void Redo() override;

protected:
    bool isEnabled;
    DAVA_VIRTUAL_REFLECTION(CommandUpdateParticleLayerEnabled, CommandUpdateParticleLayerBase);
};

class CommandUpdateParticleLayerLods : public CommandUpdateParticleLayerBase
{
public:
    CommandUpdateParticleLayerLods(ParticleLayer* layer, const Vector<bool>& lods);
    void Redo() override;

protected:
    Vector<bool> lods;
    DAVA_VIRTUAL_REFLECTION(CommandUpdateParticleLayerLods, CommandUpdateParticleLayerBase);
};

class CommandUpdateParticleSimplifiedForce : public CommandAction
{
public:
    CommandUpdateParticleSimplifiedForce(ParticleLayer* layer, uint32 forceId);

    void Init(RefPtr<PropertyLine<Vector3>> force, RefPtr<PropertyLine<float32>> forcesOverLife);

    void Redo() override;

    ParticleLayer* GetLayer() const
    {
        return layer;
    };
    uint32 GetForceIndex() const
    {
        return forceId;
    };

protected:
    ParticleLayer* layer;
    uint32 forceId;

    RefPtr<PropertyLine<Vector3>> force;
    RefPtr<PropertyLine<float32>> forcesOverLife;

    DAVA_VIRTUAL_REFLECTION(CommandUpdateParticleSimplifiedForce, CommandAction);
};

class CommandUpdateParticleForce : public CommandAction
{
public:
    struct ForceParams
    {
        String forceName;
        RefPtr<PropertyLine<Vector3>> forcePowerLine;
        RefPtr<PropertyLine<float32>> turbulenceLine;
        ParticleForce::eShape shape = ParticleForce::eShape::BOX;
        ParticleForce::eTimingType timingType = ParticleForce::eTimingType::CONSTANT;
        float32 radius = 0.0f;
        float32 windFrequency = 0.0f;
        float32 windTurbulence = 0.0f;
        float32 windTurbulenceFrequency = 0.0f;
        float32 windBias = 1.0f;
        uint32 backwardTurbulenceProbability = 0;
        uint32 reflectionPercent = 0;
        float32 pointGravityRadius = 1.0f;
        float32 planeScale = 1.0f;
        float32 reflectionChaos = 0.0f;
        float32 rndReflectionForceMin = 1.0f;
        float32 rndReflectionForceMax = 1.0f;
        float32 velocityThreshold = 1.0f;
        float32 startTime = 0.0f;
        float32 endTime = 15.0f;
        Vector3 boxSize;
        Vector3 forcePower;
        Vector3 direction;
        bool isActive = true;
        bool worldAlign = false;
        bool useInfinityRange = false;
        bool pointGravityUseRandomPointsOnSphere = false;
        bool isGlobal = false;
        bool killParticles = false;
        bool normalAsReflectionVector = true;
        bool randomizeReflectionForce = true;
    };

    CommandUpdateParticleForce(ParticleLayer* layer, uint32 forceId, ForceParams&& params);

    void Redo() override;
    void Undo() override;

    ParticleLayer* GetLayer() const
    {
        return layer;
    };
    uint32 GetForceIndex() const
    {
        return forceId;
    };

protected:
    void ApplyParams(ForceParams& params);

    ForceParams newParams;
    ForceParams oldParams;
    ParticleLayer* layer = nullptr;
    uint32 forceId = -1;

    DAVA_VIRTUAL_REFLECTION(CommandUpdateParticleForce, CommandAction);
};

// Load/save Particle Emitter Node.
class CommandLoadParticleEmitterFromYaml : public CommandAction
{
public:
    CommandLoadParticleEmitterFromYaml(ParticleEffectComponent* effect, ParticleEmitterInstance* emitter, const FilePath& path);

    void Redo() override;

    ParticleEffectComponent* GetEffect() const
    {
        return selectedEffect;
    }

    ParticleEmitterInstance* GetEmitterInstance() const
    {
        return instance;
    }

protected:
    ParticleEffectComponent* selectedEffect = nullptr;
    ParticleEmitterInstance* instance = nullptr;
    FilePath filePath;

    DAVA_VIRTUAL_REFLECTION(CommandLoadParticleEmitterFromYaml, CommandAction);
};

class CommandSaveParticleEmitterToYaml : public CommandAction
{
public:
    CommandSaveParticleEmitterToYaml(ParticleEffectComponent* effect, ParticleEmitterInstance* emitter, const FilePath& path);

    void Redo() override;

    ParticleEmitterInstance* GetEmitterInstance() const
    {
        return instance;
    }

protected:
    ParticleEffectComponent* selectedEffect = nullptr;
    ParticleEmitterInstance* instance = nullptr;
    FilePath filePath;

    DAVA_VIRTUAL_REFLECTION(CommandSaveParticleEmitterToYaml, CommandAction);
};

// Load/save Particle Inner Emitter Node.
class CommandLoadInnerParticleEmitterFromYaml : public CommandAction
{
public:
    CommandLoadInnerParticleEmitterFromYaml(ParticleEmitterInstance* emitter, const FilePath& path);

    void Redo() override;

    ParticleEmitterInstance* GetEmitterInstance() const
    {
        return instance;
    }

protected:
    ParticleEmitterInstance* instance = nullptr;
    FilePath filePath;

    DAVA_VIRTUAL_REFLECTION(CommandLoadInnerParticleEmitterFromYaml, CommandAction);
};

class CommandSaveInnerParticleEmitterToYaml : public CommandAction
{
public:
    CommandSaveInnerParticleEmitterToYaml(ParticleEmitterInstance* emitter, const FilePath& path);
    void Redo() override;

    ParticleEmitterInstance* GetEmitterInstance() const
    {
        return instance;
    }

protected:
    ParticleEmitterInstance* instance = nullptr;
    FilePath filePath;

    DAVA_VIRTUAL_REFLECTION(CommandSaveInnerParticleEmitterToYaml, CommandAction);
};

class CommandReloadEmitters : public RECommand
{
public:
    CommandReloadEmitters(ParticleEffectComponent* component_);
    ~CommandReloadEmitters();

    void Redo() override;
    void Undo() override;

    bool IsClean() const override
    {
        return true;
    }

    ParticleEffectComponent* GetComponent() const;

    const DAVA::Vector<DAVA::ParticleEmitterInstance*>& GetRedoEmitters() const
    {
        return redoParticleEmitterInstance;
    }
    const DAVA::Vector<DAVA::ParticleEmitterInstance*>& GetUndoEmitters() const
    {
        return undoParticleEmitterInstance;
    }

protected:
    void ReplaceComponentEmitters(const DAVA::Vector<DAVA::ParticleEmitterInstance*>& nextParticleEmitterInstances);

    DAVA::Vector<DAVA::ParticleEmitterInstance*> redoParticleEmitterInstance;
    DAVA::Vector<DAVA::ParticleEmitterInstance*> undoParticleEmitterInstance;

    bool undoDataInitialized = false;

    DAVA::ParticleEffectComponent* component;

    DAVA_VIRTUAL_REFLECTION(CommandReloadEmitters, RECommand);
};
} // namespace DAVA
