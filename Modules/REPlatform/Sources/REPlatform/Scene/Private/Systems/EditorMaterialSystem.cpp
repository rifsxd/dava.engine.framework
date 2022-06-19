#include "REPlatform/Scene/Systems/EditorMaterialSystem.h"

#include "REPlatform/Commands/RECommandNotificationObject.h"
#include "REPlatform/Commands/DeleteRenderBatchCommand.h"
#include "REPlatform/Commands/ConvertToShadowCommand.h"
#include "REPlatform/Commands/DeleteLODCommand.h"
#include "REPlatform/Commands/CreatePlaneLODCommand.h"
#include "REPlatform/Commands/CloneLastBatchCommand.h"
#include "REPlatform/Commands/CopyLastLODCommand.h"
#include "REPlatform/Commands/InspMemberModifyCommand.h"
#include "REPlatform/Commands/SetFieldValueCommand.h"

#include <Render/Highlevel/Landscape.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Render/Material/NMaterial.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/LandscapeSystem.h>
#include <Reflection/ReflectedTypeDB.h>

namespace DAVA
{
EditorMaterialSystem::MaterialMapping::MaterialMapping(Entity* entity_, RenderBatch* renderBatch_)
    : entity(entity_)
{
    renderBatch = SafeRetain(renderBatch_);
}

EditorMaterialSystem::MaterialMapping::MaterialMapping(const MaterialMapping& other)
{
    *this = other;
}

EditorMaterialSystem::MaterialMapping::~MaterialMapping()
{
    SafeRelease(renderBatch);
}

EditorMaterialSystem::MaterialMapping& EditorMaterialSystem::MaterialMapping::operator=(const MaterialMapping& other)
{
    if (this == &other)
        return *this;

    entity = other.entity;
    SafeRelease(renderBatch);
    renderBatch = SafeRetain(other.renderBatch);
    return *this;
}

EditorMaterialSystem::EditorMaterialSystem(Scene* scene)
    : SceneSystem(scene)
{
}

EditorMaterialSystem::~EditorMaterialSystem()
{
    materialToObjectsMap.clear();
    for (auto op : ownedParents)
    {
        SafeRelease(op);
    }
}

Entity* EditorMaterialSystem::GetEntity(NMaterial* material) const
{
    auto it = materialToObjectsMap.find(material);
    return (it == materialToObjectsMap.end()) ? nullptr : it->second.entity;
}

const RenderBatch* EditorMaterialSystem::GetRenderBatch(NMaterial* material) const
{
    auto it = materialToObjectsMap.find(material);
    if (it == materialToObjectsMap.end())
        return nullptr;

    return it->second.renderBatch;
}

const Set<NMaterial*>& EditorMaterialSystem::GetTopParents() const
{
    return ownedParents;
}

int EditorMaterialSystem::GetLightViewMode()
{
    return curViewMode;
}

bool EditorMaterialSystem::GetLightViewMode(CommonInternalSettings::MaterialLightViewMode viewMode) const
{
    return (curViewMode & viewMode) != 0;
}

void EditorMaterialSystem::SetLightViewMode(int fullViewMode)
{
    if (curViewMode != fullViewMode)
    {
        curViewMode = fullViewMode;
        ApplyViewMode();
    }
}

void EditorMaterialSystem::SetLightViewMode(CommonInternalSettings::MaterialLightViewMode viewMode, bool set)
{
    int newMode = curViewMode;

    if (set)
    {
        newMode |= viewMode;
    }
    else
    {
        newMode &= ~viewMode;
    }

    SetLightViewMode(newMode);
}

void EditorMaterialSystem::SetLightmapCanvasVisible(bool enable)
{
    if (enable != showLightmapCanvas)
    {
        showLightmapCanvas = enable;
        ApplyViewMode();
    }
}

bool EditorMaterialSystem::IsLightmapCanvasVisible() const
{
    return showLightmapCanvas;
}

void EditorMaterialSystem::AddEntity(Entity* entity)
{
    RenderObject* ro = GetRenderObject(entity);
    if (nullptr != ro)
    {
        AddMaterials(entity);
    }
}

void EditorMaterialSystem::AddMaterials(Entity* entity)
{
    RenderObject* ro = GetRenderObject(entity);
    DVASSERT(nullptr != ro);

    for (uint32 i = 0; i < ro->GetRenderBatchCount(); ++i)
    {
        RenderBatch* renderBatch = ro->GetRenderBatch(i);
        AddMaterial(renderBatch->GetMaterial(), MaterialMapping(entity, renderBatch));
    }
}

void EditorMaterialSystem::RemoveEntity(Entity* entity)
{
    RenderObject* ro = GetRenderObject(entity);
    if (nullptr != ro)
    {
        for (uint32 i = 0; i < ro->GetRenderBatchCount(); ++i)
        {
            RenderBatch* rb = ro->GetRenderBatch(i);
            NMaterial* material = rb->GetMaterial();
            RemoveMaterial(material);
        }
    }
}

void EditorMaterialSystem::PrepareForRemove()
{
    materialToObjectsMap.clear();
}

void EditorMaterialSystem::ApplyViewMode()
{
    Set<NMaterial*>::const_iterator i = ownedParents.begin();
    Set<NMaterial*>::const_iterator end = ownedParents.end();

    for (; i != end; ++i)
    {
        ApplyViewMode(*i);
    }
}

void EditorMaterialSystem::ApplyViewMode(NMaterial* material)
{
    auto SetMaterialFlag = [](NMaterial* material, const FastName& flagName, int32 value) {
        if (value == 0)
        {
            if (material->HasLocalFlag(flagName))
            {
                material->RemoveFlag(flagName);
            }
        }
        else
        {
            if (material->HasLocalFlag(flagName))
            {
                material->SetFlag(flagName, value);
            }
            else
            {
                material->AddFlag(flagName, value);
            }
        }
    };

    if (curViewMode & CommonInternalSettings::LIGHTVIEW_ALBEDO)
    {
        SetMaterialFlag(material, NMaterialFlagName::FLAG_LIGHTMAPONLY, 0);
    }
    else
    {
        SetMaterialFlag(material, NMaterialFlagName::FLAG_LIGHTMAPONLY, 1);
    }

    if (showLightmapCanvas)
    {
        SetMaterialFlag(material, NMaterialFlagName::FLAG_SETUPLIGHTMAP, 1);
    }
    else
    {
        SetMaterialFlag(material, NMaterialFlagName::FLAG_SETUPLIGHTMAP, 0);
    }

    //materials
    auto UpdateFlags = [this, SetMaterialFlag](NMaterial* material, int32 mode, const FastName& flagName) {
        if (curViewMode & mode)
        {
            SetMaterialFlag(material, flagName, 1);
        }
        else
        {
            SetMaterialFlag(material, flagName, 0);
        }
    };

    UpdateFlags(material, CommonInternalSettings::LIGHTVIEW_ALBEDO, NMaterialFlagName::FLAG_VIEWALBEDO);
    UpdateFlags(material, CommonInternalSettings::LIGHTVIEW_DIFFUSE, NMaterialFlagName::FLAG_VIEWDIFFUSE);
    UpdateFlags(material, CommonInternalSettings::LIGHTVIEW_SPECULAR, NMaterialFlagName::FLAG_VIEWSPECULAR);
    UpdateFlags(material, CommonInternalSettings::LIGHTVIEW_AMBIENT, NMaterialFlagName::FLAG_VIEWAMBIENT);
}

void EditorMaterialSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    commandNotification.ForEach<DeleteLODCommand>([&](const DeleteLODCommand* cmd) {
        const Vector<DeleteRenderBatchCommand*> batchCommands = cmd->GetRenderBatchCommands();

        const uint32 count = (const uint32)batchCommands.size();
        for (uint32 i = 0; i < count; ++i)
        {
            RenderBatch* batch = batchCommands[i]->GetRenderBatch();
            if (commandNotification.IsRedo())
            {
                RemoveMaterial(batch->GetMaterial());
            }
            else
            {
                AddMaterial(batch->GetMaterial(), MaterialMapping(cmd->GetEntity(), batch));
            }
        }
    });

