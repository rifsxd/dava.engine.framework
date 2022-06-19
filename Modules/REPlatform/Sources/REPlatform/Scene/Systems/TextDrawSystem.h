#pragma once

#include "REPlatform/Scene/Systems/EditorSceneSystem.h"

// framework
#include <Entity/SceneSystem.h>
#include <Render/2D/GraphicFont.h>

namespace DAVA
{
class NMaterial;
class SceneCameraSystem;
class TextDrawSystem : public SceneSystem, public EditorSceneSystem
{
public:
    enum class Align : uint8
    {
        TopLeft,
        TopCenter,
        TopRight,
        Left,
        Center,
        Right,
        BottomLeft,
        BottomCenter,
        BottomRight
    };

public:
    TextDrawSystem(Scene* scene);
    ~TextDrawSystem();

    void PrepareForRemove() override;
    Vector2 ToPos2d(const Vector3& pos3d) const;

    void DrawText(const Vector2& pos2d, const String& text, const Color& color, Align align = Align::TopLeft);
    void DrawText(const Vector2& pos2d, const WideString& text, const Color& color, float32 fontSize, Align align = Align::TopLeft);

    GraphicFont* GetFont() const;
    float32 GetFontSize() const;

protected:
    void Draw() override;

    struct TextToDraw
    {
        TextToDraw(const Vector2& pos_, const WideString& text_, const Color& color_, Align align_, float32 fontSize_)
            : pos(pos_)
            , text(text_)
            , color(color_)
            , align(align_)
            , fontSize(fontSize_)
        {
        }

        Vector2 pos;
        WideString text;
        Color color;
        Align align = Align::TopLeft;
        float32 fontSize = 10.f;
    };

    using GraphicFontVertexVector = Vector<GraphicFont::GraphicFontVertex>;

    void AdjustPositionBasedOnAlign(float32& x, float32& y, const Size2i& size, Align align);
    void PushNextBatch(const Color& color);

private:
    SceneCameraSystem* cameraSystem = nullptr;
    GraphicFont* font = nullptr;
    float32 fontSize = 14.f;
    NMaterial* fontMaterial = nullptr;
    Vector<TextToDraw> textToDraw;
    GraphicFontVertexVector vertices;
};

inline GraphicFont* TextDrawSystem::GetFont() const
{
    return font;
}

inline float32 TextDrawSystem::GetFontSize() const
{
    return fontSize;
}
} // namespace DAVA
