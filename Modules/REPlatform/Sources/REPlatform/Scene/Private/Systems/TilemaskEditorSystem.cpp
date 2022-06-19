#include "REPlatform/Scene/Systems/TilemaskEditorSystem.h"

#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/Scene/Systems/LandscapeEditorDrawSystem.h"
#include "REPlatform/Scene/Systems/LandscapeProxy.h"
#include "REPlatform/Scene/Systems/ModifSystem.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"
#include "REPlatform/Scene/Utils/Utils.h"

#include "REPlatform/Commands/TilemaskEditorCommands.h"

#include <DeviceManager/DeviceManager.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Input/Keyboard.h>
#include <Render/DynamicBufferAllocator.h>
#include <Render/Image/ImageConvert.h>
#include <Render/Renderer.h>
#include <UI/UIEvent.h>

namespace DAVA
{
static Array<FastName, 4> TILECOLOR_PARAM_NAMES;

static const FastName TILEMASK_EDITOR_FLAG_DRAW_TYPE("DRAW_TYPE");
static const FastName TILEMASK_EDITOR_PARAM_INTENSITY("intensity");
static const FastName TILEMASK_EDITOR_PARAM_COPYPASTE_OFFSET("copypasteOffset");

static const FastName TILEMASK_EDTIOR_TEXTURE_SOURCE("sourceTexture");
static const FastName TILEMASK_EDTIOR_TEXTURE_TOOL("toolTexture");

static const FastName TILEMASK_EDITOR_MATERIAL_PASS("2d");

TilemaskEditorSystem::TilemaskEditorSystem(Scene* scene)
    : LandscapeEditorSystem(scene, DefaultCursorPath())
    , curToolSize(0)
    , toolImageTexture(nullptr)
    , landscapeTilemaskTexture(nullptr)
    , tileTextureNum(0)
    , drawingType(TILEMASK_DRAW_NORMAL)
    , strength(0.25f)
    , toolImagePath("")
    , toolImageIndex(0)
    , copyPasteFrom(-1.f, -1.f)
    , editingIsEnabled(false)
    , toolTexture(NULL)
    , toolSpriteUpdated(false)
    , needCreateUndo(false)
    , textureLevel(Landscape::TEXTURE_TILEMASK)
{
    curToolSize = 120;

    editorMaterial = new NMaterial();
    editorMaterial->SetFXName(FastName("~res:/Materials/Landscape.Tilemask.Editor.material"));
    editorMaterial->AddFlag(TILEMASK_EDITOR_FLAG_DRAW_TYPE, 0);
    editorMaterial->AddProperty(TILEMASK_EDITOR_PARAM_INTENSITY, &strength, rhi::ShaderProp::TYPE_FLOAT1);
    editorMaterial->AddProperty(TILEMASK_EDITOR_PARAM_COPYPASTE_OFFSET, copyPasteOffset.data, rhi::ShaderProp::TYPE_FLOAT2);

    editorMaterial->PreBuildMaterial(TILEMASK_EDITOR_MATERIAL_PASS);

    quadPacket.vertexStreamCount = 1;
    quadPacket.vertexCount = 6;
    quadPacket.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    quadPacket.primitiveCount = 2;

    rhi::VertexLayout layout;
    layout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    layout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);

    quadPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(layout);

    TILECOLOR_PARAM_NAMES[0] = Landscape::PARAM_TILE_COLOR0;
    TILECOLOR_PARAM_NAMES[1] = Landscape::PARAM_TILE_COLOR1;
    TILECOLOR_PARAM_NAMES[2] = Landscape::PARAM_TILE_COLOR2;
    TILECOLOR_PARAM_NAMES[3] = Landscape::PARAM_TILE_COLOR3;
}

TilemaskEditorSystem::~TilemaskEditorSystem()
{
    SafeRelease(editorMaterial);

    SafeRelease(toolImageTexture);
    SafeRelease(toolTexture);
}

