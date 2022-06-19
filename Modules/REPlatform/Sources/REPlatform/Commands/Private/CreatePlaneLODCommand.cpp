#include "REPlatform/Commands/CreatePlaneLODCommand.h"
#include "REPlatform/Scene/SceneHelper.h"
#include "REPlatform/Scene/Utils/RETextureDescriptorUtils.h"

#include <Debug/DVAssert.h>
#include <FileSystem/FileSystem.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Image/ImageSystem.h>
#include <Render/Material/NMaterial.h>
#include <Render/Material/NMaterialNames.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Lod/LodComponent.h>

namespace DAVA
{
CreatePlaneLODCommand::CreatePlaneLODCommand(const CreatePlaneLODCommandHelper::RequestPointer& request_)
    : RECommand("Create Plane LOD")
    , request(request_)
{
    DVASSERT(GetRenderObject(GetEntity()));
}

void CreatePlaneLODCommand::Redo()
{
    CreateTextureFiles();

    ScopedPtr<Texture> fileTexture(Texture::CreateFromFile(request->texturePath));
    NMaterial* material = request->planeBatch->GetMaterial();
    if (material != nullptr)
    {
        if (material->HasLocalTexture(NMaterialTextureName::TEXTURE_ALBEDO))
        {
            material->SetTexture(NMaterialTextureName::TEXTURE_ALBEDO, fileTexture);
        }
        else
        {
            material->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, fileTexture);
        }
        fileTexture->Reload();
    }

    Entity* entity = GetEntity();
    RenderObject* renderObject = GetRenderObject(entity);
    renderObject->AddRenderBatch(request->planeBatch, request->newLodIndex, -1);
}

void CreatePlaneLODCommand::Undo()
{
    RenderObject* ro = GetRenderObject(GetEntity());

    //restore batches
    ro->RemoveRenderBatch(request->planeBatch);
}

Entity* CreatePlaneLODCommand::GetEntity() const
{
    return request->lodComponent->GetEntity();
}

void CreatePlaneLODCommand::CreateTextureFiles()
{
    DVASSERT(request->planeImage);

    FilePath folder = request->texturePath.GetDirectory();
    FileSystem::Instance()->CreateDirectory(folder, true);
    ImageSystem::Save(request->texturePath, request->planeImage);
    RETextureDescriptorUtils::CreateOrUpdateDescriptor(request->texturePath);
}

RenderBatch* CreatePlaneLODCommand::GetRenderBatch() const
{
    return request->planeBatch;
}

DAVA_VIRTUAL_REFLECTION_IMPL(CreatePlaneLODCommand)
{
    ReflectionRegistrator<CreatePlaneLODCommand>::Begin()
    .End();
}
} // namespace DAVA
