#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Base/RefPtr.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class ParticleEffectComponent;
class ParticleEmitterInstance;
class ParticleEmitterMoveCommand : public RECommand
{
public:
    ParticleEmitterMoveCommand(ParticleEffectComponent* oldEffect, ParticleEmitterInstance* emitter, ParticleEffectComponent* newEffect, int newIndex);

    void Undo() override;
    void Redo() override;

    ParticleEffectComponent* GetOldComponent() const
    {
        return oldEffect;
    }

    ParticleEffectComponent* GetNewComponent() const
    {
        return newEffect;
    }

    ParticleEmitterInstance* GetEmitterInstance() const
    {
        return instance.Get();
    }

private:
    RefPtr<ParticleEmitterInstance> instance;
    ParticleEffectComponent* oldEffect;
    ParticleEffectComponent* newEffect;
    int32 oldIndex = -1;
    int32 newIndex;

    DAVA_VIRTUAL_REFLECTION(ParticleEmitterMoveCommand, RECommand);
};
} // namespace DAVA
