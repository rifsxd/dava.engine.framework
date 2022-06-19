#pragma once

#include <TArc/DataProcessing/SettingsNode.h>

#include <Math/Color.h>

namespace DAVA
{
enum SelectionSystemDrawMode
{
    SS_DRAW_NOTHING = 0x0,

    SS_DRAW_SHAPE = 0x1,
    SS_DRAW_CORNERS = 0x2,
    SS_DRAW_BOX = 0x4,
    SS_DRAW_NO_DEEP_TEST = 0x10,

    SS_DRAW_DEFAULT = SS_DRAW_CORNERS | SS_DRAW_BOX,
    SS_DRAW_ALL = 0xFFFFFFFF
};

class GlobalSceneSettings : public SettingsNode
{
public:
    float32 gridStep = 10.0f;
    float32 gridSize = 600.0f;
    float32 cameraSpeed0 = 35.0f;
    float32 cameraSpeed1 = 100.0f;
    float32 cameraSpeed2 = 250.0f;
    float32 cameraSpeed3 = 400.0f;
    float32 cameraFOV = 70.0f;
    float32 cameraNear = 1.0f;
    float32 cameraFar = 5000.0f;
    bool cameraUseDefaultSettings = true;
    float32 heightOnLandscape = 2.0f;
    float32 heightOnLandscapeStep = 0.5f;
    bool selectionSequent = false;
    bool selectionOnClick = false;
    SelectionSystemDrawMode selectionDrawMode = SelectionSystemDrawMode::SS_DRAW_DEFAULT;
    bool modificationByGizmoOnly = false;
    float32 gizmoScale = 1.0f;
    float32 debugBoxScale = 1.0f;
    float32 debugBoxUserScale = 1.0f;
    float32 debugBoxParticleScale = 1.0f;
    float32 debugBoxWaypointScale = 1.0f;
    bool dragAndDropWithShift = false;
    bool autoSelectNewEntity = true;
    bool saveEmitters = false;
    bool saveEntityPositionOnHierarchyChange = true;
    bool saveStaticOcclusion = true;
    uint32 defaultCustomColorIndex = 0;
    bool openLastScene = false;

    // Sound settings
    bool drawSoundObjects = false;
    Color soundObjectBoxColor = Color(0.0f, 0.8f, 0.4f, 0.2f);
    Color soundObjectSphereColor = Color(0.0f, 0.8f, 0.4f, 0.1f);

    // Scene grad settings
    uint32 grabSizeWidth = 1280;
    uint32 grabSizeHeight = 1024;

    DAVA_VIRTUAL_REFLECTION(GlobalSceneSettings, SettingsNode);
};

} // namespace DAVA
