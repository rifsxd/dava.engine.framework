#include "REPlatform/Scene/Systems/CustomColorsSystem.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/CustomColorsProxy.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Systems/LandscapeProxy.h"
#include "REPlatform/Scene/Systems/ModifSystem.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"
#include "REPlatform/Scene/Utils/Utils.h"

#include "REPlatform/Commands/CustomColorsCommands2.h"
#include "REPlatform/Commands/KeyedArchiveCommand.h"
#include "REPlatform/DataNodes/ProjectManagerData.h"
#include "REplatform/Deprecated/EditorConfig.h"
#include "REPlatform/Global/StringConstants.h"

#include <TArc/Core/Deprecated.h>

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <FileSystem/FileSystem.h>
#include <Render/Renderer.h>
#include <Render/RHI/rhi_Type.h>

namespace DAVA
{
CustomColorsSystem::CustomColorsSystem(Scene* scene)
    : LandscapeEditorSystem(scene, DefaultCursorPath())
{
    SetColor(colorIndex);
}

CustomColorsSystem::~CustomColorsSystem()
{
    SafeRelease(toolImageTexture);
    SafeRelease(loadedTexture);
}

LandscapeEditorDrawSystem::eErrorType CustomColorsSystem::EnableLandscapeEditing()
{
    if (enabled)
    {
        return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
    }

    LandscapeEditorDrawSystem::eErrorType canBeEnabledError = IsCanBeEnabled();
    if (canBeEnabledError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return canBeEnabledError;
    }

    LandscapeEditorDrawSystem::eErrorType enableCustomDrawError = drawSystem->EnableCustomDraw();
    if (enableCustomDrawError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return enableCustomDrawError;
    }

    bool inputLocked = AcquireInputLock(GetScene());
    DVASSERT(inputLocked == true);
    Scene* scene = GetScene();
    scene->GetSystem<SelectionSystem>()->SetLocked(true);
    landscapeSize = Landscape::CUSTOM_COLOR_TEXTURE_SIZE;

    FilePath filePath = GetCurrentSaveFileName();
    DVASSERT(!filePath.IsEmpty());
    if (Engine::Instance()->GetContext()->fileSystem->Exists(filePath))
    {
        const bool isTextureLoaded = LoadTexture(filePath, false);
        drawSystem->GetCustomColorsProxy()->ResetLoadedState(isTextureLoaded);
    }
    else
    {
        drawSystem->GetCustomColorsProxy()->UpdateSpriteFromConfig();
    }

    drawSystem->EnableCursor();
    drawSystem->SetCursorTexture(cursorTexture);
    SetBrushSize(curToolSize);

    Texture* customColorsTexture = drawSystem->GetCustomColorsProxy()->GetTexture();
    drawSystem->GetLandscapeProxy()->SetToolTexture(customColorsTexture, true);

    if (!toolImageTexture)
    {
        CreateToolImage("~res:/ResourceEditor/LandscapeEditor/Tools/customcolorsbrush/circle.png");
    }

    enabled = true;
    return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool CustomColorsSystem::ChangesPresent()
{
    if (drawSystem && drawSystem->GetCustomColorsProxy())
    {
        return drawSystem->GetCustomColorsProxy()->GetChangesCount() > 0;
    }
    return false;
}

bool CustomColorsSystem::DisableLandscapeEdititing(bool saveNeeded)
{
    if (!enabled)
    {
        return true;
    }

    if (drawSystem->GetCustomColorsProxy()->GetChangesCount() && saveNeeded)
    {
        SaveTexture();
    }
    FinishEditing(false);

    ReleaseInputLock(GetScene());
    Scene* scene = GetScene();
    scene->GetSystem<SelectionSystem>()->SetLocked(false);

    drawSystem->DisableCursor();
    drawSystem->DisableCustomDraw();

    drawSystem->GetLandscapeProxy()->SetToolTexture(nullptr, true);
    enabled = false;

    SafeRelease(toolImageTexture);
    SafeRelease(loadedTexture);

    return !enabled;
}

void CustomColorsSystem::Process(float32 timeElapsed)
{
    if (!IsLandscapeEditingEnabled())
    {
        return;
    }

    if (editingIsEnabled && isIntersectsLandscape)
    {
        if (prevCursorPos != cursorPosition)
        {
            UpdateBrushTool();
            prevCursorPos = cursorPosition;
        }
    }
}

bool CustomColorsSystem::Input(UIEvent* event)
{
    if (!IsLandscapeEditingEnabled())
    {
        return false;
    }

    UpdateCursorPosition();

    if (event->mouseButton == eMouseButtons::LEFT)
    {
        Vector3 point;

        switch (event->phase)
        {
        case UIEvent::Phase::BEGAN:
            if (isIntersectsLandscape)
            {
                UpdateToolImage();
                StoreOriginalState();
                editingIsEnabled = true;
            }
            break;

        case UIEvent::Phase::DRAG:
            break;

        case UIEvent::Phase::ENDED:
            FinishEditing(true);
            break;

        default:
            break;
        }
    }
    return false;
}

void CustomColorsSystem::InputCancelled(UIEvent* event)
{
    if (IsLandscapeEditingEnabled() && (event->mouseButton == eMouseButtons::LEFT))
    {
        FinishEditing(true);
    }
}

void CustomColorsSystem::FinishEditing(bool applyModification)
{
    if (editingIsEnabled)
    {
        if (applyModification)
        {
            Rect updatedRect = GetUpdatedRect();
            if (updatedRect.dx > 0 && updatedRect.dy > 0)
            {
                SceneEditor2* scene = dynamic_cast<SceneEditor2*>(GetScene());
                DVASSERT(scene);

                ScopedPtr<Image> image(drawSystem->GetCustomColorsProxy()->GetTexture()->CreateImageFromMemory());
                scene->Exec(std::unique_ptr<Command>(new ModifyCustomColorsCommand(originalImage, image, drawSystem->GetCustomColorsProxy(), updatedRect, false)));
            }
        }
        SafeRelease(originalImage);
        editingIsEnabled = false;
    }
}

void CustomColorsSystem::UpdateToolImage(bool force)
{
}

void CustomColorsSystem::CreateToolImage(const FilePath& filePath)
{
    Texture* toolTexture = CreateSingleMipTexture(filePath);
    if (!toolTexture)
    {
        return;
    }

    SafeRelease(toolImageTexture);

    toolImageTexture = toolTexture;
    toolImageTexture->SetMinMagFilter(rhi::TEXFILTER_NEAREST, rhi::TEXFILTER_NEAREST, rhi::TEXMIPFILTER_NONE);
}

void CustomColorsSystem::UpdateBrushTool()
{
    Texture* colorTexture = drawSystem->GetCustomColorsProxy()->GetTexture();

    Vector2 spriteSize = Vector2(cursorSize, cursorSize) * landscapeSize;
    Vector2 spritePos = cursorPosition * landscapeSize - spriteSize / 2.f;

    Rect updatedRect;
    updatedRect.SetPosition(spritePos);
    updatedRect.SetSize(spriteSize);
    AddRectToAccumulator(updatedRect);

    auto brushMaterial = drawSystem->GetCustomColorsProxy()->GetBrushMaterial();
    RenderSystem2D::RenderTargetPassDescriptor desc;
    desc.priority = PRIORITY_SERVICE_2D;
    desc.colorAttachment = colorTexture->handle;
    desc.depthAttachment = colorTexture->handleDepthStencil;
    desc.width = colorTexture->GetWidth();
    desc.height = colorTexture->GetHeight();
    desc.clearTarget = false;
    desc.transformVirtualToPhysical = false;
    RenderSystem2D::Instance()->BeginRenderTargetPass(desc);
    RenderSystem2D::Instance()->DrawTexture(toolImageTexture, brushMaterial, drawColor, updatedRect);
    RenderSystem2D::Instance()->EndRenderTargetPass();
}

void CustomColorsSystem::ResetAccumulatorRect()
{
    float32 inf = std::numeric_limits<float32>::infinity();
    updatedRectAccumulator = Rect(inf, inf, -inf, -inf);
}

void CustomColorsSystem::AddRectToAccumulator(const Rect& rect)
{
    updatedRectAccumulator = updatedRectAccumulator.Combine(rect);
}

Rect CustomColorsSystem::GetUpdatedRect()
{
    Rect r = updatedRectAccumulator;
    drawSystem->ClampToTexture(LandscapeProxy::LANDSCAPE_TEXTURE_TOOL, r);

    return r;
}

void CustomColorsSystem::SetBrushSize(int32 brushSize, bool updateDrawSystem /*= true*/)
{
    if (brushSize > 0)
    {
        curToolSize = brushSize;
        cursorSize = static_cast<float32>(brushSize) / landscapeSize;
        if (updateDrawSystem)
        {
            drawSystem->SetCursorSize(cursorSize);
        }
    }
}

void CustomColorsSystem::SetColor(int32 colorIndex)
{
    ProjectManagerData* data = Deprecated::GetDataNode<ProjectManagerData>();
    if (data == nullptr)
    {
        return;
    }

    if (data->GetEditorConfig() != nullptr)
    {
        Vector<Color> customColors = data->GetEditorConfig()->GetColorPropertyValues("LandscapeCustomColors");
        if (colorIndex >= 0 && colorIndex < static_cast<int32>(customColors.size()))
        {
            drawColor = customColors[colorIndex];
            this->colorIndex = colorIndex;
        }
    }
}

void CustomColorsSystem::StoreOriginalState()
{
    DVASSERT(originalImage == NULL);
    originalImage = drawSystem->GetCustomColorsProxy()->GetTexture()->CreateImageFromMemory();
    ResetAccumulatorRect();
}

void CustomColorsSystem::SaveTexture(const FilePath& filePath)
{
    if (filePath.IsEmpty())
        return;

    Texture* customColorsTexture = drawSystem->GetCustomColorsProxy()->GetTexture();

    rhi::HSyncObject frameSyncObject = rhi::GetCurrentFrameSyncObject();
    Renderer::RegisterSyncCallback(rhi::GetCurrentFrameSyncObject(), [this, customColorsTexture, filePath, frameSyncObject](rhi::HSyncObject syncObject)
                                   {
                                       if (frameSyncObject != syncObject)
                                           return;

                                       Image* image = customColorsTexture->CreateImageFromMemory();
                                       ImageSystem::Save(filePath, image);
                                       SafeRelease(image);

                                       StoreSaveFileName(filePath);
                                       drawSystem->GetCustomColorsProxy()->ResetChanges();
                                   });
}

void CustomColorsSystem::SaveTexture()
{
    SaveTexture(GetCurrentSaveFileName());
}

bool CustomColorsSystem::LoadTexture(const FilePath& filePath, bool createUndo)
{
    if (filePath.IsEmpty())
        return false;

    Vector<Image*> images;
    ImageSystem::Load(filePath, images);
    if (images.empty())
        return false;

    Image* image = images.front();
    if (CouldApplyImage(image, filePath.GetFilename()))
    {
        AddRectToAccumulator(Rect(Vector2(0.f, 0.f), Vector2(image->GetWidth(), image->GetHeight())));

        if (createUndo)
        {
            DVASSERT(originalImage == nullptr);
            originalImage = drawSystem->GetCustomColorsProxy()->GetTexture()->CreateImageFromMemory();

            SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());

            scene->BeginBatch("Load custom colors texture", 2);
            StoreSaveFileName(filePath);
            scene->Exec(std::unique_ptr<Command>(new ModifyCustomColorsCommand(originalImage, image, drawSystem->GetCustomColorsProxy(), GetUpdatedRect(), true)));
            scene->EndBatch();

            SafeRelease(originalImage);
        }
        else
        {
            SafeRelease(loadedTexture);
            loadedTexture = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight(), false);

            Texture* target = drawSystem->GetCustomColorsProxy()->GetTexture();

            RenderSystem2D::RenderTargetPassDescriptor desc;
            desc.priority = PRIORITY_SERVICE_2D;
            desc.colorAttachment = target->handle;
            desc.depthAttachment = target->handleDepthStencil;
            desc.width = target->GetWidth();
            desc.height = target->GetHeight();
            desc.clearTarget = false;
            desc.transformVirtualToPhysical = false;
            RenderSystem2D::Instance()->BeginRenderTargetPass(desc);
            RenderSystem2D::Instance()->DrawTexture(loadedTexture, RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL, Color::White);
            RenderSystem2D::Instance()->EndRenderTargetPass();
        }
    }

