#include "REPlatform/Scene/Systems/SelectionSystem.h"
#include "REPlatform/Scene/Systems/CameraSystem.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Systems/HoodSystem.h"
#include "REPlatform/Scene/Systems/ModifSystem.h"

#include "REPlatform/Commands/ConvertToBillboardCommand.h"
#include "REPlatform/Commands/EntityParentChangeCommand.h"
#include "REPlatform/Commands/EntityRemoveCommand.h"
#include "REPlatform/Commands/RECommandNotificationObject.h"
#include "REPlatform/Commands/TransformCommand.h"
#include "REPlatform/DataNodes/Settings/GlobalSceneSettings.h"

#include <TArc/Core/Deprecated.h>

#include <DeviceManager/DeviceManager.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Input/Keyboard.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>
#include <Scene3D/Systems/EventSystem.h>
#include <Particles/ParticleEmitter.h>
#include <Math/TransformUtils.h>
#include <Math/Transform.h>
#include <Base/Vector.h>
#include <Base/Any.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Render/2D/Systems/RenderSystem2D.h>

namespace DAVA
{
SelectionSystem::SelectionSystem(Scene* scene)
    : SceneSystem(scene)
{
    collisionSystem = scene->GetSystem<SceneCollisionSystem>();
    hoodSystem = scene->GetSystem<HoodSystem>();
    modificationSystem = scene->GetSystem<EntityModificationSystem>();
    componentMaskForSelection.set();

    DVASSERT(collisionSystem != nullptr);
    DVASSERT(hoodSystem != nullptr);
    DVASSERT(modificationSystem != nullptr);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SWITCH_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::GEO_DECAL_CHANGED);

    wasLockedInActiveMode = IsLocked();
}

SelectionSystem::~SelectionSystem()
{
    if (GetScene())
    {
        GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::SWITCH_CHANGED);
        GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::GEO_DECAL_CHANGED);
    }
}

void SelectionSystem::ImmediateEvent(Component* component, uint32 event)
{
    using namespace DAVA;
    switch (event)
    {
    case EventSystem::SWITCH_CHANGED:
    {
        if (currentSelection.ContainsObject(component->GetEntity()))
        {
            invalidSelectionBoxes = true;
        }
        break;
    }
    case EventSystem::GEO_DECAL_CHANGED:
    {
        if (currentSelection.ContainsObject(component->GetEntity()))
        {
            invalidSelectionBoxes = true;
        }
        break;
    }
    default:
        break;
    }
}

void SelectionSystem::UpdateGroupSelectionMode()
{
    Engine* engine = Engine::Instance();
    DVASSERT(engine != nullptr);
    const EngineContext* engineContext = engine->GetContext();
    DVASSERT(engineContext != nullptr);
    if (engineContext->inputSystem == nullptr)
    {
        return;
    }
    Keyboard* kb = engineContext->deviceManager->GetKeyboard();

    bool addSelection = kb->GetKeyState(eInputElements::KB_LCTRL).IsPressed() || kb->GetKeyState(eInputElements::KB_RCTRL).IsPressed();
    bool excludeSelection = kb->GetKeyState(eInputElements::KB_LALT).IsPressed() || kb->GetKeyState(eInputElements::KB_RALT).IsPressed();

    if (addSelection)
    {
        groupSelectionMode = GroupSelectionMode::Add;
    }
    else if (excludeSelection)
    {
        groupSelectionMode = GroupSelectionMode::Remove;
    }
    else
    {
        groupSelectionMode = GroupSelectionMode::Replace;
    }
}

namespace SelectionSystemDetails
{
bool FindIfParentWasAdded(Entity* entity, const List<Entity*>& container, Scene* scene)
{
    Entity* parent = entity->GetParent();
    if (parent == scene || parent == nullptr)
    {
        return false;
    }

    auto found = std::find(container.begin(), container.end(), parent);
    if (found != container.end())
    {
        return true;
    }

    return FindIfParentWasAdded(parent, container, scene);
}
}

