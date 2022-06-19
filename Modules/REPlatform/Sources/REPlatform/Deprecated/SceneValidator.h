#pragma once

#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <FileSystem/FilePath.h>
#include <Particles/ParticleEmitterInstance.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/RenderBase.h>
#include <Render/Texture.h>
#include <Render/TextureDescriptor.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
class SceneValidator
{
public:
    /*
     \brief Function to validate Scene errors
     \param[in] scene scene for validation
     \param[out] errorsLog set for validation errors
	 */
    void ValidateScene(Scene* scene, const FilePath& scenePath);

    /*
     \brief Function to find Scales in models transformations
     \param[in] scene scene for validation
     \param[out] errorsLog set for validation errors
	 */
    void ValidateScales(Scene* scene);

    /*
     \brief Function to validate Texture errors
     \param[in] texture texture for validation
     \param[out] errorsLog set for validation errors
	 */

    void ValidateTexture(Texture* texture, const String& validatedObjectName);

    /*
     \brief Function to validate LandscapeNode errors
     \param[in] landscape landscape for validation
     \param[out] errorsLog set for validation errors
	 */
    void ValidateLandscape(Landscape* landscape);

    /*
     \brief Function to validate Entity errors
     \param[in] sceneNode sceneNode for validation
     \param[out] errorsLog set for validation errors
	 */
    void ValidateSceneNode(Entity* sceneNode);

    /*
     \brief Function to validate Materials errors
     \param[in] scene that has materials for validation
     \param[out] errorsLog set for validation errors
	 */
    void ValidateMaterials(Scene* scene);

    /*
     \brief Function sets 3d folder path for checking texture pathnames
     \param[in] pathname path to DataSource/3d folder
     \return old path for checking
	 */

    void ValidateNodeCustomProperties(Entity* sceneNode);

    FilePath SetPathForChecking(const FilePath& pathname);

    void EnumerateNodes(Scene* scene);

    static bool IsTextureChanged(const TextureDescriptor* descriptor, eGPUFamily forGPU);
    static bool IsTextureChanged(const FilePath& texturePathname, eGPUFamily forGPU);

    bool ValidateTexturePathname(const FilePath& pathForValidation);
    bool ValidateHeightmapPathname(const FilePath& pathForValidation);

    bool IsPathCorrectForProject(const FilePath& pathname);

    DAVA_DEPRECATED(static void FindSwitchesWithDifferentLODs(Entity* entity, Set<FastName>& names));
    DAVA_DEPRECATED(static bool IsEntityHasDifferentLODsCount(Entity* entity));
    DAVA_DEPRECATED(static bool IsObjectHasDifferentLODsCount(RenderObject* renderObject));

    static void ExtractEmptyRenderObjects(Entity* entity);

protected:
    void ValidateRenderComponent(Entity* ownerNode);
    void ValidateRenderBatch(Entity* ownerNode, RenderBatch* renderBatch);

    void ValidateParticleEffectComponent(Entity* ownerNode) const;
    void ValidateParticleEmitter(ParticleEmitterInstance* emitter, Entity* owner) const;

    void ValidateLandscapeTexture(Landscape* landscape, const FastName& texLevel);
    void ValidateCustomColorsTexture(Entity* landscapeEntity);

    void FixIdentityTransform(Entity* ownerNode, const String& errorMessage);

    bool ValidateColor(Color& color);

    int32 EnumerateSceneNodes(Entity* node);

    void ValidateScalesInternal(Entity* sceneNode);

    bool ValidatePathname(const FilePath& pathForValidation, const String& validatedObjectName);

    bool NodeRemovingDisabled(Entity* node);

    bool WasTextureChanged(Texture* texture, eGPUFamily forGPU);

    bool IsTextureDescriptorPath(const FilePath& path);

    bool IsFBOTexture(Texture* texture);

    VariantType* GetCustomPropertyFromParentsTree(Entity* ownerNode, const String& key);

    Set<Entity*> emptyNodesForDeletion;

    FilePath pathForChecking;
    String sceneName;
};
} // namespace DAVA
