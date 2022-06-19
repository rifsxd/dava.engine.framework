#include "UI/Spine/Private/SpineSkeleton.h"

#include "UI/Spine/Private/SpineBone.h"
#include "UI/Spine/Private/SpineTrackEntry.h"

#include <Debug/DVAssert.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <FileSystem/LocalizationSystem.h>
#include <Logger/Logger.h>
#include <Render/2D/Systems/RenderSystem2D.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <Render/Texture.h>
#include <UI/UIControl.h>
#include <UI/UIControlSystem.h>

#include <spine/spine.h>

namespace DAVA
{
namespace SpinePrivate
{
static const uint32 MAX_VERTICES_PER_BATCH = 1024;
static const uint32 VERTICES_COMPONENTS_COUNT = 2;
static const uint32 TEXTURE_COMPONENTS_COUNT = 2;
static const uint32 COLOR_STRIDE = 1;
static const uint32 QUAD_VERTICES_COUNT = 4;
static const uint32 QUAD_TRIANGLES_COUNT = 6;
static const uint32 QUAD_VERTICES_LENGTH = QUAD_VERTICES_COUNT * VERTICES_COMPONENTS_COUNT;
static const uint16 QUAD_TRIANGLES[6] = { 0, 1, 2, 2, 3, 0 };

FilePath GetScaledName(const FilePath& path)
{
    VirtualCoordinatesSystem* vcs = Engine::Instance()->GetContext()->uiControlSystem->vcs;
    if (!path.IsEmpty() && vcs != nullptr)
    {
        String pathname;
        if (FilePath::PATH_IN_RESOURCES == path.GetType())
        {
            //as we can have several res folders we should work with 'FrameworkPath' instead of 'AbsolutePathname'
            pathname = path.GetFrameworkPath();
        }
        else
        {
            pathname = path.GetAbsolutePathname();
        }

        const String baseGfxFolderName = vcs->GetResourceFolder(vcs->GetBaseResourceIndex());
        String::size_type pos = pathname.find(baseGfxFolderName);
        if (String::npos != pos)
        {
            const String& desirableGfxFolderName = vcs->GetResourceFolder(vcs->GetDesirableResourceIndex());
            pathname.replace(pos, baseGfxFolderName.length(), desirableGfxFolderName);
            return FilePath(pathname);
        }
        else if (vcs->GetResourceFoldersCount() == 1 && baseGfxFolderName != "Gfx")
        { // magic for QE
            String::size_type startPos = pathname.find("/Gfx/");
            if (String::npos != startPos)
            {
                pathname.replace(startPos, 5, "/" + baseGfxFolderName + "/");
                return pathname;
            }
        }
    }
    return path;
}

FilePath GetLocalizedName(const FilePath& path)
{
    LocalizationSystem* locSys = Engine::Instance()->GetContext()->localizationSystem;
    if (!path.IsEmpty() && locSys != nullptr)
    {
        FilePath localizedPath(path);
        localizedPath.ReplaceDirectory((path.GetDirectory() + locSys->GetCurrentLocale()).MakeDirectoryPathname());
        return localizedPath;
    }
    return path;
}
}

SpineSkeleton::SpineSkeleton()
{
}

SpineSkeleton::~SpineSkeleton()
{
    ReleaseAtlas();
    ReleaseSkeleton();
}

void SpineSkeleton::ReleaseAtlas()
{
    if (atlas != nullptr)
    {
        spAtlas_dispose(atlas);
        atlas = nullptr;
    }

    if (currentTexture)
    {
        currentTexture = nullptr;
    }

    ClearData();
}

void SpineSkeleton::ReleaseSkeleton()
{
    if (skeleton != nullptr)
    {
        spSkeletonData_dispose(skeleton->data);
        spSkeleton_dispose(skeleton);
        skeleton = nullptr;
    }

    if (state != nullptr)
    {
        spAnimationStateData_dispose(state->data);
        spAnimationState_dispose(state);
        state = nullptr;
    }

    worldVertices.clear();
    animationsNames.clear();
    skinsNames.clear();

    ClearData();
}

bool SpineSkeleton::Load(const FilePath& dataPath_, const FilePath& atlasPath_)
{
    ReleaseAtlas();
    ReleaseSkeleton();

    if (dataPath_.IsEmpty() || atlasPath_.IsEmpty())
    {
        return false;
    }

    FilePath atlasPath(atlasPath_);
    FilePath dataPath(dataPath_);

    // Check scaled resource (only atlas)
    FilePath scaledPath = SpinePrivate::GetScaledName(atlasPath);
    VirtualCoordinatesSystem* vcs = Engine::Instance()->GetContext()->uiControlSystem->vcs;
    if (vcs->GetResourceFoldersCount() == 1 || scaledPath.Exists()) // magic for QE: we don't need try generates same file path twice for situations with only one Gfx2 folder (without Gfx)
    {
        atlasPath = scaledPath;
    }

    // Check localized resource (atlas and data)
    FilePath localizedPath = SpinePrivate::GetLocalizedName(atlasPath);
    if (localizedPath.Exists())
    {
        atlasPath = localizedPath;
    }
    localizedPath = SpinePrivate::GetLocalizedName(dataPath);
    if (localizedPath.Exists())
    {
        dataPath = localizedPath;
    }

    if (!dataPath.Exists())
    {
        Logger::Error("[SpineSkeleton::Load] Data file `%s` doesn't exist!", dataPath.GetStringValue().c_str());
        return false;
    }

    if (!atlasPath.Exists())
    {
        Logger::Error("[SpineSkeleton::Load] Atlas file `%s` doesn't exist!", atlasPath.GetStringValue().c_str());
        return false;
    }

    atlas = spAtlas_createFromFile(atlasPath.GetAbsolutePathname().c_str(), 0);
    if (atlas == nullptr)
    {
        Logger::Error("[SpineSkeleton::Load] Error reading atlas file!");
        return false;
    }
    currentTexture = reinterpret_cast<Texture*>(atlas->pages[0].rendererObject);

    spSkeletonData* skeletonData = nullptr;
    String dataLoadingError = "Error reading skeleton data file!";

    if (dataPath.IsEqualToExtension(".json"))
    {
        spSkeletonJson* json = spSkeletonJson_create(atlas);
        if (json != nullptr)
        {
            json->scale = 1.f;
            try
            {
                skeletonData = spSkeletonJson_readSkeletonDataFile(json, dataPath.GetAbsolutePathname().c_str());
                if (json->error)
                {
                    dataLoadingError = json->error;
                }
            }
            catch (...)
            {
                if (json->attachmentLoader && json->attachmentLoader->error1 != nullptr)
                {
                    dataLoadingError = String(json->attachmentLoader->error1);
                    if (json->attachmentLoader->error2 != nullptr)
                    {
                        dataLoadingError += String(json->attachmentLoader->error2);
                    }
                }
                else
                {
                    // Skip Spine internal error while parsing data file
                    dataLoadingError = "Internal unhandled error while parsing data file!";
                }
            }
            spSkeletonJson_dispose(json);
        }
    }
    else if (dataPath.IsEqualToExtension(".skel"))
    {
        spSkeletonBinary* binary = spSkeletonBinary_create(atlas);
        if (binary != nullptr)
        {
            binary->scale = 1.f;
            try
            {
                skeletonData = spSkeletonBinary_readSkeletonDataFile(binary, dataPath.GetAbsolutePathname().c_str());
                if (binary->error)
                {
                    dataLoadingError = binary->error;
                }
            }
            catch (...)
            {
                if (binary->attachmentLoader && binary->attachmentLoader->error1 != nullptr)
                {
                    dataLoadingError = String(binary->attachmentLoader->error1);
                    if (binary->attachmentLoader->error2 != nullptr)
                    {
                        dataLoadingError += String(binary->attachmentLoader->error2);
                    }
                }
                else
                {
                    // Skip Spine internal error while parsing data file
                    dataLoadingError = "Internal unhandled error while parsing data file!";
                }
            }
            spSkeletonBinary_dispose(binary);
        }
    }

    if (skeletonData == nullptr)
    {
        Logger::Error("[SpineSkeleton::Load] %s", dataLoadingError.c_str());
        ReleaseAtlas();
        return false;
    }

    skeleton = spSkeleton_create(skeletonData);
    if (skeleton == nullptr)
    {
        Logger::Error("[SpineSkeleton::Load] Create skeleton failure!");
        ReleaseAtlas();
        return false;
    }

    state = spAnimationState_create(spAnimationStateData_create(skeleton->data));
    if (state == nullptr)
    {
        Logger::Error("[SpineSkeleton::Load] %s", "Error creating animation state!");
        ReleaseAtlas();
        ReleaseSkeleton();
        return false;
    }

    state->listener = [](spAnimationState* state, int32 trackIndex, spEventType type, spEvent* event, int32 loopCount) {
        switch (type)
        {
        case SP_ANIMATION_START:
            reinterpret_cast<SpineSkeleton*>(state->rendererObject)->onStart.Emit(trackIndex);
            break;
        case SP_ANIMATION_END:
            reinterpret_cast<SpineSkeleton*>(state->rendererObject)->onFinish.Emit(trackIndex);
            break;
        case SP_ANIMATION_COMPLETE:
            reinterpret_cast<SpineSkeleton*>(state->rendererObject)->onComplete.Emit(trackIndex);
            break;
        case SP_ANIMATION_EVENT:
            reinterpret_cast<SpineSkeleton*>(state->rendererObject)->onEvent.Emit(trackIndex, (event && event->stringValue) ? String(event->stringValue) : "");
            break;
        }
    };
    state->rendererObject = this;

    animationsNames.clear();
    int32 animCount = skeleton->data->animationsCount;
    for (int32 i = 0; i < animCount; ++i)
    {
        spAnimation* anim = skeleton->data->animations[i];
        animationsNames.push_back(String(anim->name));
    }

    skinsNames.clear();
    int32 skinsCount = skeleton->data->skinsCount;
    for (int32 i = 0; i < skinsCount; ++i)
    {
        spSkin* skin = skeleton->data->skins[i];
        skinsNames.push_back(String(skin->name));
    }

    // Init initial state
    spSkeleton_setToSetupPose(skeleton);
    spSkeleton_update(skeleton, 0);
    spAnimationState_update(state, 0);
    spAnimationState_apply(state, skeleton);
    spSkeleton_updateWorldTransform(skeleton);

    return true;
}

void SpineSkeleton::PushBatch()
{
    uint32 vCount = static_cast<uint32>(verticesCoords.size()) - currentVerticesStart;
    uint32 iCount = static_cast<uint32>(verticesIndices.size()) - currentIndicesStart;
    if (vCount > 0 && iCount > 0)
    {
        float32* vCoords = reinterpret_cast<float32*>(verticesCoords.data() + currentVerticesStart);
        float32* uvCoords = reinterpret_cast<float32*>(verticesUVs.data() + currentVerticesStart);
        uint32* colors = verticesColors.data() + currentVerticesStart;
        uint16* indices = verticesIndices.data() + currentIndicesStart;

        currentVerticesStart += vCount;
        currentIndicesStart += iCount;

        BatchDescriptor2D batch;
        batch.material = RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL;
        batch.vertexCount = vCount;
        batch.indexCount = iCount;
        batch.vertexStride = SpinePrivate::VERTICES_COMPONENTS_COUNT;
        batch.texCoordStride = SpinePrivate::TEXTURE_COMPONENTS_COUNT;
        batch.colorStride = SpinePrivate::COLOR_STRIDE;
        batch.vertexPointer = vCoords;
        batch.texCoordPointer[0] = uvCoords;
        batch.colorPointer = colors;
        batch.indexPointer = indices;
        batch.textureSetHandle = currentTexture->singleTextureSet;
        batch.samplerStateHandle = currentTexture->samplerStateHandle;

        batchDescriptors.push_back(std::move(batch));
    }
}

void SpineSkeleton::ClearData()
{
    batchDescriptors.clear();
    verticesCoords.clear();
    verticesUVs.clear();
    verticesColors.clear();
    verticesIndices.clear();
    currentVerticesStart = 0;
    currentIndicesStart = 0;
}

void SpineSkeleton::SwitchTexture(Texture* texture)
{
    if (texture != currentTexture)
    {
        PushBatch();
        currentTexture = texture;
    }
}

void SpineSkeleton::Update(const float32 timeElapsed)
{
    if (!skeleton || !state)
        return;

    spSkeleton_update(skeleton, timeElapsed * timeScale);
    spAnimationState_update(state, timeElapsed * timeScale);
    spAnimationState_apply(state, skeleton);
    spSkeleton_updateWorldTransform(skeleton);

    // Draw
    ClearData();

    if (!skeleton || !state || !currentTexture)
        return;

    for (int32 i = 0, n = skeleton->slotsCount; i < n; i++)
    {
        spSlot* slot = skeleton->drawOrder[i];
        Color color(slot->r, slot->g, slot->b, slot->a);
        if (slot->attachment)
        {
            const float32* uvs = nullptr;
            const uint16* indices = nullptr;
            uint32 verticesCount = 0;
            uint32 indicesCount = 0;

            switch (slot->attachment->type)
            {
            case SP_ATTACHMENT_REGION:
            {
                spRegionAttachment* attachment = reinterpret_cast<spRegionAttachment*>(slot->attachment);

                if (worldVertices.size() < SpinePrivate::QUAD_VERTICES_LENGTH)
                {
                    worldVertices.resize(SpinePrivate::QUAD_VERTICES_LENGTH);
                }
                spRegionAttachment_computeWorldVertices(attachment, slot->bone, worldVertices.data());

                SwitchTexture(reinterpret_cast<Texture*>(reinterpret_cast<spAtlasRegion*>(attachment->rendererObject)->page->rendererObject));

                uvs = attachment->uvs;
                verticesCount = SpinePrivate::QUAD_VERTICES_COUNT;
                indices = SpinePrivate::QUAD_TRIANGLES;
                indicesCount = SpinePrivate::QUAD_TRIANGLES_COUNT;
                break;
            }
            case SP_ATTACHMENT_MESH:
            {
                spMeshAttachment* attachment = reinterpret_cast<spMeshAttachment*>(slot->attachment);

                if (worldVertices.size() < static_cast<uint32>(attachment->super.worldVerticesLength))
                {
                    worldVertices.resize(static_cast<uint32>(attachment->super.worldVerticesLength));
                }
                spMeshAttachment_computeWorldVertices(attachment, slot, worldVertices.data());

                SwitchTexture(reinterpret_cast<Texture*>(reinterpret_cast<spAtlasRegion*>(attachment->rendererObject)->page->rendererObject));

                uvs = attachment->uvs;
                verticesCount = static_cast<uint32>(attachment->super.worldVerticesLength / 2);
                indices = attachment->triangles;
                indicesCount = static_cast<uint32>(attachment->trianglesCount);
                break;
            }
            default:
                // Skip other attachment types
                break;
            }

            // TODO: mix colors from control
            // TODO: store slot color for bone

            if ((verticesCoords.size() - currentVerticesStart + verticesCount) > SpinePrivate::MAX_VERTICES_PER_BATCH)
            {
                PushBatch();
            }

            DVASSERT((verticesCoords.size() - currentVerticesStart) < std::numeric_limits<uint16>::max());
            uint16 startIndex = static_cast<uint16>(verticesCoords.size() - currentVerticesStart);
            for (uint32 i = 0; i < verticesCount; ++i)
            {
                Vector2 point(worldVertices[i * SpinePrivate::VERTICES_COMPONENTS_COUNT], -worldVertices[i * SpinePrivate::VERTICES_COMPONENTS_COUNT + 1]);
                Vector2 uv(uvs[i * SpinePrivate::VERTICES_COMPONENTS_COUNT], uvs[i * SpinePrivate::VERTICES_COMPONENTS_COUNT + 1]);

                verticesCoords.push_back(point);
                verticesUVs.push_back(uv);
                verticesColors.push_back(color.GetRGBA());
            }
            for (uint32 i = 0; i < indicesCount; i++)
            {
                verticesIndices.push_back(startIndex + indices[i]);
            }
        }
    }
    PushBatch();
}

void SpineSkeleton::ResetSkeleton()
{
    if (skeleton)
    {
        spSkeleton_setToSetupPose(skeleton);
    }
}

void SpineSkeleton::SetOriginOffset(const Vector2& offset)
{
    if (skeleton)
    {
        skeleton->x = offset.x;
        skeleton->y = -offset.y;
    }
}

Vector2 SpineSkeleton::GetSkeletonOriginOffset() const
{
    if (skeleton)
    {
        return Vector2(skeleton->x, -skeleton->y);
    }
    return Vector2();
}

const Vector<BatchDescriptor2D>& SpineSkeleton::GetRenderBatches() const
{
    return batchDescriptors;
}

const Vector<String>& SpineSkeleton::GetAvailableAnimationsNames() const
{
    return animationsNames;
}

std::shared_ptr<SpineTrackEntry> SpineSkeleton::SetAnimation(int32 trackIndex, const String& name, bool loop)
{
    if (skeleton != nullptr && state != nullptr)
    {
        spAnimation* animation = spSkeletonData_findAnimation(skeleton->data, name.c_str());
        if (animation)
        {
            spTrackEntry* entry = spAnimationState_setAnimation(state, trackIndex, animation, loop);
            if (entry)
            {
                return std::make_shared<SpineTrackEntry>(entry);
            }
            else
            {
                Logger::Error("[SpineSkeleton] Animation '%s' was not setted!", name.c_str());
            }
        }
        else
        {
            Logger::Error("[SpineSkeleton] Animation '%s' was not found!", name.c_str());
        }
    }
    return nullptr;
}

std::shared_ptr<SpineTrackEntry> SpineSkeleton::AddAnimation(int32 trackIndex, const String& name, bool loop, float32 delay)
{
    if (skeleton != nullptr && state != nullptr)
    {
        spAnimation* animation = spSkeletonData_findAnimation(skeleton->data, name.c_str());
        if (animation)
        {
            spTrackEntry* entry = spAnimationState_addAnimation(state, trackIndex, animation, loop, delay);
            if (entry)
            {
                return std::make_shared<SpineTrackEntry>(entry);
            }
            else
            {
                Logger::Error("[SpineSkeleton] Animation '%s' was not added!", name.c_str());
            }
        }
        else
        {
            Logger::Error("[SpineSkeleton] Animation '%s' was not found!", name.c_str());
        }
    }
    return nullptr;
}

std::shared_ptr<SpineTrackEntry> SpineSkeleton::GetTrack(int32 trackIndex)
{
    if (state != nullptr)
    {
        spTrackEntry* entry = spAnimationState_getCurrent(state, trackIndex);
        if (entry)
        {
            return std::make_shared<SpineTrackEntry>(entry);
        }
    }
    return nullptr;
}

void SpineSkeleton::SetAnimationMix(const String& fromAnimation, const String& toAnimation, float32 duration)
{
    if (state != nullptr)
    {
        spAnimationStateData_setMixByName(state->data, fromAnimation.c_str(), toAnimation.c_str(), duration);
    }
}

void SpineSkeleton::ClearTracks()
{
    if (state != nullptr)
    {
        spAnimationState_clearTracks(state);
    }
}

void SpineSkeleton::CleatTrack(int32 trackIndex)
{
    if (state != nullptr)
    {
        spAnimationState_clearTrack(state, trackIndex);
    }
}

void SpineSkeleton::SetTimeScale(float32 timeScale_)
{
    timeScale = timeScale_;
}

float32 SpineSkeleton::GetTimeScale() const
{
    return timeScale;
}

bool SpineSkeleton::SetSkin(const String& skinName)
{
    if (skeleton != nullptr)
    {
        int32 skin = spSkeleton_setSkinByName(skeleton, skinName.empty() ? nullptr : skinName.c_str());
        return skin != 0;
    }
    return false;
}

String SpineSkeleton::GetSkinName() const
{
    if (skeleton != nullptr && skeleton->skin != nullptr)
    {
        return String(skeleton->skin->name);
    }
    return String();
}

const Vector<String>& SpineSkeleton::GetAvailableSkinsNames() const
{
    return skinsNames;
}

std::shared_ptr<SpineBone> SpineSkeleton::FindBone(const String& boneName)
{
    if (skeleton)
    {
        spBone* bone = spSkeleton_findBone(skeleton, boneName.c_str());
        if (bone)
        {
            return std::make_shared<SpineBone>(bone);
        }
    }
    return nullptr;
}
}