void SelectionSystem::Process(float32 timeElapsed)
{
    using namespace DAVA;
    InputLockGuard lockGuard(GetScene(), this);
    if (lockGuard.IsLockAcquired() == false)
    {
        return;
    }

    TransformSingleComponent* tsc = GetScene()->transformSingleComponent;
    for (Entity* entity : tsc->localTransformChanged)
    {
        if (currentSelection.ContainsObject(entity))
        {
            invalidSelectionBoxes = true;
            break;
        }
    }
    for (Entity* entity : tsc->transformParentChanged)
    {
        if (currentSelection.ContainsObject(entity))
        {
            invalidSelectionBoxes = true;
            break;
        }
    }

    if (!entitiesForSelection.empty())
    {
        SelectableGroup::CollectionType objectsToSelect;
        objectsToSelect.reserve(entitiesForSelection.size());
        for (Entity* entity : entitiesForSelection)
        {
            if (false == SelectionSystemDetails::FindIfParentWasAdded(entity, entitiesForSelection, GetScene()))
            {
                Any anyEntity(entity);
                Selectable obj(anyEntity);
                obj.SetBoundingBox(collisionSystem->GetUntransformedBoundingBox(anyEntity));
                objectsToSelect.push_back(obj);
            }
        }
        entitiesForSelection.clear();

        SelectableGroup newSelection;
        newSelection.Add(objectsToSelect);
        SetSelection(newSelection);
    }

    // if boxes are invalid we should request them from collision system
    // and store them in the currentSelection objects
    if (invalidSelectionBoxes)
    {
        for (Selectable& item : currentSelection.GetMutableContent())
        {
            item.SetBoundingBox(collisionSystem->GetUntransformedBoundingBox(item.GetContainedObject()));
        }

        currentSelection.RebuildIntegralBoundingBox();
        invalidSelectionBoxes = false;

        selectionBox = currentSelection.GetTransformedBoundingBox();
    }

    UpdateGroupSelectionMode();
    UpdateHoodPos();
}

void SelectionSystem::ProcessSelectedGroup(const SelectableGroup::CollectionType& allObjects)
{
    SelectableGroup::CollectionType collisionObjects;
    collisionObjects.reserve(allObjects.size());

    for (const auto& item : allObjects)
    {
        bool wasAdded = false;

        Entity* entity = item.AsEntity();
        if (entity == nullptr)
        {
            Selectable selectable = GetSelectableObject(item);
            if (selectable.ContainsObject())
            {
                collisionObjects.emplace_back(selectable.GetContainedObject());
                wasAdded = true;
            }
        }
        else if ((componentMaskForSelection & entity->GetAvailableComponentMask()).any())
        {
            if (GetCamera(entity) != GetScene()->GetCurrentCamera())
            {
                collisionObjects.emplace_back(GetSelectableEntity(entity));
                wasAdded = true;
            }
        }

        if (wasAdded)
        {
            collisionObjects.back().SetBoundingBox(collisionSystem->GetUntransformedBoundingBox(collisionObjects.back().GetContainedObject()));
        }
    }

    Any firstEntity = collisionObjects.empty() == false ? collisionObjects.front().GetContainedObject() : Any();
    Any nextEntity = firstEntity;

    // sequent selection?
    GlobalSceneSettings* settings = Deprecated::GetDataNode<GlobalSceneSettings>();

    if (settings->selectionSequent == true && (currentSelection.GetSize() <= 1))
    {
        // find first after currently selected items
        for (size_t i = 0, e = collisionObjects.size(); i < e; i++)
        {
            if (currentSelection.ContainsObject(collisionObjects[i].GetContainedObject()))
            {
                if ((i + 1) < e)
                {
                    nextEntity = collisionObjects[i + 1].GetContainedObject();
                    break;
                }
            }
        }
    }

    Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();

    bool addSelection = kb->GetKeyState(eInputElements::KB_LCTRL).IsPressed() || kb->GetKeyState(eInputElements::KB_RCTRL).IsPressed();
    bool excludeSelection = kb->GetKeyState(eInputElements::KB_LALT).IsPressed() || kb->GetKeyState(eInputElements::KB_RALT).IsPressed();

    if (addSelection && (firstEntity.IsEmpty() == false))
    {
        SelectableGroup newSelection = currentSelection;
        Selectable objToSelect(firstEntity);
        objToSelect.SetBoundingBox(collisionSystem->GetUntransformedBoundingBox(firstEntity));

        newSelection.Add(firstEntity);
        SetSelection(newSelection);
    }
    else if (excludeSelection && (firstEntity.IsEmpty() == false))
    {
        SelectableGroup newSelection = currentSelection;
        Selectable objToSelect(firstEntity);

        newSelection.Remove(firstEntity);
        objectsToSelect.Remove(firstEntity);
        SetSelection(newSelection);
    }
    else
    {
        bool selectOnClick = settings->selectionOnClick;
        // if new selection is NULL or is one of already selected items
        // we should change current selection only on phase end
        // TODO : review and remove?
        bool containsAlreadySelectedObjects = false;
        for (const auto& obj : collisionObjects)
        {
            if (currentSelection.ContainsObject(obj.GetContainedObject()))
            {
                containsAlreadySelectedObjects = true;
                break;
            }
        }

        if (selectOnClick || (nextEntity.IsEmpty() == true) || containsAlreadySelectedObjects)
        {
            applyOnPhaseEnd = true;
            objectsToSelect.Clear();
            if (nextEntity.IsEmpty() == false)
            {
                objectsToSelect.Add(nextEntity, collisionSystem->GetUntransformedBoundingBox(nextEntity));
            }
        }
        else
        {
            SelectableGroup newSelection;
            newSelection.Add(nextEntity, collisionSystem->GetUntransformedBoundingBox(nextEntity));
            SetSelection(newSelection);
        }
    }
}

