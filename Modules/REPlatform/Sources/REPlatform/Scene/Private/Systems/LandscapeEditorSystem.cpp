#include "REPlatform/Scene/Systems/LandscapeEditorSystem.h"
#include "REPlatform/Scene/Systems/LandscapeProxy.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "REPlatform/Scene/Utils/Utils.h"

#include <Scene3D/Scene.h>

namespace DAVA
{
LandscapeEditorSystem::LandscapeEditorSystem(Scene* scene, const FilePath& cursorPath)
    : SceneSystem(scene)
    , cursorPathName(cursorPath)
    , cursorPosition(-100.f, -100.f)
    , prevCursorPos(-1.f, -1.f)
{
    cursorTexture = CreateSingleMipTexture(cursorPathName);
    cursorTexture->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);

    drawSystem = scene->GetSystem<LandscapeEditorDrawSystem>();

    Renderer::GetSignals().needRestoreResources.Connect(this, &LandscapeEditorSystem::RenderRestoreCallback);
}

LandscapeEditorSystem::~LandscapeEditorSystem()
{
    Renderer::GetSignals().needRestoreResources.Disconnect(this);
    SafeRelease(cursorTexture);
    drawSystem = nullptr;
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorSystem::IsCanBeEnabled() const
{
    return drawSystem->VerifyLandscape();
}

bool LandscapeEditorSystem::IsLandscapeEditingEnabled() const
{
    return enabled;
}

void LandscapeEditorSystem::UpdateCursorPosition()
{
    Vector3 landPos;
    isIntersectsLandscape = GetScene()->GetSystem<SceneCollisionSystem>()->LandRayTestFromCamera(landPos);
    if (isIntersectsLandscape)
    {
        landPos.x = std::floor(landPos.x);
        landPos.y = std::floor(landPos.y);

        const AABBox3& box = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();

        cursorPosition.x = (landPos.x - box.min.x) / (box.max.x - box.min.x);
        cursorPosition.y = (landPos.y - box.min.y) / (box.max.y - box.min.y);
        cursorPosition.x = cursorPosition.x;
        cursorPosition.y = 1.f - cursorPosition.y;

        drawSystem->SetCursorPosition(cursorPosition);
    }
    else
    {
        // hide cursor
        drawSystem->SetCursorPosition(Vector2(-100.f, -100.f));
    }
}

void LandscapeEditorSystem::RenderRestoreCallback()
{
    if (rhi::NeedRestoreTexture(cursorTexture->handle))
    {
        ScopedPtr<Image> image(ImageSystem::LoadSingleMip(cursorPathName));
        rhi::UpdateTexture(cursorTexture->handle, image->GetData(), 0);
    }
}
} // namespace DAVA
