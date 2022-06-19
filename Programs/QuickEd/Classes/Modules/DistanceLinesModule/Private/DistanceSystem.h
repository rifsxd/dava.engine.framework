#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "Modules/CanvasModule/CanvasDataAdapter.h"

#include <TArc/DataProcessing/DataWrapper.h>
#include <TArc/DataProcessing/DataListener.h>
#include <TArc/DataProcessing/SettingsNode.h>

#include <Math/Color.h>
#include <Render/2D/Font.h>
#include <Base/RefPtr.h>

class DistanceLinesFactory;

class DistanceSystem : public BaseEditorSystem
{
public:
    DistanceSystem(DAVA::ContextAccessor* accessor);
    ~DistanceSystem() override;

    DAVA::Function<ControlNode*(void)> getHighlight;

private:
    eSystems GetOrder() const override;
    void OnUpdate() override;

    bool CanProcessInput(DAVA::UIEvent* currentInput, eInputSource inputSource) const override;
    void ProcessInput(DAVA::UIEvent* currentInput, eInputSource inputSource) override;

    bool CanDrawDistances() const;

    void DrawLines(const DAVA::Rect& selectedRect, const DAVA::Rect& highlightedRect, const DAVA::Matrix3& transformMatrix) const;
    void DrawSolidLine(DAVA::Vector2 startPos, DAVA::Vector2 endPos, const DAVA::Matrix3& transformMatrix, DAVA::Vector2::eAxis axis) const;
    void DrawDotLine(const DAVA::Rect& rect, DAVA::Vector2 endPos, const DAVA::Matrix3& transformMatrix, DAVA::Vector2::eAxis axis) const;

    //internal cache depending on inputState
    //as an example we can not draw distances after mouse scroll events but we can draw distances after mouse move events
    bool canDrawDistancesAfterInput = false;
};