void SelectionSystem::PerformSelectionAtPoint(const Vector2& point)
{
    Vector3 traceFrom;
    Vector3 traceTo;
    SceneCameraSystem* cameraSystem = GetScene()->GetSystem<SceneCameraSystem>();
    if (cameraSystem->GetCurCamera() != nullptr)
    {
        cameraSystem->GetRayTo2dPoint(point, cameraSystem->GetCurCamera()->GetZFar(), traceFrom, traceTo);
    }
    SelectableGroup::CollectionType collection;
    collisionSystem->ObjectsRayTest(traceFrom, traceTo, collection);
    ProcessSelectedGroup(collection);
}

void SelectionSystem::PerformSelectionInCurrentBox()
{
    using namespace DAVA;
    UpdateGroupSelectionMode();

    const float32 minSelectionSize = 2.0f;

    Vector2 selectionSize = selectionEndPoint - selectionStartPoint;
    if ((std::abs(selectionSize.x) < minSelectionSize) || (std::abs(selectionSize.y) < minSelectionSize))
    {
        return;
    };

    float32 minX = std::min(selectionStartPoint.x, selectionEndPoint.x);
    float32 minY = std::min(selectionStartPoint.y, selectionEndPoint.y);
    float32 maxX = std::max(selectionStartPoint.x, selectionEndPoint.x);
    float32 maxY = std::max(selectionStartPoint.y, selectionEndPoint.y);

    Vector3 p0;
    Vector3 p1;
    Vector3 p2;
    Vector3 p3;
    Vector3 p4;

    SceneCameraSystem* cameraSystem = GetScene()->GetSystem<SceneCameraSystem>();
    cameraSystem->GetRayTo2dPoint(Vector2(minX, minY), 1.0f, p0, p1);
    cameraSystem->GetRayTo2dPoint(Vector2(maxX, minY), 1.0f, p0, p2);
    cameraSystem->GetRayTo2dPoint(Vector2(minX, maxY), 1.0f, p0, p4);
    cameraSystem->GetRayTo2dPoint(Vector2(maxX, maxY), 1.0f, p0, p3);

    const Vector<Plane> planes =
    {
      Plane(p2, p1, p0),
      Plane(p3, p2, p0),
      Plane(p4, p3, p0),
      Plane(p1, p4, p0)
    };

    SelectableGroup::CollectionType allSelectedObjects;
    collisionSystem->ClipObjectsToPlanes(planes, allSelectedObjects);

    SelectableGroup selectedObjects;
    for (const Selectable& item : allSelectedObjects)
    {
        Entity* entity = item.AsEntity();
        if (entity == nullptr)
        {
            Selectable selectableItem = GetSelectableObject(item);
            Any object = selectableItem.GetContainedObject();
            if (!selectedObjects.ContainsObject(object))
            {
                selectedObjects.Add(object, collisionSystem->GetUntransformedBoundingBox(object));
            }
        }
        else if (IsEntitySelectable(entity))
        {
            if (GetCamera(entity) != GetScene()->GetCurrentCamera())
            {
                Entity* selectableEntity = GetSelectableEntity(entity);
                if (!selectableEntity->GetLocked() && !selectedObjects.ContainsObject(selectableEntity))
                {
                    selectedObjects.Add(selectableEntity, collisionSystem->GetUntransformedBoundingBox(selectableEntity));
                }
            }
        }
    }

    UpdateSelectionGroup(selectedObjects);
    applyOnPhaseEnd = true;
}

