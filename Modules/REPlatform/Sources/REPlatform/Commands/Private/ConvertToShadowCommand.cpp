#include "REPlatform/Commands/ConvertToShadowCommand.h"
#include "REPlatform/Scene/Systems/EditorMaterialSystem.h"

#include <Base/BaseObject.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/3D/MeshUtils.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
ConvertToShadowCommand::ConvertToShadowCommand(Entity* entity_, RenderBatch* batch)
    : RECommand("Convert To Shadow")
    , entity(SafeRetain(entity_))
    , oldBatch(batch)
    , newBatch(NULL)
{
    DVASSERT(entity);
    DVASSERT(oldBatch);

    renderObject = SafeRetain(oldBatch->GetRenderObject());
    DVASSERT(renderObject);

    oldBatch->Retain();

    newBatch = new RenderBatch();
    PolygonGroup* shadowPg = MeshUtils::CreateShadowPolygonGroup(oldBatch->GetPolygonGroup());
    shadowPg->BuildBuffers();
    newBatch->SetPolygonGroup(shadowPg);
    shadowPg->Release();

    ScopedPtr<NMaterial> shadowMaterialInst(new NMaterial());
    shadowMaterialInst->SetMaterialName(FastName("Shadow_Material_Instance"));

    newBatch->SetMaterial(shadowMaterialInst);

    Scene* scene = entity->GetScene();
    DVASSERT(scene != nullptr);
    EditorMaterialSystem* system = scene->GetSystem<EditorMaterialSystem>();
    const Set<NMaterial*> topLevelMaterials = system->GetTopParents();
    Set<NMaterial*>::iterator iter = std::find_if(topLevelMaterials.begin(), topLevelMaterials.end(), [](NMaterial* material) {
        DVASSERT(material->HasLocalFXName());
        return material->GetLocalFXName() == NMaterialName::SHADOW_VOLUME;
    });

    if (iter != topLevelMaterials.end())
    {
        shadowMaterialInst->SetParent(*iter);
    }
    else
    {
        ScopedPtr<NMaterial> shadowMaterial(new NMaterial());
        shadowMaterial->SetMaterialName(FastName("Shadow_Material"));
        shadowMaterial->SetFXName(NMaterialName::SHADOW_VOLUME);

        shadowMaterialInst->SetParent(shadowMaterial.get());
    }
}

ConvertToShadowCommand::~ConvertToShadowCommand()
{
    SafeRelease(oldBatch);
    SafeRelease(newBatch);
    SafeRelease(renderObject);
    SafeRelease(entity);
}

void ConvertToShadowCommand::Redo()
{
    renderObject->ReplaceRenderBatch(oldBatch, newBatch);
}

void ConvertToShadowCommand::Undo()
{
    renderObject->ReplaceRenderBatch(newBatch, oldBatch);
}

Entity* ConvertToShadowCommand::GetEntity() const
{
    return entity;
}

bool ConvertToShadowCommand::CanConvertBatchToShadow(RenderBatch* renderBatch)
{
    if (renderBatch && renderBatch->GetMaterial() && renderBatch->GetPolygonGroup())
    {
        return renderBatch->GetMaterial()->GetEffectiveFXName() != NMaterialName::SHADOW_VOLUME;
    }

    return false;
}

DAVA_VIRTUAL_REFLECTION_IMPL(ConvertToShadowCommand)
{
    ReflectionRegistrator<ConvertToShadowCommand>::Begin()
    .End();
}
} // namespace DAVA
