#pragma once

#include "REPlatform/Commands/RECommand.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleEmitter.h"

namespace DAVA
{
struct ParticleLayer;
class ParticleEmitter;

class ParticleLayerRemoveCommand : public RECommand
{
public:
    ParticleLayerRemoveCommand(ParticleEmitter* emitter, ParticleLayer* layer);
    ~ParticleLayerRemoveCommand();

    void Undo() override;
    void Redo() override;

    ParticleLayer* layer;
    ParticleLayer* before;
    ParticleEmitter* emitter;

private:
    DAVA_VIRTUAL_REFLECTION(ParticleLayerRemoveCommand, RECommand);
};
} // namespace DAVA
