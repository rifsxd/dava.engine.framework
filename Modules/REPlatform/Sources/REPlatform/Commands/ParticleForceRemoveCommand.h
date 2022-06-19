#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class ParticleForce;
struct ParticleLayer;
class ParticleForceRemoveCommand : public RECommand
{
public:
    ParticleForceRemoveCommand(ParticleForce* force, ParticleLayer* layer);
    ~ParticleForceRemoveCommand();

    void Undo() override;
    void Redo() override;

    ParticleForce* force;
    ParticleLayer* layer;

private:
    DAVA_VIRTUAL_REFLECTION(ParticleForceRemoveCommand, RECommand);
};
} // namespace DAVA