    commandNotification.ForEach<CreatePlaneLODCommand>([&](const CreatePlaneLODCommand* cmd) {
        RenderBatch* batch = cmd->GetRenderBatch();
        if (commandNotification.IsRedo())
        {
            AddMaterial(batch->GetMaterial(), MaterialMapping(cmd->GetEntity(), batch));
        }
        else
        {
            RemoveMaterial(batch->GetMaterial());
        }
    });

    commandNotification.ForEach<DeleteRenderBatchCommand>([&](const DeleteRenderBatchCommand* cmd) {
        RenderBatch* batch = cmd->GetRenderBatch();
        if (commandNotification.IsRedo())
        {
            RemoveMaterial(batch->GetMaterial());
        }
        else
        {
            AddMaterial(batch->GetMaterial(), MaterialMapping(cmd->GetEntity(), batch));
        }
    });

    commandNotification.ForEach<ConvertToShadowCommand>([&](const ConvertToShadowCommand* cmd) {
        RenderBatch* oldBatch = cmd->oldBatch;
        RenderBatch* newBatch = cmd->newBatch;

        if (!commandNotification.IsRedo())
        {
            std::swap(oldBatch, newBatch);
        }

        RemoveMaterial(oldBatch->GetMaterial());
        AddMaterial(newBatch->GetMaterial(), MaterialMapping(cmd->GetEntity(), newBatch));
    });

