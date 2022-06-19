#pragma once

#include "REPlatform/Scene/Private/Systems/VisibilityCheckRenderer.h"
#include "REPlatform/Scene/Systems/EditorSceneSystem.h"

#include <Entity/SceneSystem.h>

namespace DAVA
{
class Landscape;
class VisibilityCheckSystem : public SceneSystem, VisibilityCheckRendererDelegate, public EditorSceneSystem
{
public:
    static void ReleaseCubemapRenderTargets();

public:
    VisibilityCheckSystem(Scene* scene);
    ~VisibilityCheckSystem();

    void RegisterEntity(Entity* entity) override;
    void UnregisterEntity(Entity* entity) override;
    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    void Recalculate();
    void Process(float32 timeElapsed) override;

    void InvalidateMaterials();

    void FixCurrentFrame();
    void ReleaseFixedFrame();

protected:
    void Draw() override;

private:
    using EntityMap = Map<Entity*, Vector<Vector3>>;

    void BuildPointSetForEntity(EntityMap::value_type& item);
    void BuildIndexSet();
    Color GetNormalizedColorForEntity(const EntityMap::value_type& item) const;

    Camera* GetRenderCamera() const;
    Camera* GetFinalGatherCamera() const;

    void UpdatePointSet();
    void Prerender();

    bool CacheIsValid();
    void BuildCache();

    bool ShouldDrawRenderObject(RenderObject*) override;

    struct StateCache
    {
        Size2i viewportSize;
        Matrix4 viewprojMatrix;
        Camera* camera = nullptr;
    };

private:
    EntityMap entitiesWithVisibilityComponent;
    Landscape* landscape = nullptr;
    Vector<VisibilityCheckRenderer::VisbilityPoint> controlPoints;
    Vector<uint32> controlPointIndices;
    Map<RenderObject*, Entity*> renderObjectToEntity;
    VisibilityCheckRenderer renderer;
    ScopedPtr<NMaterial> debugMaterial;
    StateCache stateCache;
    size_t currentPointIndex = 0;
    bool shouldPrerender = true;
    bool forceRebuildPoints = true;
    bool shouldFixFrame = false;
};
} // namespace DAVA