LandscapeEditorDrawSystem::eErrorType TilemaskEditorSystem::EnableLandscapeEditing()
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

    LandscapeEditorDrawSystem::eErrorType enablingError = drawSystem->EnableTilemaskEditing();
    if (enablingError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return enablingError;
    }

    drawSystem->UpdateTilemaskPathname();
    bool inited = drawSystem->InitTilemaskImageCopy();
    if (!inited)
    {
        return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_TILEMASK_TEXTURE_ABSENT;
    }

    bool inputLocked = AcquireInputLock(GetScene());
    DVASSERT(inputLocked);
    Scene* scene = GetScene();
    scene->GetSystem<SelectionSystem>()->SetLocked(true);
    scene->GetSystem<EntityModificationSystem>()->SetLocked(true);

    landscapeSize = drawSystem->GetTextureSize(textureLevel);
    copyPasteFrom = Vector2(-1.f, -1.f);

    drawSystem->EnableCursor();
    drawSystem->EnableCustomDraw();
    drawSystem->SetCursorTexture(cursorTexture);
    drawSystem->SetCursorSize(cursorSize);
    SetBrushSize(curToolSize);

    InitSprites();

    Texture* srcSprite = drawSystem->GetLandscapeProxy()->GetTilemaskDrawTexture(LandscapeProxy::TILEMASK_TEXTURE_SOURCE);
    Texture* dstSprite = drawSystem->GetLandscapeProxy()->GetTilemaskDrawTexture(LandscapeProxy::TILEMASK_TEXTURE_DESTINATION);

    srcSprite->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NONE);
    dstSprite->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NONE);

    editorMaterial->AddTexture(TILEMASK_EDTIOR_TEXTURE_TOOL, toolTexture);
    editorMaterial->AddTexture(TILEMASK_EDTIOR_TEXTURE_SOURCE, srcSprite);

    enabled = true;
    return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool TilemaskEditorSystem::DisableLandscapeEdititing()
{
    if (!enabled)
    {
        return true;
    }

    FinishEditing();
    needCreateUndo = false;

    ReleaseInputLock(GetScene());
    Scene* scene = GetScene();
    scene->GetSystem<SelectionSystem>()->SetLocked(false);
    scene->GetSystem<EntityModificationSystem>()->SetLocked(false);

    drawSystem->DisableCursor();
    drawSystem->DisableCustomDraw();
    drawSystem->DisableTilemaskEditing();

    editorMaterial->RemoveTexture(TILEMASK_EDTIOR_TEXTURE_TOOL);
    editorMaterial->RemoveTexture(TILEMASK_EDTIOR_TEXTURE_SOURCE);

    SafeRelease(landscapeTilemaskTexture);

    enabled = false;
    return !enabled;
}

void TilemaskEditorSystem::Process(float32 timeElapsed)
{
    if (!IsLandscapeEditingEnabled())
    {
        return;
    }

    if (editingIsEnabled && isIntersectsLandscape)
    {
        if (prevCursorPos != cursorPosition)
        {
            prevCursorPos = cursorPosition;

            Vector2 toolSize = Vector2(static_cast<float32>(curToolSize), static_cast<float32>(curToolSize));
            Vector2 toolPos = cursorPosition * landscapeSize - toolSize / 2.f;
            Rect toolRect(std::floor(toolPos.x), std::floor(toolPos.y), std::ceil(toolSize.x), std::ceil(toolSize.y));

            RenderSystem2D::RenderTargetPassDescriptor desc;
            desc.priority = PRIORITY_SERVICE_2D;
            desc.colorAttachment = toolTexture->handle;
            desc.depthAttachment = toolTexture->handleDepthStencil;
            desc.width = toolTexture->GetWidth();
            desc.height = toolTexture->GetHeight();
            desc.transformVirtualToPhysical = false;
            RenderSystem2D::Instance()->BeginRenderTargetPass(desc);
            RenderSystem2D::Instance()->DrawTexture(toolImageTexture, RenderSystem2D::DEFAULT_2D_TEXTURE_MATERIAL, Color::White, toolRect);
            RenderSystem2D::Instance()->EndRenderTargetPass();

            toolSpriteUpdated = true;

            if (activeDrawingType == TILEMASK_DRAW_COPY_PASTE)
            {
                editorMaterial->SetPropertyValue(TILEMASK_EDITOR_PARAM_COPYPASTE_OFFSET, copyPasteOffset.data);
            }

            AddRectToAccumulator(toolRect);
        }
    }
}

