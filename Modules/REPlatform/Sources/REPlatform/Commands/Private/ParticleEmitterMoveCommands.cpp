#include "REPlatform/Commands/ParticleEmitterMoveCommands.h"

#include <Particles/ParticleEmitterInstance.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/ParticleEffectComponent.h>

namespace DAVA
{
ParticleEmitterMoveCommand::ParticleEmitterMoveCommand(ParticleEffectComponent* oldEffect_, ParticleEmitterInstance* emitter_,
                                                       ParticleEffectComponent* newEffect_, int newIndex_)
    : RECommand("Move particle emitter")
    , oldEffect(oldEffect_)
    , newEffect(newEffect_)
    , oldIndex(-1)
    , newIndex(newIndex_)
{
    if (nullptr != emitter_ && nullptr != oldEffect_)
    {
        oldIndex = oldEffect->GetEmitterInstanceIndex(emitter_);
        instance = RefPtr<ParticleEmitterInstance>::ConstructWithRetain(oldEffect->GetEmitterInstance(oldIndex));
        DVASSERT(instance->GetEmitter() == emitter_->GetEmitter());
    }
}

void ParticleEmitterMoveCommand::Undo()
{
    if ((instance.Get() == nullptr) || (instance->GetEmitter() == nullptr))
        return;

    if (nullptr != newEffect)
    {
        newEffect->RemoveEmitterInstance(instance.Get());
    }

    if (nullptr != oldEffect)
    {
        if (-1 != oldIndex)
        {
            oldEffect->InsertEmitterInstanceAt(instance.Get(), oldIndex);
        }
        else
        {
            oldEffect->AddEmitterInstance(instance.Get());
        }
    }
}

void ParticleEmitterMoveCommand::Redo()
{
    if ((instance.Get() == nullptr) || (instance->GetEmitter() == nullptr) || (newEffect == nullptr))
        return;

    if (nullptr != oldEffect)
    {
        oldEffect->RemoveEmitterInstance(instance.Get());
    }

    if (-1 != newIndex)
    {
        newEffect->InsertEmitterInstanceAt(instance.Get(), newIndex);
    }
    else
    {
        newEffect->AddEmitterInstance(instance.Get());
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(ParticleEmitterMoveCommand)
{
    ReflectionRegistrator<ParticleEmitterMoveCommand>::Begin()
    .End();
}
} // namespace DAVA
