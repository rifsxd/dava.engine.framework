#include "REPlatform/Commands/ParticleLayerMoveCommand.h"

#include <Particles/ParticleLayer.h>
#include <Particles/ParticleEmitterInstance.h>

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
ParticleLayerMoveCommand::ParticleLayerMoveCommand(ParticleEmitterInstance* oldEmitter_, ParticleLayer* layer_,
                                                   ParticleEmitterInstance* newEmitter_, ParticleLayer* newBefore_ /* = nullptr */)
    : RECommand("Move particle layer")
    , layer(layer_)
    , oldEmitter(oldEmitter_)
    , newEmitter(newEmitter_)
    , newBefore(newBefore_)
{
    SafeRetain(layer);

    if ((layer != nullptr) && (oldEmitter != nullptr))
    {
        oldBefore = oldEmitter->GetEmitter()->GetNextLayer(layer);
    }
}

ParticleLayerMoveCommand::~ParticleLayerMoveCommand()
{
    SafeRelease(layer);
}

void ParticleLayerMoveCommand::Undo()
{
    if (layer == nullptr)
        return;

    if (newEmitter != nullptr)
    {
        newEmitter->GetEmitter()->RemoveLayer(layer);
    }

    if (oldEmitter != nullptr)
    {
        if (oldBefore != nullptr)
        {
            oldEmitter->GetEmitter()->InsertLayer(layer, oldBefore);
        }
        else
        {
            oldEmitter->GetEmitter()->AddLayer(layer);
        }
    }
}

void ParticleLayerMoveCommand::Redo()
{
    if ((layer == nullptr) || (newEmitter == nullptr))
        return;

    if (nullptr != oldEmitter)
    {
        oldEmitter->GetEmitter()->RemoveLayer(layer);
    }

    if (nullptr != newBefore)
    {
        newEmitter->GetEmitter()->InsertLayer(layer, newBefore);
    }
    else
    {
        newEmitter->GetEmitter()->AddLayer(layer);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(ParticleLayerMoveCommand)
{
    ReflectionRegistrator<ParticleLayerMoveCommand>::Begin()
    .End();
}
} // namespace DAVA