void SelectionSystem::AddEntity(Entity* entity)
{
    if (IsSystemEnabled() && entity->GetName().find("editor.") == String::npos && (entity != GetScene()))
    { // need ignore editor specific entities in auto selection
        auto autoSelectionEnabled = Deprecated::GetDataNode<GlobalSceneSettings>()->autoSelectNewEntity;
        if (autoSelectionEnabled && !IsLocked())
        {
            Entity* parent = entity->GetParent();
            auto found = std::find(entitiesForSelection.begin(), entitiesForSelection.end(), parent);
            if (found == entitiesForSelection.end())
            {
                entitiesForSelection.push_back(entity); // need add only parent entities to select one
            }
        }
    }
}

namespace SelectionSystemDetails
{
void EnumerateSelectableObjects(ParticleEmitter* emitter, Vector<Any>& enumeratedObjects)
{
    for (ParticleLayer* layer : emitter->layers)
    {
        if (layer->innerEmitter != nullptr)
        {
            EnumerateSelectableObjects(layer->innerEmitter->GetEmitter(), enumeratedObjects);
        }
    }

    enumeratedObjects.push_back(emitter);
}

void EnumerateSelectableObjects(Entity* entity, Vector<Any>& enumeratedObjects)
{
    ParticleEffectComponent* particleEffect = GetEffectComponent(entity);
    if (particleEffect != nullptr)
    {
        for (int32 i = 0, e = particleEffect->GetEmittersCount(); i < e; ++i)
        {
            EnumerateSelectableObjects(particleEffect->GetEmitterInstance(i)->GetEmitter(), enumeratedObjects);
            enumeratedObjects.push_back(particleEffect->GetEmitterInstance(i));
        }
    }

    enumeratedObjects.push_back(entity);
}
}

void SelectionSystem::RemoveEntity(Entity* entity)
{
    if (!entitiesForSelection.empty())
    {
        entitiesForSelection.remove(entity);
    }

    Vector<Any> potentiallySelectedObjects;
    SelectionSystemDetails::EnumerateSelectableObjects(entity, potentiallySelectedObjects);

    Set<Selectable> currentSelected(currentSelection.GetContent().begin(), currentSelection.GetContent().end());
    for (const Any& object : potentiallySelectedObjects)
    {
        currentSelected.erase(Selectable(object));
    }

    SelectableGroup newSelection;
    newSelection.Add(SelectableGroup::CollectionType(currentSelected.begin(), currentSelected.end()));
    SetSelection(newSelection);
}

void SelectionSystem::PrepareForRemove()
{
    entitiesForSelection.clear();
    currentSelection.Clear();
    lastGroupSelection.Clear();
    objectsToSelect.Clear();
    invalidSelectionBoxes = true;
}