    for_each(images.begin(), images.end(), SafeRelease<Image>);
    return true;
}

bool CustomColorsSystem::CouldApplyImage(Image* image, const String& imageName) const
{
    if (image == nullptr)
    {
        return false;
    }

    if (image->GetPixelFormat() != FORMAT_RGBA8888)
    {
        Logger::Error("[CustomColorsSystem] %s has wrong format (%s). We need RGBA888", imageName.c_str(), GlobalEnumMap<PixelFormat>::Instance()->ToString(image->GetPixelFormat()));
        return false;
    }

    const Texture* oldTexture = drawSystem->GetCustomColorsProxy()->GetTexture();
    if (oldTexture != nullptr)
    {
        const Size2i imageSize(image->GetWidth(), image->GetHeight());
        const Size2i textureSize(oldTexture->GetWidth(), oldTexture->GetHeight());

        if (imageSize != textureSize)
        {
            Logger::Error("[CustomColorsSystem] %s has wrong size (%d x %d). We need (%d x %d)", imageName.c_str(), imageSize.dx, imageSize.dy, textureSize.dx, textureSize.dy);
            return false;
        }
    }

    return true;
}

void CustomColorsSystem::StoreSaveFileName(const FilePath& filePath)
{
    std::unique_ptr<Command> command = CreateSaveFileNameCommand(GetRelativePathToProjectPath(filePath));
    if (command)
    {
        SceneEditor2* sc = static_cast<SceneEditor2*>(GetScene());
        sc->Exec(std::move(command));
    }
}

