#pragma once

#include "REPlatform/Scene/Systems/LandscapeEditorDrawSystem.h"
#include "REPlatform/Scene/Systems/EditorSceneSystem.h"

#include <Entity/SceneSystem.h>

namespace DAVA
{
class SceneCollisionSystem;
class LandscapeEditorDrawSystem;
class EntityModificationSystem;

class LandscapeEditorSystem : public SceneSystem, public EditorSceneSystem
{
public:
    LandscapeEditorSystem(Scene* scene, const FilePath& cursorPathname);
    ~LandscapeEditorSystem() override;

    bool IsLandscapeEditingEnabled() const;

protected:
    LandscapeEditorDrawSystem::eErrorType IsCanBeEnabled() const;

    void UpdateCursorPosition();
    void RenderRestoreCallback();

protected:
    LandscapeEditorDrawSystem* drawSystem = nullptr;
    FilePath cursorPathName;
    Vector2 cursorPosition;
    Vector2 prevCursorPos;
    Texture* cursorTexture = nullptr;
    float32 cursorSize = 0.0f;
    float32 landscapeSize = 0.0f;
    bool isIntersectsLandscape = false;
    bool enabled = false;
};
} // namespace DAVA
