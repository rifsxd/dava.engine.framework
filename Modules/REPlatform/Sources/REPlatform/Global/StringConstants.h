#pragma once

#include <Base/String.h>

namespace DAVA
{
namespace ResourceEditor
{
// Node names
static const String VEGETATION_NODE_NAME = String("Vegetation");
static const String LANDSCAPE_NODE_NAME = String("Landscape");
static const String LIGHT_NODE_NAME = String("Light");
static const String SERVICE_NODE_NAME = String("Servicenode");
static const String CAMERA_NODE_NAME = String("Camera");
static const String IMPOSTER_NODE_NAME = String("Imposter");
static const String PARTICLE_EMITTER_NODE_NAME = String("Particle Emitter");
static const String USER_NODE_NAME = String("UserNode");
static const String SWITCH_NODE_NAME = String("SwitchNode");
static const String PARTICLE_EFFECT_NODE_NAME = String("Particle Effect");
static const String WIND_NODE_NAME = String("Wind");
static const String PATH_NODE_NAME = String("Path");
static const String LAYER_NODE_NAME = "Layer";
static const String ENTITY_NAME = String("Entity");

// Base node names
static const String EDITOR_BASE = "editor.";
static const String EDITOR_MAIN_CAMERA = "editor.main-camera";
static const String EDITOR_DEBUG_CAMERA = String("editor.debug-camera");
static const String EDITOR_ARROWS_NODE = "editor.arrows-node";
static const String EDITOR_CAMERA_LIGHT = String("editor.camera-light");
static const String EDITOR_2D_CAMERA = String("editor.2d-camera");
static const String EDITOR_SPRITE = String("Sprite");

// Headers
static const WideString CREATE_NODE_LANDSCAPE = L"createnode.landscape";
static const WideString CREATE_NODE_LIGHT = L"createnode.light";
static const WideString CREATE_NODE_SERVICE = L"createnode.servicenode";
static const WideString CREATE_NODE_CAMERA = L"createnode.camera";
static const WideString CREATE_NODE_IMPOSTER = L"createnode.imposter";
static const WideString CREATE_NODE_PARTICLE_EMITTER = L"createnode.particleemitter";
static const WideString CREATE_NODE_USER = L"createnode.usernode";
static const WideString CREATE_NODE_SWITCH = L"createnode.switchnode";
static const WideString CREATE_NODE_PARTICLE_EFFECT = L"Particle Effect";

// Properties
static const String EDITOR_REFERENCE_TO_OWNER = "editor.referenceToOwner";
static const String EDITOR_CONST_REFERENCE = "editor.constReferent";
static const String EDITOR_IS_LOCKED = "editor.isLocked";
static const String EDITOR_DO_NOT_REMOVE = "editor.donotremove";
static const String EDITOR_DYNAMIC_LIGHT_ENABLE = "editor.dynamiclight.enable";

//Documentation
static const String DOCUMENTATION_PATH = "~doc:/ResourceEditorHelp/";

//service strings

static const String CUSTOM_COLOR_TEXTURE_PROP = "customColorTexture";

static const String TILEMASK_EDITOR_BRUSH_SIZE_CAPTION = "Brush\nsize:";
static const String TILEMASK_EDITOR_BRUSH_IMAGE_CAPTION = "Brush\nimage:";
static const String TILEMASK_EDITOR_DRAW_CAPTION = "Normal draw";
static const String TILEMASK_EDITOR_COPY_PASTE_CAPTION = "Copy/paste";
static const String TILEMASK_EDITOR_TILE_TEXTURE_CAPTION = "Tile texture:";
static const String TILEMASK_EDITOR_STRENGTH_CAPTION = "Strength:";
static const String TILEMASK_EDITOR_BRUSH_SIZE_MIN = "tilemask-editor.brush-size.min";
static const String TILEMASK_EDITOR_BRUSH_SIZE_MAX = "tilemask-editor.brush-size.max";
static const String TILEMASK_EDITOR_STRENGTH_MIN = "tilemask-editor.strength.min";
static const String TILEMASK_EDITOR_STRENGTH_MAX = "tilemask-editor.strength.max";
static const String TILEMASK_EDITOR_TOOLS_PATH = "~res:/ResourceEditor/LandscapeEditor/Tools/";
static const String TILEMASK_EDITOR_ENABLE_ERROR = "Error enabling Tile Mask Editor. Make sure there is valid landscape at the scene.";
static const String TILEMASK_EDITOR_DISABLE_ERROR = "Error disabling Tile Mask Editor";

static const String CUSTOM_COLORS_BRUSH_SIZE_CAPTION = "Brush\nsize:";
static const String CUSTOM_COLORS_BRUSH_SIZE_MIN = "custom-colors.brush-size.min";
static const String CUSTOM_COLORS_BRUSH_SIZE_MAX = "custom-colors.brush-size.max";
static const String CUSTOM_COLORS_PROPERTY_COLORS = "LandscapeCustomColors";
static const String CUSTOM_COLORS_PROPERTY_DESCRIPTION = "LandscapeCustomColorsDescription";
static const String CUSTOM_COLORS_SAVE_CAPTION = "Save texture";
static const String CUSTOM_COLORS_LOAD_CAPTION = "Load texture";
static const String CUSTOM_COLORS_ENABLE_ERROR = "Error enabling Custom Colors editor. Make sure there is valid landscape at the scene.";
static const String CUSTOM_COLORS_DISABLE_ERROR = "Error disabling Custom Colors editor.";

static const String VISIBILITY_TOOL_AREA_SIZE_CAPTION = "Visibility Area Size:";
static const String VISIBILITY_TOOL_AREA_SIZE_MIN = "visibility-tool.area-size.min";
static const String VISIBILITY_TOOL_AREA_SIZE_MAX = "visibility-tool.area-size.max";
static const String VISIBILITY_TOOL_ENABLE_ERROR = "Error enabling Visibility Check Tool. Make sure there is valid landscape at the scene.";
static const String VISIBILITY_TOOL_DISABLE_ERROR = "Error disabling Visibility Check Tool";
static const String VISIBILITY_TOOL_SAVE_CAPTION = "Save visibility tool texture";
static const String VISIBILITY_TOOL_ADD_POINT_CAPTION = "Add Visibility Point";
static const String VISIBILITY_TOOL_SAVE_TEXTURE_CAPTION = "Save Texture";
static const String VISIBILITY_TOOL_COMPUTE_VISIBILITY_CAPTION = "Compute Visibility";

static const String RULER_TOOL_LENGTH_CAPTION = "Length:";
static const String RULER_TOOL_PREVIEW_LENGTH_CAPTION = "Preview length:";
static const String RULER_TOOL_ENABLE_ERROR = "Error enabling Ruler Tool. Make sure there is valid landscape at the scene.";
static const String RULER_TOOL_DISABLE_ERROR = "Error disabling Ruler Tool";
static const String LANDSCAPE_DIALOG_WRONG_PNG_ERROR = "PNG file should be in format A8 or A16.";

static const String HEIGHTMAP_EDITOR_BRUSH_SIZE_CAPTION = "Brush\nsize:";
static const String HEIGHTMAP_EDITOR_STRENGTH_CAPTION = "Strength:";
static const String HEIGHTMAP_EDITOR_AVERAGE_STRENGTH_CAPTION = "Average\nstrength:";
static const String HEIGHTMAP_EDITOR_BRUSH_SIZE_MIN = "heightmap-editor.brush-size.min";
static const String HEIGHTMAP_EDITOR_BRUSH_SIZE_MAX = "heightmap-editor.brush-size.max";
static const String HEIGHTMAP_EDITOR_STRENGTH_MAX = "heightmap-editor.strength.max";
static const String HEIGHTMAP_EDITOR_AVERAGE_STRENGTH_MIN = "heightmap-editor.average-strength.min";
static const String HEIGHTMAP_EDITOR_AVERAGE_STRENGTH_MAX = "heightmap-editor.average-strength.max";
static const String HEIGHTMAP_EDITOR_TOOLS_PATH = "~res:/ResourceEditor/LandscapeEditor/Tools/";
static const String HEIGHTMAP_EDITOR_ENABLE_ERROR = "Error enabling Height Map editor. Make sure there is valid landscape at the scene.";
static const String HEIGHTMAP_EDITOR_DISABLE_ERROR = "Error disabling Height Map editor.";
static const String HEIGHTMAP_EDITOR_RADIO_COPY_PASTE = "Copy/paste";
static const String HEIGHTMAP_EDITOR_RADIO_ABS_DROP = "Abs & Drop";
static const String HEIGHTMAP_EDITOR_RADIO_ABSOLUTE = "Absolute";
static const String HEIGHTMAP_EDITOR_RADIO_AVERAGE = "Average";
static const String HEIGHTMAP_EDITOR_RADIO_DROPPER = "Dropper";
static const String HEIGHTMAP_EDITOR_RADIO_RELATIVE = "Relative";
static const String HEIGHTMAP_EDITOR_CHECKBOX_HEIGHTMAP = "Height Map";
static const String HEIGHTMAP_EDITOR_CHECKBOX_TILEMASK = "Tile Mask";
static const String HEIGHTMAP_EDITOR_LABEL_BRUSH_IMAGE = "Brush\nimage:";
static const String HEIGHTMAP_EDITOR_LABEL_DROPPER_HEIGHT = "Height:";

static const String NOT_PASSABLE_TERRAIN_ENABLE_ERROR = "Error enabling Not Passable Terrain. Make sure there is valid landscape at the scene.";
static const String NOT_PASSABLE_TERRAIN_DISABLE_ERROR = "Error disabling Not Passable Terrain";

static const String SHORTCUT_BRUSH_SIZE_INCREASE_SMALL = "landscape-editor.brush-size.increase.small";
static const String SHORTCUT_BRUSH_SIZE_DECREASE_SMALL = "landscape-editor.brush-size.decrease.small";
static const String SHORTCUT_BRUSH_SIZE_INCREASE_LARGE = "landscape-editor.brush-size.increase.large";
static const String SHORTCUT_BRUSH_SIZE_DECREASE_LARGE = "landscape-editor.brush-size.decrease.large";
static const String SHORTCUT_BRUSH_IMAGE_NEXT = "landscape-editor.brush-image.next";
static const String SHORTCUT_BRUSH_IMAGE_PREV = "landscape-editor.brush-image.prev";
static const String SHORTCUT_TEXTURE_NEXT = "landscape-editor.texture.next";
static const String SHORTCUT_TEXTURE_PREV = "landscape-editor.texture.prev";
static const String SHORTCUT_STRENGTH_INCREASE_SMALL = "landscape-editor.strength.increase.small";
static const String SHORTCUT_STRENGTH_DECREASE_SMALL = "landscape-editor.strength.decrease.small";
static const String SHORTCUT_STRENGTH_INCREASE_LARGE = "landscape-editor.strength.increase.large";
static const String SHORTCUT_STRENGTH_DECREASE_LARGE = "landscape-editor.strength.decrease.large";
static const String SHORTCUT_AVG_STRENGTH_INCREASE_SMALL = "landscape-editor.average-strength.increase.small";
static const String SHORTCUT_AVG_STRENGTH_DECREASE_SMALL = "landscape-editor.average-strength.decrease.small";
static const String SHORTCUT_AVG_STRENGTH_INCREASE_LARGE = "landscape-editor.average-strength.increase.large";
static const String SHORTCUT_AVG_STRENGTH_DECREASE_LARGE = "landscape-editor.average-strength.decrease.large";
static const String SHORTCUT_SET_COPY_PASTE = "heightmap-editor.set-copy-paste";
static const String SHORTCUT_SET_ABSOLUTE = "heightmap-editor.set-absolute";
static const String SHORTCUT_SET_RELATIVE = "heightmap-editor.set-relative";
static const String SHORTCUT_SET_AVERAGE = "heightmap-editor.set-average";
static const String SHORTCUT_SET_ABS_DROP = "heightmap-editor.set-abs-drop";
static const String SHORTCUT_SET_DROPPER = "heightmap-editor.set-dropper";
static const String SHORTCUT_NORMAL_DRAW_TILEMASK = "tilemaskeditor-editor.set-normal-draw";
static const String SHORTCUT_COPY_PASTE_TILEMASK = "tilemaskeditor-editor.set-copy-paste";

static const String NO_LANDSCAPE_ERROR_MESSAGE = "Error. Check is there landscape at the scene.";
static const String INVALID_LANDSCAPE_MESSAGE = "Error. Check if all necessary properties of the landscape are set.";

static const String SCENE_NODE_DESIGNER_NAME_PROPERTY_NAME = "editor.designerName";
static const String SCENE_NODE_MODIFICATION_DATA_PROPERTY_NAME = "editor.modificationData";

static const String SLIDER_WIDGET_CHANGE_VALUE_TOOLTIP = "Double click to change value";
static const String SLIDER_WIDGET_CURRENT_VALUE = "Current value";

static const String TILE_TEXTURE_PREVIEW_CHANGE_COLOR_TOOLTIP = "Click to change color";

static const String LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS = "No errors.";
static const String LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS = "Error: All another landscape editors should be disabled";
static const String LANDSCAPE_EDITOR_SYSTEM_LANDSCAPE_ENTITY_ABSENT = "Error: landscape entity is absent.";
static const String LANDSCAPE_EDITOR_SYSTEM_TILEMASK_TEXTURE_ABSETN = "Error: tile mask texture is absent.";
static const String LANDSCAPE_EDITOR_SYSTEM_FULLTILED_TEXTURE_ABSETN = "Error: full tiled texture is absent.";
static const String LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE_ABSENT = "Error: tile texture is absent.";
static const String LANDSCAPE_EDITOR_SYSTEM_COLOR_TEXTURE_ABSENT = "Error: color texture is absent.";
static const String LANDSCAPE_EDITOR_SYSTEM_HEIGHTMAP_ABSENT = "Error: heightmap is absent.";
static const String LANDSCAPE_EDITOR_SYSTEM_CUSTOMCOLORS_ABSENT = "Warning: custom color texture is absent. Default texture will be created.";

static const String SCENE_TREE_WRONG_REF_TO_OWNER = "Wrong reference(s) to owner: ";
}
} // namespace DAVA
