#include "REPlatform/Scene/Systems/GridSystem.h"

#include "REPlatform/DataNodes/Settings/GlobalSceneSettings.h"

#include <TArc/Core/Deprecated.h>

#include <Render/Highlevel/RenderSystem.h>
#include <Render/RenderHelper.h>
#include <Scene3D/Scene.h>

#define LOWEST_GRID_STEP 0.1f
#define LOWEST_GRID_SIZE 1.0f

namespace DAVA
{
SceneGridSystem::SceneGridSystem(Scene* scene)
    : SceneSystem(scene)
{
}

void SceneGridSystem::Draw()
{
    GlobalSceneSettings* settings = Deprecated::GetDataNode<GlobalSceneSettings>();

    const float32 gridStep = settings->gridStep;
    const float32 gridMax = settings->gridSize;

    if (gridStep >= LOWEST_GRID_STEP && gridMax >= LOWEST_GRID_SIZE)
    {
        for (float32 x = -gridMax; x <= gridMax; x += gridStep)
        {
            const Vector3 v1(x, -gridMax, 0);
            const Vector3 v2(x, gridMax, 0);

            const Vector3 v3(-gridMax, x, 0);
            const Vector3 v4(gridMax, x, 0);

            if (x != 0.0f)
            {
                static const Color gridColor(0.4f, 0.4f, 0.4f, 1.0f);

                GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v1, v2, gridColor);
                GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v3, v4, gridColor);
            }
        }

        static const Color grid0Color(0.0f, 0.0f, 0.0f, 1.0f);

        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(Vector3(-gridMax, 0, 0), Vector3(gridMax, 0, 0), grid0Color);
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(Vector3(0, -gridMax, 0), Vector3(0, gridMax, 0), grid0Color);
    }
}
} // namespace DAVA
