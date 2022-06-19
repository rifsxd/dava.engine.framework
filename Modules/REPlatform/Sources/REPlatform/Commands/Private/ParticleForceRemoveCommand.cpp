#include "REPlatform/Commands/ParticleForceRemoveCommand.h"

#include <Particles/ParticleLayer.h>
#include <Particles/ParticleForce.h>

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
ParticleForceRemoveCommand::ParticleForceRemoveCommand(ParticleForce* _force, ParticleLayer* _layer)
    : RECommand("Remove particle force")
    , force(_force)
    , layer(_layer)
{
    SafeRetain(force);
}

ParticleForceRemoveCommand::~ParticleForceRemoveCommand()
{
    SafeRelease(force);
}

void ParticleForceRemoveCommand::Undo()
{
    if (NULL != layer && NULL != force)
    {
        layer->AddForce(force);
    }
}

void ParticleForceRemoveCommand::Redo()
{
    if (NULL != layer && NULL != force)
    {
        layer->RemoveForce(force);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(ParticleForceRemoveCommand)
{
    ReflectionRegistrator<ParticleForceRemoveCommand>::Begin()
    .End();
}
} // namespace DAVA