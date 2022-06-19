#include "REPlatform/DataNodes/Settings/GlobalSceneSettings.h"

#include <Base/GlobalEnum.h>
#include <Reflection/ReflectionRegistrator.h>

ENUM_DECLARE(DAVA::SelectionSystemDrawMode)
{
    ENUM_ADD(DAVA::SS_DRAW_SHAPE);
    ENUM_ADD(DAVA::SS_DRAW_CORNERS);
    ENUM_ADD(DAVA::SS_DRAW_BOX);
    ENUM_ADD(DAVA::SS_DRAW_NO_DEEP_TEST);
}

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(GlobalSceneSettings)
{
    ReflectionRegistrator<GlobalSceneSettings>::Begin()[M::DisplayName("Scene"), M::SettingsSortKey(90)]
    .ConstructorByPointer()
    .Field("openLastScene", &GlobalSceneSettings::openLastScene)[M::DisplayName("Open last opened scene on launch")]
    .Field("dragAndDropWithShift", &GlobalSceneSettings::dragAndDropWithShift)[M::DisplayName("Drag'n'Drop with shift")]
    .Field("saveEmitters", &GlobalSceneSettings::saveEmitters)[M::DisplayName("Save Emitters")]
    .Field("saveEntityPositionOnHierarchyChange", &GlobalSceneSettings::saveEntityPositionOnHierarchyChange)[DAVA::M::DisplayName("Save entity position on hierarchy change")]
    .Field("saveStaticOcclusion", &GlobalSceneSettings::saveStaticOcclusion)[M::DisplayName("Save static occlusion")]
    .Field("defaultCustomColorIndex", &GlobalSceneSettings::defaultCustomColorIndex)[M::DisplayName("Default custom color index")]
    .Field("selectionDrawMode", &GlobalSceneSettings::selectionDrawMode)[M::DisplayName("Selection draw mode"), M::FlagsT<SelectionSystemDrawMode>()]
    .Field("gridStep", &GlobalSceneSettings::gridStep)[M::DisplayName("Step"), M::Group("Grid")]
    .Field("gridSize", &GlobalSceneSettings::gridSize)[M::DisplayName("Size"), M::Group("Grid")]
    .Field("cameraSpeed0", &GlobalSceneSettings::cameraSpeed0)[M::DisplayName("Speed 0"), M::Group("Camera")]
    .Field("cameraSpeed1", &GlobalSceneSettings::cameraSpeed1)[M::DisplayName("Speed 1"), M::Group("Camera")]
    .Field("cameraSpeed2", &GlobalSceneSettings::cameraSpeed2)[M::DisplayName("Speed 2"), M::Group("Camera")]
    .Field("cameraSpeed3", &GlobalSceneSettings::cameraSpeed3)[M::DisplayName("Speed 3"), M::Group("Camera")]
    .Field("cameraFOV", &GlobalSceneSettings::cameraFOV)[M::DisplayName("Camera FOV"), M::Group("Camera")]
    .Field("cameraNear", &GlobalSceneSettings::cameraNear)[M::DisplayName("Camera near"), M::Group("Camera")]
    .Field("cameraFar", &GlobalSceneSettings::cameraFar)[M::DisplayName("Camera far"), M::Group("Camera")]
    .Field("cameraRestoreFullParameters", &GlobalSceneSettings::cameraUseDefaultSettings)[M::DisplayName("Use default settings for editor.camera"), M::Group("Camera")]
    .Field("heightOnLandscape", &GlobalSceneSettings::heightOnLandscape)[M::DisplayName("Height"), M::Group("Snap camera to landscape")]
    .Field("heightOnLandscapeStep", &GlobalSceneSettings::heightOnLandscapeStep)[M::DisplayName("Height step"), M::Group("Snap camera to landscape")]
    .Field("selectionSequent", &GlobalSceneSettings::selectionSequent)[M::DisplayName("Select Sequent"), M::Group("Selection")]
    .Field("selectionOnClick", &GlobalSceneSettings::selectionOnClick)[M::DisplayName("Select on click"), M::Group("Selection")]
    .Field("autoSelectNewEntity", &GlobalSceneSettings::autoSelectNewEntity)[M::DisplayName("Autoselect new entity"), M::Group("Selection")]
    .Field("modificationByGizmoOnly", &GlobalSceneSettings::modificationByGizmoOnly)[M::DisplayName("Modify by Gizmo only"), M::Group("Modification Gyzmo")]
    .Field("gizmoScale", &GlobalSceneSettings::gizmoScale)[M::DisplayName("Gizmo scale"), M::Group("Modification Gyzmo")]
    .Field("debugBoxScale", &GlobalSceneSettings::debugBoxScale)[M::DisplayName("Box scale"), M::Group("Debug draw")]
    .Field("debugBoxUserScale", &GlobalSceneSettings::debugBoxUserScale)[M::DisplayName("Box user scale"), M::Group("Debug draw")]
    .Field("debugBoxParticleScale", &GlobalSceneSettings::debugBoxParticleScale)[M::DisplayName("Box particle scale"), M::Group("Debug draw")]
    .Field("debugBoxWaypointScale", &GlobalSceneSettings::debugBoxWaypointScale)[M::DisplayName("Box waypoint scale"), M::Group("Debug draw")]
    .Field("drawSoundObjects", &GlobalSceneSettings::drawSoundObjects)[M::DisplayName("Draw sound objects"), M::Group("Sound")]
    .Field("soundObjectBoxColor", &GlobalSceneSettings::soundObjectBoxColor)[M::DisplayName("Sound box color"), M::Group("Sound")]
    .Field("soundObjectSphereColor", &GlobalSceneSettings::soundObjectSphereColor)[M::DisplayName("Sound sphere color"), M::Group("Sound")]
    .Field("grabSizeWidth", &GlobalSceneSettings::grabSizeWidth)[M::DisplayName("Texture width"), M::Group("Grab scene")]
    .Field("grabSizeHeight", &GlobalSceneSettings::grabSizeHeight)[M::DisplayName("Texture height"), M::Group("Grab scene")]
    .End();
}
} // namespace DAVA
