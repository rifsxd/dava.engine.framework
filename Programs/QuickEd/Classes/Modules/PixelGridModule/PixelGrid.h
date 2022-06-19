#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "Modules/CanvasModule/CanvasDataAdapter.h"

#include <TArc/DataProcessing/SettingsNode.h>

#include <Math/Vector.h>
#include <Math/Color.h>

namespace DAVA
{
class Any;
}

class PixelGridPreferences : public DAVA::SettingsNode
{
public:
    DAVA::Color gridColor = DAVA::Color(0.925f, 0.925f, 0.925f, 0.15f);
    bool isVisible = true;
    DAVA::float32 scaleToDisplay = 800.0f;

    DAVA_VIRTUAL_REFLECTION(PixelGridPreferences, DAVA::SettingsNode);
};

class PixelGrid : public BaseEditorSystem
{
public:
    PixelGrid(DAVA::ContextAccessor* accessor);
    ~PixelGrid() override;

private:
    eSystems GetOrder() const override;
    void OnUpdate() override;

    bool CanShowGrid() const;

    CanvasDataAdapter canvasDataAdapter;
};