bool SelectionSystem::Input(UIEvent* event)
{
    InputLockGuard guard(GetScene(), this);
    if (guard.IsLockAcquired() == false || !selectionAllowed || componentMaskForSelection.none() || (event->mouseButton != eMouseButtons::LEFT))
    {
        return false;
    }

    if (UIEvent::Phase::BEGAN == event->phase)
    {
        for (auto selectionDelegate : selectionDelegates)
        {
            if (selectionDelegate->AllowPerformSelectionHavingCurrent(currentSelection) == false)
            {
                return false;
            }
        }

        selecting = true;
        selectionStartPoint = event->point;
        selectionEndPoint = selectionStartPoint;
        lastGroupSelection.Clear();
        PerformSelectionAtPoint(selectionStartPoint);
    }
    else if (selecting && (UIEvent::Phase::DRAG == event->phase))
    {
        selectionEndPoint = event->point;
        PerformSelectionInCurrentBox();
    }
    else if (UIEvent::Phase::ENDED == event->phase)
    {
        if ((event->mouseButton == eMouseButtons::LEFT) && applyOnPhaseEnd)
        {
            FinishSelection();
        }
        applyOnPhaseEnd = false;
        selecting = false;
    }
    return false;
}

void SelectionSystem::DrawItem(const AABBox3& originalBox, const Matrix4& transform, int32 drawMode,
                               RenderHelper::eDrawType wireDrawType, RenderHelper::eDrawType solidDrawType, const Color& color)
{
    AABBox3 bbox;
    originalBox.GetTransformedBox(transform, bbox);

    // draw selection share
    if (drawMode & SS_DRAW_SHAPE)
    {
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(bbox, color, wireDrawType);
    }
    // draw selection share
    else if (drawMode & SS_DRAW_CORNERS)
    {
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABoxCorners(bbox, color, wireDrawType);
    }
    // fill selection shape
    if (drawMode & SS_DRAW_BOX)
    {
        Color alphaScaledColor = color;
        alphaScaledColor.a *= 0.15f;
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(bbox, alphaScaledColor, solidDrawType);
    }
}

void SelectionSystem::Draw()
{
    InputLockGuard guard(GetScene(), this);
    if (guard.IsLockAcquired() == false)
    {
        return;
    }

    Vector2 selectionSize = selectionEndPoint - selectionStartPoint;
    if (selecting && (selectionSize.Length() >= 1.0f))
    {
        Rect targetRect(selectionStartPoint, selectionSize);
        RenderSystem2D::Instance()->FillRect(targetRect, Color(1.0f, 1.0f, 1.0f, 1.0f / 3.0f));
        RenderSystem2D::Instance()->DrawRect(targetRect, Color::White);
    }

    int32 drawMode = static_cast<int32>(Deprecated::GetDataNode<GlobalSceneSettings>()->selectionDrawMode);
    if (drawMode == SelectionSystemDrawMode::SS_DRAW_NOTHING)
    {
        return;
    }

    RenderHelper::eDrawType wireDrawType = (!(drawMode & SS_DRAW_NO_DEEP_TEST)) ?
    RenderHelper::DRAW_WIRE_DEPTH :
    RenderHelper::DRAW_WIRE_NO_DEPTH;

    RenderHelper::eDrawType solidDrawType = (!(drawMode & SS_DRAW_NO_DEEP_TEST)) ?
    RenderHelper::DRAW_SOLID_DEPTH :
    RenderHelper::DRAW_SOLID_NO_DEPTH;

    bool replacingSelection = selecting && (groupSelectionMode == GroupSelectionMode::Replace);
    if (!replacingSelection)
    {
        for (const auto& item : currentSelection.GetContent())
        {
            if (item.SupportsTransformType(Selectable::TransformType::Disabled))
            {
                DrawItem(item.GetBoundingBox(), TransformUtils::ToMatrix(item.GetWorldTransform()), drawMode, wireDrawType, solidDrawType, Color::White);
            }
        }
    }

    Color drawColor = Color::White;
    if (groupSelectionMode == GroupSelectionMode::Add)
    {
        drawColor = Color(0.5f, 1.0f, 0.5f, 1.0f);
    }
    else if (groupSelectionMode == GroupSelectionMode::Remove)
    {
        drawColor = Color(1.0f, 0.5f, 0.5f, 1.0f);
    }

    for (const auto& item : objectsToSelect.GetContent())
    {
        DrawItem(item.GetBoundingBox(), TransformUtils::ToMatrix(item.GetWorldTransform()), drawMode, wireDrawType, solidDrawType, drawColor);
    }
}

void SelectionSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    if (commandNotification.MatchCommandTypes<EntityRemoveCommand, EntityParentChangeCommand,
                                              TransformCommand, ConvertToBillboardCommand>() == true)
    {
        invalidSelectionBoxes = true;
    }
}

