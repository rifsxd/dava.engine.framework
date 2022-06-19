#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "Modules/CanvasModule/CanvasDataAdapter.h"

#include <TArc/DataProcessing/SettingsNode.h>

#include <Math/Vector.h>
#include <Math/Color.h>

class DisplaySafeAreaPreferences : public DAVA::SettingsNode
{
public:
    DAVA::Color linesColor = DAVA::Color(0.925f, 0.075f, 0.075f, 0.8f);
    bool isVisible = false;
    bool isEnabled = false;
    DAVA::float32 leftInset = 44.0f;
    DAVA::float32 topInset = 0.0f;
    DAVA::float32 rightInset = 44.0f;
    DAVA::float32 bottomInset = 21.0f;
    bool isLeftNotch = false;
    bool isRightNotch = false;

    DAVA_VIRTUAL_REFLECTION(SafeAreaPreferences, DAVA::SettingsNode);
};

class DisplaySafeArea : public BaseEditorSystem
{
public:
    DisplaySafeArea(DAVA::ContextAccessor* accessor);
    ~DisplaySafeArea() override;

private:
    eSystems GetOrder() const override;
    void OnUpdate() override;

    CanvasDataAdapter canvasDataAdapter;
};
