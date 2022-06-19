#include "REPlatform/Commands/ParticleForceMoveCommand.h"

#include <Particles/ParticleLayer.h>
#include <Particles/ParticleForce.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
ParticleSimplifiedForceMoveCommand::ParticleSimplifiedForceMoveCommand(ParticleForceSimplified* _force, ParticleLayer* _oldLayer, ParticleLayer* _newLayer)
    : RECommand("Move particle simplified force")
    , force(_force)
    , oldLayer(_oldLayer)
    , newLayer(_newLayer)
{
    SafeRetain(force);
}

ParticleSimplifiedForceMoveCommand::~ParticleSimplifiedForceMoveCommand()
{
    SafeRelease(force);
}

void ParticleSimplifiedForceMoveCommand::Undo()
{
    if (NULL != force)
    {
        if (NULL != newLayer)
        {
            newLayer->RemoveSimplifiedForce(force);
        }

        if (NULL != oldLayer)
        {
            oldLayer->AddSimplifiedForce(force);
        }
    }
}

void ParticleSimplifiedForceMoveCommand::Redo()
{
    if (NULL != force)
    {
        if (NULL != oldLayer)
        {
            oldLayer->RemoveSimplifiedForce(force);
        }

        if (NULL != newLayer)
        {
            newLayer->AddSimplifiedForce(force);
        }
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(ParticleSimplifiedForceMoveCommand)
{
    ReflectionRegistrator<ParticleSimplifiedForceMoveCommand>::Begin()
    .End();
}

ParticleForceMoveCommand::ParticleForceMoveCommand(ParticleForce* _force, ParticleLayer* _oldLayer, ParticleLayer* _newLayer)
    : RECommand("Move particle force")
    , force(_force)
    , oldLayer(_oldLayer)
    , newLayer(_newLayer)
{
    SafeRetain(force);
}

ParticleForceMoveCommand::~ParticleForceMoveCommand()
{
    SafeRelease(force);
}

void ParticleForceMoveCommand::Undo()
{
    if (nullptr != force)
    {
        if (nullptr != newLayer)
        {
            newLayer->RemoveForce(force);
        }

        if (nullptr != oldLayer)
        {
            oldLayer->AddForce(force);
        }
    }
}

void ParticleForceMoveCommand::Redo()
{
    if (nullptr != force)
    {
        if (nullptr != oldLayer)
        {
            oldLayer->RemoveForce(force);
        }

        if (nullptr != newLayer)
        {
            newLayer->AddForce(force);
        }
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(ParticleForceMoveCommand)
{
    ReflectionRegistrator<ParticleForceMoveCommand>::Begin()
    .End();
}
} // namespace DAVA
