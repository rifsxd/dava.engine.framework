#include "REPlatform/Scene/Systems/EditorVegetationSystem.h"

#include <Debug/DVAssert.h>
#include <Entity/Component.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Highlevel/Vegetation/VegetationRenderObject.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
EditorVegetationSystem::EditorVegetationSystem(Scene* scene)
    : SceneSystem(scene)
{
}

void EditorVegetationSystem::AddEntity(Entity* entity)
{
    DVASSERT(HasComponent(entity, Type::Instance<RenderComponent>()));

    VegetationRenderObject* vro = GetVegetation(entity);
    if (vro != nullptr)
    {
        DVASSERT(std::find(vegetationObjects.begin(), vegetationObjects.end(), vro) == vegetationObjects.end());
        vegetationObjects.push_back(vro);
    }
}

void EditorVegetationSystem::RemoveEntity(Entity* entity)
{
    DVASSERT(DAVA::HasComponent(entity, Type::Instance<RenderComponent>()));

    VegetationRenderObject* vro = GetVegetation(entity);
    if (vro != nullptr)
    {
        DVASSERT(std::find(vegetationObjects.begin(), vegetationObjects.end(), vro) != vegetationObjects.end());
        FindAndRemoveExchangingWithLast(vegetationObjects, vro);
    }
}

void EditorVegetationSystem::PrepareForRemove()
{
    vegetationObjects.clear();
}

void EditorVegetationSystem::GetActiveVegetation(Vector<VegetationRenderObject*>& activeVegetationObjects)
{
    static const uint32 VISIBILITY_CRITERIA = RenderObject::VISIBLE | RenderObject::VISIBLE_QUALITY;

    for (VegetationRenderObject* ro : vegetationObjects)
    {
        if ((ro->GetFlags() & VISIBILITY_CRITERIA) == VISIBILITY_CRITERIA)
        {
            activeVegetationObjects.push_back(ro);
        }
    }
}

void EditorVegetationSystem::ReloadVegetation()
{
    for (VegetationRenderObject* ro : vegetationObjects)
    {
        ro->Rebuild();
    }
}
} // namespace DAVA