bool TilemaskEditorSystem::Input(UIEvent* event)
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
            if (isIntersectsLandscape && !needCreateUndo)
            {
                if (drawingType == TILEMASK_DRAW_COPY_PASTE)
                {
                    Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
                    bool altPressed = (kb != nullptr) && (kb->GetKeyState(eInputElements::KB_LALT).IsPressed() || kb->GetKeyState(eInputElements::KB_RALT).IsPressed());
                    if (altPressed == true)
                    {
                        copyPasteFrom = cursorPosition;
                        copyPasteOffset = Vector2();
                        return false;
                    }
                    else
                    {
                        if (copyPasteFrom == Vector2(-1.f, -1.f))
                        {
                            return false;
                        }
                        copyPasteOffset = copyPasteFrom - cursorPosition;
                    }
                }

                ResetAccumulatorRect();
                editingIsEnabled = true;
                activeDrawingType = drawingType;
            }
            break;

        case UIEvent::Phase::DRAG:
            break;

        case UIEvent::Phase::ENDED:
            FinishEditing();
            break;

        default:
            break;
        }
    }
    return false;
}

void TilemaskEditorSystem::InputCancelled(UIEvent* event)
{
    if (IsLandscapeEditingEnabled() && (event->mouseButton == eMouseButtons::LEFT))
    {
        FinishEditing();
    }
}

void TilemaskEditorSystem::FinishEditing()
{
    if (editingIsEnabled)
    {
        needCreateUndo = true;
        editingIsEnabled = false;
    }
    prevCursorPos = Vector2(-1.f, -1.f);
}

void TilemaskEditorSystem::SetBrushSize(int32 brushSize)
{
    if (brushSize > 0)
    {
        curToolSize = brushSize;
        cursorSize = static_cast<float32>(brushSize) / landscapeSize;
        drawSystem->SetCursorSize(cursorSize);

        UpdateToolImage();
    }
}

void TilemaskEditorSystem::SetStrength(float32 strength_)
{
    if (strength_ >= 0)
    {
        strength = strength_;
        editorMaterial->SetPropertyValue(TILEMASK_EDITOR_PARAM_INTENSITY, &strength);
    }
}

void TilemaskEditorSystem::SetToolImage(const FilePath& toolImagePath, int32 index)
{
    this->toolImagePath = toolImagePath;
    this->toolImageIndex = index;
    UpdateToolImage();
}

void TilemaskEditorSystem::SetTileTexture(uint32 tileTexture)
{
    if (tileTexture >= GetTileTextureCount())
    {
        return;
    }

    tileTextureNum = tileTexture;

    editorMaterial->SetFlag(TILEMASK_EDITOR_FLAG_DRAW_TYPE, tileTextureNum);
    editorMaterial->PreBuildMaterial(TILEMASK_EDITOR_MATERIAL_PASS);
}