std::unique_ptr<Command> CustomColorsSystem::CreateSaveFileNameCommand(const String& filePath)
{
    KeyedArchive* customProps = drawSystem->GetLandscapeCustomProperties();
    bool keyExists = customProps->IsKeyExists(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP);

    if (keyExists)
    {
        String curPath = customProps->GetString(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP);
        if (curPath != filePath)
        {
            return std::unique_ptr<KeyeadArchiveSetValueCommand>(new KeyeadArchiveSetValueCommand(customProps, ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP, VariantType(filePath)));
        }
    }
    else
    {
        return std::unique_ptr<KeyedArchiveAddValueCommand>(new KeyedArchiveAddValueCommand(customProps, ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP, VariantType(filePath)));
    }

    return std::unique_ptr<Command>();
}

FilePath CustomColorsSystem::GetCurrentSaveFileName()
{
    String currentSaveName;

    KeyedArchive* customProps = drawSystem->GetLandscapeCustomProperties();
    if (customProps && customProps->IsKeyExists(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP))
    {
        currentSaveName = customProps->GetString(ResourceEditor::CUSTOM_COLOR_TEXTURE_PROP);
    }

    FilePath currentTexturePath = GetAbsolutePathFromProjectPath(currentSaveName);
    if (currentTexturePath.IsEmpty())
    {
        FilePath scenePathName = static_cast<SceneEditor2*>(GetScene())->GetScenePath();
        scenePathName.ReplaceExtension("");

        Texture* colorMapTexture = drawSystem->GetLandscapeProxy()->GetLandscapeTexture(Landscape::TEXTURE_COLOR);
        DVASSERT(colorMapTexture != nullptr);

        FilePath colorMapDir = colorMapTexture->GetPathname().GetDirectory();
        currentTexturePath = colorMapDir + (scenePathName.GetFilename() + "_passability.png");
    }

    return currentTexturePath;
}