    commandNotification.ForEach<CopyLastLODToLod0Command>([&](const CopyLastLODToLod0Command* cmd) {
        uint32 batchCount = static_cast<uint32>(cmd->newBatches.size());
        for (uint32 i = 0; i < batchCount; ++i)
        {
            RenderBatch* batch = cmd->newBatches[i];
            NMaterial* material = batch->GetMaterial();
            if (commandNotification.IsRedo())
            {
                AddMaterial(material, MaterialMapping(cmd->GetEntity(), batch));
            }
            else
            {
                RemoveMaterial(material);
            }
        }
    });

    commandNotification.ForEach<InspMemberModifyCommand>([&](const InspMemberModifyCommand* cmd) {
        const Vector<Entity*>& landscapes = GetScene()->landscapeSystem->GetLandscapeEntities();
        for (Entity* landEntity : landscapes)
        {
            Landscape* landObject = GetLandscape(landEntity);
            if (landObject == cmd->object)
            {
                RemoveMaterial(landObject->GetMaterial());
                AddMaterials(landEntity);
            }
        }
    });

    commandNotification.ForEach<SetFieldValueCommand>([&](const SetFieldValueCommand* cmd) {
        const Reflection::Field& field = cmd->GetField();
        ReflectedObject object = field.ref.GetDirectObject();

        if (object.GetReflectedType() == ReflectedTypeDB::Get<Landscape>())
        {
            Landscape* landscape = object.GetPtr<Landscape>();
            const Vector<Entity*>& landscapes = GetScene()->landscapeSystem->GetLandscapeEntities();
            for (Entity* landEntity : landscapes)
            {
                Landscape* landObject = GetLandscape(landEntity);
                if (landObject == landscape)
                {
                    RemoveMaterial(landObject->GetMaterial());
                    AddMaterials(landEntity);
                    break;
                }
            }
        }
    });
}

bool EditorMaterialSystem::HasMaterial(NMaterial* material) const
{
    return (materialToObjectsMap.count(material) > 0) || (ownedParents.count(material) > 0);
}

void EditorMaterialSystem::AddMaterial(NMaterial* material, const MaterialMapping& mapping)
{
    if (nullptr != material)
    {
        NMaterial* curGlobalMaterial = nullptr;
        if ((nullptr != mapping.entity) && (nullptr != mapping.entity->GetScene()))
        {
            curGlobalMaterial = mapping.entity->GetScene()->GetGlobalMaterial();
        }

        materialToObjectsMap[material] = mapping;

        // remember parent material, if still isn't
        NMaterial* parent = material;
        for (;;)
        {
            NMaterial* nextParent = parent->GetParent();
            if (nullptr == nextParent || curGlobalMaterial == nextParent)
            {
                break;
            }
            else
            {
                parent = nextParent;
            }
        }

        if (0 == ownedParents.count(parent))
        {
            if (IsEditable(parent))
            {
                ownedParents.insert(parent);
                SafeRetain(parent);
                ApplyViewMode(parent);
            }
        }
    }
}

void EditorMaterialSystem::RemoveMaterial(NMaterial* material)
{
    materialToObjectsMap.erase(material);
}

bool EditorMaterialSystem::IsEditable(NMaterial* material) const
{
    return (!material->IsRuntime());
}
} // namespace DAVA
