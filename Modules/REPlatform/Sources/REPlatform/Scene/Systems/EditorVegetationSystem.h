#pragma once

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>

namespace DAVA
{
class Entity;
class VegetationRenderObject;
class EditorVegetationSystem : public SceneSystem
{
public:
    EditorVegetationSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    void GetActiveVegetation(Vector<VegetationRenderObject*>& activeVegetationObjects);

    void ReloadVegetation();

private:
    Vector<VegetationRenderObject*> vegetationObjects;
};
} // namespace DAVA