void TilemaskEditorSystem::UpdateBrushTool()
{
    struct QuadVertex
    {
        Vector3 position;
        Vector2 texCoord;
    };

    if (drawingType == TILEMASK_DRAW_COPY_PASTE && (copyPasteFrom == Vector2(-1.f, -1.f)))
        return;

    Texture* srcTexture = drawSystem->GetLandscapeProxy()->GetTilemaskDrawTexture(LandscapeProxy::TILEMASK_TEXTURE_SOURCE);
    Texture* dstTexture = drawSystem->GetLandscapeProxy()->GetTilemaskDrawTexture(LandscapeProxy::TILEMASK_TEXTURE_DESTINATION);

    editorMaterial->SetTexture(TILEMASK_EDTIOR_TEXTURE_SOURCE, srcTexture);

    rhi::RenderPassConfig passConf;
    passConf.colorBuffer[0].texture = dstTexture->handle;
    passConf.priority = PRIORITY_SERVICE_2D;
    passConf.viewport.width = dstTexture->GetWidth();
    passConf.viewport.height = dstTexture->GetHeight();
    passConf.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    Memset(passConf.colorBuffer[0].clearColor, 0, 4 * sizeof(float32));

    editorMaterial->PreBuildMaterial(TILEMASK_EDITOR_MATERIAL_PASS);
    editorMaterial->BindParams(quadPacket);

    DynamicBufferAllocator::AllocResultVB quadBuffer = DynamicBufferAllocator::AllocateVertexBuffer(sizeof(QuadVertex), 4);
    QuadVertex* quadVertices = reinterpret_cast<QuadVertex*>(quadBuffer.data);

    quadVertices[0].position = Vector3(-1.f, -1.f, .0f);
    quadVertices[1].position = Vector3(-1.f, 1.f, .0f);
    quadVertices[2].position = Vector3(1.f, -1.f, .0f);
    quadVertices[3].position = Vector3(1.f, 1.f, .0f);

    if (rhi::DeviceCaps().isCenterPixelMapping)
    {
        const float32 pixelOffset = 1.f / srcTexture->GetWidth();
        for (uint32 i = 0; i < 4; ++i)
        {
            quadVertices[i].position.x -= pixelOffset;
            quadVertices[i].position.y -= pixelOffset;
        }
    }

    if (rhi::DeviceCaps().isUpperLeftRTOrigin)
    {
        for (uint32 i = 0; i < 4; ++i)
            quadVertices[i].position.y = -quadVertices[i].position.y;
    }

    quadVertices[0].texCoord = Vector2(0.f, 0.f);
    quadVertices[1].texCoord = Vector2(0.f, 1.f);
    quadVertices[2].texCoord = Vector2(1.f, 0.f);
    quadVertices[3].texCoord = Vector2(1.f, 1.f);

    quadPacket.vertexStream[0] = quadBuffer.buffer;
    quadPacket.baseVertex = quadBuffer.baseVertex;
    quadPacket.indexBuffer = DynamicBufferAllocator::AllocateQuadListIndexBuffer(1);

    rhi::HPacketList pList;
    rhi::HRenderPass pass = rhi::AllocateRenderPass(passConf, 1, &pList);

    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(pList);

    rhi::AddPacket(pList, quadPacket);

    rhi::EndPacketList(pList);
    rhi::EndRenderPass(pass);

    drawSystem->SetTileMaskTexture(dstTexture);
    drawSystem->GetLandscapeProxy()->SwapTilemaskDrawTextures();
}

void TilemaskEditorSystem::UpdateToolImage()
{
    SafeRelease(toolImageTexture);

    Vector<Image*> images;
    ImageSystem::Load(toolImagePath, images);
    if (images.size())
    {
        DVASSERT(images.size() == 1);
        DVASSERT(images[0]->GetPixelFormat() == FORMAT_RGBA8888);

        ScopedPtr<Image> toolImage(Image::Create(curToolSize, curToolSize, FORMAT_RGBA8888));
        ImageConvert::ResizeRGBA8Billinear(reinterpret_cast<uint32*>(images[0]->data), images[0]->GetWidth(), images[0]->GetHeight(),
                                           reinterpret_cast<uint32*>(toolImage->data), curToolSize, curToolSize);

        SafeRelease(images[0]);

        toolImageTexture = Texture::CreateFromData(toolImage, false);
    }
}

void TilemaskEditorSystem::AddRectToAccumulator(const Rect& rect)
{
    updatedRectAccumulator = updatedRectAccumulator.Combine(rect);
}

void TilemaskEditorSystem::ResetAccumulatorRect()
{
    float32 inf = std::numeric_limits<float32>::infinity();
    updatedRectAccumulator = Rect(inf, inf, -inf, -inf);
}

Rect TilemaskEditorSystem::GetUpdatedRect()
{
    Rect r = updatedRectAccumulator;
    drawSystem->ClampToTexture(textureLevel, r);

    return r;
}

uint32 TilemaskEditorSystem::GetTileTextureCount() const
{
    return 4;
}

Texture* TilemaskEditorSystem::GetTileTexture()
{
    return drawSystem->GetLandscapeProxy()->GetLandscapeTexture(Landscape::TEXTURE_TILE);
}

Color TilemaskEditorSystem::GetTileColor(uint32 index)
{
    if (index < GetTileTextureCount())
    {
        return drawSystem->GetLandscapeProxy()->GetLandscapeTileColor(TILECOLOR_PARAM_NAMES[index]);
    }

    return Color::Black;
}

void TilemaskEditorSystem::SetTileColor(int32 index, const Color& color)
{
    if (index < 0 || index >= static_cast<int32>(GetTileTextureCount()))
    {
        return;
    }

    Color curColor = drawSystem->GetLandscapeProxy()->GetLandscapeTileColor(TILECOLOR_PARAM_NAMES[index]);

    if (curColor != color)
    {
        SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());
        scene->Exec(std::make_unique<SetTileColorCommand>(drawSystem->GetLandscapeProxy(), TILECOLOR_PARAM_NAMES[index], color));
    }
}

