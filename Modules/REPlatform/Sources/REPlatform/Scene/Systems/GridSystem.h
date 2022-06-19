#pragma once

#include "REPlatform/Scene/Systems/EditorSceneSystem.h"

#include <Entity/SceneSystem.h>

namespace DAVA
{
class SceneGridSystem : public SceneSystem, public EditorSceneSystem
{
public:
    SceneGridSystem(Scene* scene);

    void PrepareForRemove() override
    {
    }

protected:
    void Draw() override;
};
} // namespace DAVA