void SelectionSystem::SetSelection(SelectableGroup& newSelection)
{
    if (IsLocked())
        return;

    newSelection.RemoveIf([this](const Selectable& obj) {
        return (obj.CanBeCastedTo<Entity>() && !IsEntitySelectable(obj.AsEntity()));
    });

    /*
	 * Ask delegates if selection could be changed
	 */
    for (auto selectionDelegate : selectionDelegates)
    {
        if (selectionDelegate->AllowChangeSelectionReplacingCurrent(currentSelection, newSelection) == false)
        {
            return;
        }
    }

    /*
	 * Actually change selection
	 */
    for (const auto& i : currentSelection.GetContent())
    {
        objectsToSelect.Remove(i.GetContainedObject());
    }
    currentSelection = newSelection;

    invalidSelectionBoxes = true;

    const SelectableGroup& transformableSelection = modificationSystem->GetTransformableSelection();
    if (transformableSelection.IsEmpty() == false)
    {
        hoodSystem->SetAxes(modificationSystem->GetModifAxisX(), modificationSystem->GetModifAxisY(), modificationSystem->GetModifAxisZ());
    }

    UpdateHoodPos();
}

bool SelectionSystem::IsEntitySelectable(Entity* entity) const
{
    if (!IsLocked() && (entity != nullptr))
    {
        return ((componentMaskForSelection & entity->GetAvailableComponentMask()).any());
    }

    return false;
}

void SelectionSystem::ExcludeSingleItem(const Any& entity)
{
    auto newSelection = currentSelection;
    if (newSelection.ContainsObject(entity))
    {
        newSelection.Remove(entity);
    }
    if (objectsToSelect.ContainsObject(entity))
    {
        objectsToSelect.Remove(entity);
    }
    SetSelection(newSelection);
}

void SelectionSystem::Clear()
{
    if (!IsLocked())
    {
        SelectableGroup::CollectionType allItems = currentSelection.GetContent();
        currentSelection.Clear();
        Set<Selectable> items(allItems.begin(), allItems.end());

        SelectableGroup::CollectionType allowedToSelect;
        allowedToSelect.reserve(objectsToSelect.GetSize());
        for (const Selectable& obj : objectsToSelect.GetContent())
        {
            if (items.count(obj) == 0)
            {
                allowedToSelect.push_back(obj);
            }
        }
        objectsToSelect.Clear();
        objectsToSelect.Add(allowedToSelect);

        invalidSelectionBoxes = true;
        UpdateHoodPos();
    }
}

const SelectableGroup& SelectionSystem::GetSelection() const
{
    static const SelectableGroup emptyGroup = SelectableGroup();
    return IsLocked() ? emptyGroup : currentSelection;
}

void SelectionSystem::CancelSelection()
{
    // don't change selection on phase end
    applyOnPhaseEnd = false;
}

void SelectionSystem::SetLocked(bool lock)
{
    SceneSystem::SetLocked(lock);

    hoodSystem->LockAxis(lock);
    hoodSystem->SetVisible(!lock);

    if (!lock)
    {
        UpdateHoodPos();
    }
}

void SelectionSystem::UpdateHoodPos() const
{
    if (currentSelection.IsEmpty())
    {
        hoodSystem->LockModif(false);
        hoodSystem->SetVisible(false);
    }
    else
    {
        const SelectableGroup& transformableSelection = modificationSystem->GetTransformableSelection();
        bool transformableSelectionEmpty = transformableSelection.IsEmpty();
        hoodSystem->LockModif(transformableSelectionEmpty);

        if (!transformableSelectionEmpty)
        {
            Vector3 hoodCenter;
            if (modificationSystem->GetPivotPoint() == Selectable::TransformPivot::ObjectCenter)
            {
                hoodCenter = currentSelection.GetFirst().GetWorldTransform().GetTranslation();
            }
            else
            {
                hoodCenter = currentSelection.GetCommonWorldSpaceTranslationVector();
            }
            hoodSystem->SetPosition(hoodCenter);
            hoodSystem->SetVisible(true);
        }
    }

    GetScene()->GetSystem<SceneCameraSystem>()->UpdateDistanceToCamera();
}