void TilemaskEditorSystem::CreateMaskTexture()
{
    Texture* tilemask = drawSystem->GetLandscapeProxy()->GetLandscapeTexture(Landscape::TEXTURE_TILEMASK);
    Texture* srcTexture = drawSystem->GetLandscapeProxy()->GetTilemaskDrawTexture(LandscapeProxy::TILEMASK_TEXTURE_SOURCE);

    if (tilemask != srcTexture)
    {
        landscapeTilemaskTexture = SafeRetain(tilemask);

        Rect destRect(0.0f, 0.0f, landscapeTilemaskTexture->width, landscapeTilemaskTexture->height);
        Rect sourceRect(0.0f, 0.0f, 1.0f, 1.0f);

        RenderSystem2D::RenderTargetPassDescriptor desc;
        desc.priority = PRIORITY_SERVICE_2D;
        desc.colorAttachment = srcTexture->handle;
        desc.depthAttachment = srcTexture->handleDepthStencil;
        desc.width = srcTexture->GetWidth();
        desc.height = srcTexture->GetHeight();
        desc.transformVirtualToPhysical = false;
        desc.clearTarget = true;

        RenderSystem2D::Instance()->BeginRenderTargetPass(desc);
        RenderSystem2D::Instance()->DrawTextureWithoutAdjustingRects(landscapeTilemaskTexture, RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL,
                                                                     Color::White, destRect, sourceRect);
        RenderSystem2D::Instance()->EndRenderTargetPass();

        drawSystem->SetTileMaskTexture(srcTexture);
    }
}

void TilemaskEditorSystem::Draw()
{
    if (!IsLandscapeEditingEnabled())
    {
        return;
    }

    if (toolSpriteUpdated)
    {
        UpdateBrushTool();
        toolSpriteUpdated = false;
    }

    if (needCreateUndo)
    {
        CreateUndoPoint();
        needCreateUndo = false;
    }
}

void TilemaskEditorSystem::CreateUndoPoint()
{
    Rect rect = GetUpdatedRect();
    if (rect.dx > 0 && rect.dy > 0)
    {
        SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());
        scene->Exec(std::make_unique<ModifyTilemaskCommand>(drawSystem->GetLandscapeProxy(), rect));
    }
}

int32 TilemaskEditorSystem::GetBrushSize()
{
    return curToolSize;
}

float32 TilemaskEditorSystem::GetStrength()
{
    return strength;
}

int32 TilemaskEditorSystem::GetToolImage()
{
    return toolImageIndex;
}

uint32 TilemaskEditorSystem::GetTileTextureIndex()
{
    return tileTextureNum;
}

void TilemaskEditorSystem::InitSprites()
{
    int32 texSize = static_cast<int32>(drawSystem->GetTextureSize(textureLevel));
    if (toolTexture != nullptr && texSize != toolTexture->GetWidth())
    {
        SafeRelease(toolTexture);
    }

    if (toolTexture == nullptr)
    {
        toolTexture = Texture::CreateFBO(texSize, texSize, FORMAT_RGBA8888 /*, Texture::DEPTH_NONE*/);
    }

    drawSystem->GetLandscapeProxy()->InitTilemaskDrawTextures();
    CreateMaskTexture();
}

void TilemaskEditorSystem::SetDrawingType(eTilemaskDrawType type)
{
    if (type >= TILEMASK_DRAW_NORMAL && type < TILEMASK_DRAW_TYPES_COUNT)
    {
        drawingType = type;

        if (type == TILEMASK_DRAW_COPY_PASTE)
        {
            editorMaterial->SetFlag(TILEMASK_EDITOR_FLAG_DRAW_TYPE, 4);
        }
        else
        {
            editorMaterial->SetFlag(TILEMASK_EDITOR_FLAG_DRAW_TYPE, tileTextureNum);
        }

        editorMaterial->PreBuildMaterial(TILEMASK_EDITOR_MATERIAL_PASS);
    }
}

TilemaskEditorSystem::eTilemaskDrawType TilemaskEditorSystem::GetDrawingType()
{
    return drawingType;
}
} // namespace DAVA
