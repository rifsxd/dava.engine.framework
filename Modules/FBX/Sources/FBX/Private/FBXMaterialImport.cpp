#include "FBXMaterialImport.h"

#include "Render/Material/NMaterial.h"

namespace DAVA
{
namespace FBXImporterDetails
{
namespace FBXMaterialImportDetails
{
Map<std::pair<const FbxSurfaceMaterial*, uint32>, NMaterial*> materialCache;
}

NMaterial* ImportMaterial(const FbxSurfaceMaterial* fbxMaterial, uint32 maxVertexInfluence)
{
    using namespace FBXMaterialImportDetails;

    auto found = materialCache.find(std::make_pair(fbxMaterial, maxVertexInfluence));
    if (found == materialCache.end())
    {
        NMaterial* material = new NMaterial();
        material->SetFXName(NMaterialName::TEXTURED_OPAQUE);

        if (maxVertexInfluence > 0)
        {
            if (maxVertexInfluence == 1)
                material->AddFlag(NMaterialFlagName::FLAG_HARD_SKINNING, 1);
            else
                material->AddFlag(NMaterialFlagName::FLAG_SOFT_SKINNING, maxVertexInfluence);
        }

        if (fbxMaterial != nullptr)
        {
            material->SetMaterialName(FastName(fbxMaterial->GetName()));

            Vector<std::pair<const char*, FastName>> texturesToImport = {
                { FbxSurfaceMaterial::sDiffuse, NMaterialTextureName::TEXTURE_ALBEDO },
                { FbxSurfaceMaterial::sNormalMap, NMaterialTextureName::TEXTURE_NORMAL }
            };

            for (auto& tex : texturesToImport)
            {
                const char* texturePath = GetFBXTexturePath(fbxMaterial->FindProperty(tex.first));
                if (texturePath)
                    material->AddTexture(tex.second, Texture::CreateFromFile(FilePath(texturePath)));
            }
        }
        else
        {
            material->SetMaterialName(FastName("UNNAMED"));
        }

        found = materialCache.emplace(std::make_pair(fbxMaterial, maxVertexInfluence), material).first;
    }

    return found->second;
}

void ClearMaterialCache()
{
    for (auto& it : FBXMaterialImportDetails::materialCache)
    {
        SafeRelease(it.second);
    }

    FBXMaterialImportDetails::materialCache.clear();
}

}; //ns Details
}; //ns DAVA