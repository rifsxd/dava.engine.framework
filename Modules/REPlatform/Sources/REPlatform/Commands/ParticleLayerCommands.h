#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Base/RefPtr.h>
#include <FileSystem/FilePath.h>
#include <Particles/ParticlePropertyLine.h>
#include <Render/RenderBase.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
struct ParticleLayer;
class CommandChangeLayerMaterialProperties : public RECommand
{
public:
    CommandChangeLayerMaterialProperties(ParticleLayer* layer, const FilePath& spritePath, eBlending blending, bool enableFog, bool enableBlending);

    void Undo() override;
    void Redo() override;

    ParticleLayer* GetLayer() const;

private:
    struct LayerParams
    {
        FilePath spritePath;
        eBlending blending = BLENDING_NONE;
        bool enableFog = false;
        bool enableBlending = false;
    };

    void ApplyParams(const LayerParams& params);

private:
    LayerParams newParams;
    LayerParams oldParams;
    ParticleLayer* layer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandChangeLayerMaterialProperties, RECommand);
};

class CommandChangeFlowProperties : public RECommand
{
public:
    struct FlowParams
    {
        FilePath spritePath;
        bool enableFlow = false;
        bool enabelFlowAnimation = false;
        RefPtr<PropertyLine<float32>> flowSpeed;
        RefPtr<PropertyLine<float32>> flowSpeedVariation;
        RefPtr<PropertyLine<float32>> flowOffset;
        RefPtr<PropertyLine<float32>> flowOffsetVariation;
    };

    CommandChangeFlowProperties(ParticleLayer* layer_, FlowParams&& params);

    void Undo() override;
    void Redo() override;

    ParticleLayer* GetLayer() const;

private:
    void ApplyParams(FlowParams& params);

    FlowParams newParams;
    FlowParams oldParams;
    ParticleLayer* layer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandChangeFlowProperties, RECommand);
};

class CommandChangeNoiseProperties : public RECommand
{
public:
    struct NoiseParams
    {
        FilePath noisePath;
        bool enableNoise = false;
        RefPtr<PropertyLine<float32>> noiseScale;
        RefPtr<PropertyLine<float32>> noiseScaleVariation;
        RefPtr<PropertyLine<float32>> noiseScaleOverLife;
        bool enableNoiseScroll = false;
        RefPtr<PropertyLine<float32>> noiseUScrollSpeed;
        RefPtr<PropertyLine<float32>> noiseUScrollSpeedVariation;
        RefPtr<PropertyLine<float32>> noiseUScrollSpeedOverLife;
        RefPtr<PropertyLine<float32>> noiseVScrollSpeed;
        RefPtr<PropertyLine<float32>> noiseVScrollSpeedVariation;
        RefPtr<PropertyLine<float32>> noiseVScrollSpeedOverLife;
    };

    CommandChangeNoiseProperties(ParticleLayer* layer, NoiseParams&& params);

    void Undo() override;
    void Redo() override;

    ParticleLayer* GetLayer() const;

private:
    void ApplyParams(NoiseParams& params);

    NoiseParams newParams;
    NoiseParams oldParams;
    ParticleLayer* layer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandChangeNoiseProperties, RECommand);
};

class CommandChangeFresnelToAlphaProperties : public RECommand
{
public:
    struct FresnelToAlphaParams
    {
        bool useFresnelToAlpha = false;
        float32 fresnelToAlphaBias = 0.0f;
        float32 fresnelToAlphaPower = 0.0f;
    };

    CommandChangeFresnelToAlphaProperties(ParticleLayer* layer, FresnelToAlphaParams&& params);

    void Undo() override;
    void Redo() override;

    ParticleLayer* GetLayer() const;

private:
    void ApplyParams(FresnelToAlphaParams& params);

    FresnelToAlphaParams newParams;
    FresnelToAlphaParams oldParams;
    ParticleLayer* layer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandChangeFresnelToAlphaProperties, RECommand);
};

class CommandChangeParticlesStripeProperties : public RECommand
{
public:
    struct StripeParams
    {
        float32 stripeVertexSpawnStep;
        float32 stripeLifetime;
        float32 stripeStartSize;
        float32 stripeUScrollSpeed;
        float32 stripeVScrollSpeed;
        float32 stripeFadeDistanceFromTop;
        bool stripeInheritPositionForBase;
        bool usePerspectiveMapping;
        RefPtr<PropertyLine<float32>> stripeTextureTileOverLife;
        RefPtr<PropertyLine<float32>> stripeSizeOverLife;
        RefPtr<PropertyLine<float32>> stripeNoiseUScrollSpeedOverLife;
        RefPtr<PropertyLine<float32>> stripeNoiseVScrollSpeedOverLife;
        RefPtr<PropertyLine<Color>> stripeColorOverLife;
    };

    CommandChangeParticlesStripeProperties(ParticleLayer* layer, StripeParams&& params);

    void Undo() override;
    void Redo() override;

    ParticleLayer* GetLayer() const;

private:
    void ApplyParams(StripeParams& params);

    StripeParams newParams;
    StripeParams oldParams;
    ParticleLayer* layer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandChangeParticlesStripeProperties, RECommand);
};

class CommandChangeAlphaRemapProperties : public RECommand
{
public:
    struct AlphaRemapParams
    {
        FilePath alphaRemapPath;
        bool enableAlphaRemap = false;
        float32 alphaRemapLoopCount = 1.0f;
        RefPtr<PropertyLine<float32>> alphaRemapOverLife;
    };

    CommandChangeAlphaRemapProperties(ParticleLayer* layer, AlphaRemapParams&& params);

    void Undo() override;
    void Redo() override;

    ParticleLayer* GetLayer() const;

private:
    void ApplyParams(AlphaRemapParams& params);

    AlphaRemapParams newParams;
    AlphaRemapParams oldParams;
    ParticleLayer* layer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandChangeAlphaRemapProperties, RECommand);
};

class CommandChangeThreePointGradientProperties : public RECommand
{
public:
    struct ThreePointGradientParams
    {
        RefPtr<PropertyLine<Color>> gradientColorForWhite;
        RefPtr<PropertyLine<Color>> gradientColorForBlack;
        RefPtr<PropertyLine<Color>> gradientColorForMiddle;
        RefPtr<PropertyLine<float32>> gradientMiddlePointLine;
        float32 gradientMiddlePoint = 0.5f;
        bool useThreePointGradient = false;
    };

    CommandChangeThreePointGradientProperties(DAVA::ParticleLayer* layer, ThreePointGradientParams&& params);
    void Undo() override;
    void Redo() override;

    ParticleLayer* GetLayer() const;

private:
    void ApplyParams(ThreePointGradientParams& params);

    ThreePointGradientParams newParams;
    ThreePointGradientParams oldParams;
    ParticleLayer* layer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandChangeThreePointGradientProperties, RECommand);
};
} // namespace DAVA
