#pragma once

#include <TArc/DataProcessing/TArcDataNode.h>

class SpritesPacker;

class SpritesPackerData : public DAVA::TArcDataNode
{
public:
    SpritesPackerData();
    ~SpritesPackerData() override;

    const SpritesPacker* GetSpritesPacker() const;

private:
    friend class SpritesPackerModule;
    friend struct SpritesPackerModuleTest;
    SpritesPacker* GetSpritesPacker();

    std::unique_ptr<SpritesPacker> spritesPacker;

    DAVA_VIRTUAL_REFLECTION(SpritesPackerData, DAVA::TArcDataNode);
};