void SelectionSystem::SetSelectionComponentMask(const ComponentMask& mask)
{
    componentMaskForSelection = mask;

    if (currentSelection.IsEmpty() == false)
    {
        Clear();
    }
}

void SelectionSystem::Activate()
{
    SetLocked(wasLockedInActiveMode);
}

void SelectionSystem::Deactivate()
{
    wasLockedInActiveMode = IsLocked();
    SetLocked(true);
}

void SelectionSystem::UpdateSelectionGroup(const SelectableGroup& newSelection)
{
    objectsToSelect.Exclude(lastGroupSelection);
    objectsToSelect.RemoveIf([](const Selectable& e) {
        return e.SupportsTransformType(Selectable::TransformType::Disabled);
    });

    if (groupSelectionMode == GroupSelectionMode::Replace)
    {
        objectsToSelect.Join(newSelection);
    }
    else if (groupSelectionMode == GroupSelectionMode::Add)
    {
        for (const auto& item : newSelection.GetContent())
        {
            auto obj = item.GetContainedObject();
            if (!currentSelection.ContainsObject(obj))
            {
                objectsToSelect.Add(obj, item.GetBoundingBox());
            }
        }
    }
    else if (groupSelectionMode == GroupSelectionMode::Remove)
    {
        for (const auto& item : newSelection.GetContent())
        {
            if (currentSelection.ContainsObject(item.GetContainedObject()))
            {
                objectsToSelect.Add(item.GetContainedObject(), item.GetBoundingBox());
            }
        }
    }

    lastGroupSelection = newSelection;
}

void SelectionSystem::FinishSelection()
{
    SelectableGroup newSelection;

    if (groupSelectionMode == GroupSelectionMode::Replace)
    {
        newSelection.Join(objectsToSelect);
    }
    else if (groupSelectionMode == GroupSelectionMode::Add)
    {
        newSelection.Join(objectsToSelect);
        newSelection.Join(currentSelection);
    }
    else if (groupSelectionMode == GroupSelectionMode::Remove)
    {
        newSelection.Join(currentSelection);
        newSelection.Exclude(objectsToSelect);
    }
    else
    {
        DVASSERT(0, "Invalid selection mode");
    }
    objectsToSelect.Clear();

    SetSelection(newSelection);
}

void SelectionSystem::AddDelegate(SelectionSystemDelegate* delegate_)
{
    DVASSERT(std::find(selectionDelegates.begin(), selectionDelegates.end(), delegate_) == selectionDelegates.end());
    selectionDelegates.push_back(delegate_);
}

void SelectionSystem::RemoveDelegate(SelectionSystemDelegate* delegate_)
{
    auto i = std::remove(selectionDelegates.begin(), selectionDelegates.end(), delegate_);
    selectionDelegates.erase(i, selectionDelegates.end());
}

const AABBox3& SelectionSystem::GetSelectionBox() const
{
    return selectionBox;
}

Entity* GetSelectableEntity(Entity* selectionCandidate)
{
    Entity* parent = selectionCandidate;
    while (nullptr != parent)
    {
        if (parent->GetSolid())
        {
            selectionCandidate = parent;
        }
        parent = parent->GetParent();
    }
    return selectionCandidate;
}

Selectable GetSelectableObject(const Selectable& object)
{
    if (object.CanBeCastedTo<Entity>())
    {
        Entity* obj = object.Cast<Entity>();
        obj = GetSelectableEntity(obj);
        return Selectable(Any(obj));
    }
    else if (object.CanBeCastedTo<ParticleEmitterInstance>())
    {
        ParticleEmitterInstance* instance = object.Cast<ParticleEmitterInstance>();
        ParticleEffectComponent* component = instance->GetOwner();
        if (component == nullptr)
        {
            return Selectable();
        }

        Selectable result = object;
        Entity* entity = component->GetEntity();
        while (entity != nullptr)
        {
            if (entity->GetSolid() == true)
            {
                result = Selectable(Any(entity));
                break;
            }
            entity = entity->GetParent();
        }

        return result;
    }

    DVASSERT(false);
    return Selectable();
}
} // namespace DAVA
