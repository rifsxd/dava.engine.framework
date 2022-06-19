#include "REPlatform/Commands/ParticleLayerRemoveCommand.h"

#include <Particles/ParticleEmitter.h>
#include <Particles/ParticleLayer.h>

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
ParticleLayerRemoveCommand::ParticleLayerRemoveCommand(ParticleEmitter* _emitter, ParticleLayer* _layer)
    : RECommand("Remove particle layer")
    , layer(_layer)
    , before(nullptr)
    , emitter(_emitter)
{
    SafeRetain(layer);

    if ((nullptr != layer) && (nullptr != emitter))
    {
        before = emitter->GetNextLayer(layer);
    }
}

ParticleLayerRemoveCommand::~ParticleLayerRemoveCommand()
{
    SafeRelease(layer);
}

void ParticleLayerRemoveCommand::Undo()
{
    if (nullptr != layer && nullptr != emitter)
    {
        if (nullptr != before)
        {
            emitter->InsertLayer(layer, before);
        }
        else
        {
            emitter->AddLayer(layer);
        }
    }
}

void ParticleLayerRemoveCommand::Redo()
{
    if (nullptr != layer && nullptr != emitter)
    {
        emitter->RemoveLayer(layer);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(ParticleLayerRemoveCommand)
{
    ReflectionRegistrator<ParticleLayerRemoveCommand>::Begin()
    .End();
}

} // namespace DAVA
