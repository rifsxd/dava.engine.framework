#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class ParticleForce;
class ParticleForceSimplified;
struct ParticleLayer;

class ParticleSimplifiedForceMoveCommand : public RECommand
{
public:
    ParticleSimplifiedForceMoveCommand(ParticleForceSimplified* force, ParticleLayer* oldLayer, ParticleLayer* newLayer);
    ~ParticleSimplifiedForceMoveCommand();

    void Undo() override;
    void Redo() override;

    ParticleForceSimplified* force;
    ParticleLayer* oldLayer;
    ParticleLayer* newLayer;

private:
    DAVA_VIRTUAL_REFLECTION(ParticleSimplifiedForceMoveCommand, RECommand);
};

class ParticleForceMoveCommand : public RECommand
{
public:
    ParticleForceMoveCommand(ParticleForce* force, ParticleLayer* oldLayer, ParticleLayer* newLayer);
    ~ParticleForceMoveCommand();

    void Undo() override;
    void Redo() override;

    ParticleForce* force;
    ParticleLayer* oldLayer;
    ParticleLayer* newLayer;

private:
    DAVA_VIRTUAL_REFLECTION(ParticleForceMoveCommand, RECommand);
};
} // namespace DAVA