FilePath CustomColorsSystem::GetScenePath()
{
    return static_cast<SceneEditor2*>(GetScene())->GetScenePath().GetDirectory();
}

String CustomColorsSystem::GetRelativePathToScenePath(const FilePath& absolutePath)
{
    if (absolutePath.IsEmpty())
        return String();

    return absolutePath.GetRelativePathname(GetScenePath());
}

FilePath CustomColorsSystem::GetAbsolutePathFromScenePath(const String& relativePath)
{
    if (relativePath.empty())
        return FilePath();

    return (GetScenePath() + relativePath);
}

String CustomColorsSystem::GetRelativePathToProjectPath(const FilePath& absolutePath)
{
    if (absolutePath.IsEmpty())
        return String();

    ProjectManagerData* data = Deprecated::GetDataNode<ProjectManagerData>();
    DVASSERT(data != nullptr);

    return absolutePath.GetRelativePathname(data->GetProjectPath());
}

FilePath CustomColorsSystem::GetAbsolutePathFromProjectPath(const String& relativePath)
{
    if (relativePath.empty())
        return FilePath();

    ProjectManagerData* data = Deprecated::GetDataNode<ProjectManagerData>();
    DVASSERT(data != nullptr);
    return data->GetProjectPath() + relativePath;
}

int32 CustomColorsSystem::GetBrushSize()
{
    return curToolSize;
}

int32 CustomColorsSystem::GetColor()
{
    return colorIndex;
}
} // namespace DAVA
