#include "REPlatform/Scene/Systems/ModifSystem.h"
#include "REPlatform/Commands/BakeTransformCommand.h"
#include "REPlatform/Commands/EntityAddCommand.h"
#include "REPlatform/Commands/EntityLockCommand.h"
#include "REPlatform/Commands/TransformCommand.h"
#include "REPlatform/DataNodes/Settings/GlobalSceneSettings.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/Scene/SceneHelper.h"
#include "REPlatform/Scene/Systems/CameraSystem.h"
#include "REPlatform/Scene/Systems/EditorParticlesSystem.h"
#include "REPlatform/Scene/Systems/HoodSystem.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"

#include <TArc/Utils/Utils.h>
#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Core/Deprecated.h>

#include <Particles/ParticleForce.h>
#include <Render/Highlevel/Landscape.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Systems/StaticOcclusionSystem.h>

namespace DAVA
{
Vector<EntityToModify> CreateEntityToModifyVector(SelectableGroup entities, Scene* scene)
{
    Vector<EntityToModify> modifEntities;
    if (entities.IsEmpty())
        return modifEntities;

    entities.RemoveObjectsWithDependantTransform();

    AABBox3 localBox;
    for (const Selectable& item : entities.GetContent())
    {
        localBox.AddPoint(item.GetLocalTransform().GetTranslation());
    }
    Vector3 averageLocalTranslation = localBox.GetCenter();

    modifEntities.reserve(entities.GetSize());
    for (const Selectable& item : entities.GetContent())
    {
        modifEntities.emplace_back();
        EntityToModify& etm = modifEntities.back();

        etm.object = item;
        etm.originalTransform = item.GetLocalTransform();

        etm.toLocalZero.BuildTranslation(-etm.originalTransform.GetTranslation());
        etm.fromLocalZero.BuildTranslation(etm.originalTransform.GetTranslation());

        etm.toWorldZero.BuildTranslation(-averageLocalTranslation);
        etm.fromWorldZero.BuildTranslation(averageLocalTranslation);

        etm.originalParentWorldTransform.Identity();

        // inverse parent world transform, and remember it
        Entity* entity = item.AsEntity();
        if ((entity != nullptr) && (entity->GetParent() != nullptr))
        {
            TransformComponent* tc = entity->GetParent()->GetComponent<TransformComponent>();
            etm.originalParentWorldTransform = tc->GetWorldMatrix();
        }
        else if (item.CanBeCastedTo<ParticleEmitterInstance>()) // special case for emitter
        {
            ParticleEmitterInstance* emitter = item.Cast<ParticleEmitterInstance>();
            ParticleEffectComponent* ownerComponent = emitter->GetOwner();
            if (ownerComponent != nullptr)
            {
                Entity* ownerEntity = ownerComponent->GetEntity();
                if (ownerEntity != nullptr)
                {
                    TransformComponent* tc = ownerEntity->GetComponent<TransformComponent>();
                    etm.originalParentWorldTransform = tc->GetWorldMatrix();
                }
            }
        }
        else if (item.CanBeCastedTo<ParticleForce>()) // and for force
        {
            ParticleForce* force = item.Cast<ParticleForce>();
            if (!force->worldAlign) // if force is world align we ignore emitter's rotation
            {
                DataContext* context = Deprecated::GetActiveContext();
                if (context != nullptr)
                {
                    EditorParticlesSystem* particleSystem = scene->GetSystem<EditorParticlesSystem>();
                    ParticleLayer* layer = particleSystem->GetForceOwner(force);
                    ParticleEffectComponent* ownerComponent = particleSystem->GetRootEmitterLayerOwner(layer)->GetOwner();
                    if (ownerComponent != nullptr)
                    {
                        Entity* ownerEntity = ownerComponent->GetEntity();
                        if (ownerEntity != nullptr)
                        {
                            TransformComponent* tc = ownerEntity->GetComponent<TransformComponent>();
                            etm.originalParentWorldTransform = tc->GetWorldMatrix();
                        }
                    }
                }
            }
        }

        etm.inversedParentWorldTransform = etm.originalParentWorldTransform;
        etm.inversedParentWorldTransform.SetTranslationVector(Vector3(0.0f, 0.0f, 0.0f));
        if (!etm.inversedParentWorldTransform.Inverse())
        {
            etm.inversedParentWorldTransform.Identity();
        }
    }
    return modifEntities;
}

void ApplyModificationToScene(Scene* scene, const Vector<EntityToModify>& entities)
{
    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(scene);
    bool transformChanged = false;
    if (std::find_if(std::begin(entities), std::end(entities),
                     [](const EntityToModify& entity)
                     {
                         return entity.originalTransform != entity.object.GetLocalTransform();
                     }) != std::end(entities))
    {
        sceneEditor->BeginBatch("Multiple transform", static_cast<uint32>(entities.size()));
        for (const EntityToModify& entity : entities)
        {
            sceneEditor->Exec(std::make_unique<TransformCommand>(entity.object, entity.originalTransform, entity.object.GetLocalTransform()));
        }
        sceneEditor->EndBatch();
    }
}

EntityModificationSystem::EntityModificationSystem(Scene* scene)
    : SceneSystem(scene)
{
    collisionSystem = scene->GetSystem<SceneCollisionSystem>();
    cameraSystem = scene->GetSystem<SceneCameraSystem>();
    hoodSystem = scene->GetSystem<HoodSystem>();

    SetTransformType(Selectable::TransformType::Disabled);
    SetModifAxis(ST_AXIS_Z);
    UpdateTransformationAxes();
}

void EntityModificationSystem::SetModifAxis(ST_Axis axis)
{
    if (axis != ST_AXIS_NONE)
    {
        curAxis = axis;
        hoodSystem->SetModifAxis(axis);
    }
}

const DAVA::Vector3& EntityModificationSystem::GetModifAxisX() const
{
    return axisX;
}

const DAVA::Vector3& EntityModificationSystem::GetModifAxisY() const
{
    return axisY;
}

const DAVA::Vector3& EntityModificationSystem::GetModifAxisZ() const
{
    return axisZ;
}

ST_Axis EntityModificationSystem::GetModifAxis() const
{
    return curAxis;
}

void EntityModificationSystem::SetTransformType(Selectable::TransformType mode)
{
    transformType = mode;
    hoodSystem->SetTransformType(mode);
}

bool EntityModificationSystem::GetModifyInLocalCoordinates() const
{
    return modifyInLocalCoordinates;
}

void EntityModificationSystem::SetModifyInLocalCoordinates(bool inLocal)
{
    modifyInLocalCoordinates = inLocal;
    UpdateTransformationAxes();
}

Selectable::TransformType EntityModificationSystem::GetTransformType() const
{
    return transformType;
}

bool EntityModificationSystem::GetLandscapeSnap() const
{
    return snapToLandscape;
}

void EntityModificationSystem::SetLandscapeSnap(bool snap)
{
    snapToLandscape = snap;
}

void EntityModificationSystem::PlaceOnLandscape(const SelectableGroup& entities)
{
    if (ModifCanStart(entities))
    {
        bool prevSnapToLandscape = snapToLandscape;

        snapToLandscape = true;
        BeginModification(entities);

        // move by z axis, so we will snap to landscape and keep x,y coords unmodified
        Vector3 newPos3d = modifStartPos3d;
        newPos3d.z += 1.0f;
        Move(newPos3d);

        ApplyModification();
        EndModification();

        snapToLandscape = prevSnapToLandscape;
    }
}

void EntityModificationSystem::ResetTransform(const SelectableGroup& entities)
{
    SceneEditor2* sceneEditor = ((SceneEditor2*)GetScene());
    if (nullptr != sceneEditor && ModifCanStart(entities))
    {
        Matrix4 zeroTransform;
        zeroTransform.Identity();

        sceneEditor->BeginBatch("Multiple transform", entities.GetSize());
        for (const Selectable& item : entities.GetContent())
        {
            sceneEditor->Exec(std::unique_ptr<Command>(new TransformCommand(item, item.GetLocalTransform(), zeroTransform)));
        }
        sceneEditor->EndBatch();
    }
}

bool EntityModificationSystem::InModifState() const
{
    return inModifState;
}

bool EntityModificationSystem::InCloneState() const
{
    return (cloneState == CLONE_NEED);
}

bool EntityModificationSystem::InCloneDoneState() const
{
    return (cloneState == CLONE_DONE);
}

bool EntityModificationSystem::Input(UIEvent* event)
{
    if (IsLocked() || (collisionSystem == nullptr))
    {
        return false;
    }

    SelectionSystem* selectionSystem = GetScene()->GetSystem<SelectionSystem>();

    Camera* camera = cameraSystem->GetCurCamera();

    // if we are not in modification state, try to find some selected item
    // that have mouse cursor at the top of it
    if (!inModifState)
    {
        // can we start modification???
        if (ModifCanStartByMouse(transformableSelection))
        {
            mouseOverSelection.Emit(static_cast<SceneEditor2*>(GetScene()), &selectionSystem->GetSelection());

            if (UIEvent::Phase::BEGAN == event->phase)
            {
                if (event->mouseButton == eMouseButtons::LEFT)
                {
                    // go to modification state
                    inModifState = true;

                    // select current hood axis as active
                    if ((transformType == Selectable::TransformType::Translation) || (transformType == Selectable::TransformType::Rotation))
                    {
                        SetModifAxis(hoodSystem->GetPassingAxis());
                    }

                    // set entities to be modified
                    BeginModification(transformableSelection);

                    // init some values, needed for modifications
                    modifStartPos3d = CamCursorPosToModifPos(camera, event->point);
                    modifStartPos2d = event->point;

                    // check if this is move with copy action
                    bool shiftPressed = IsKeyPressed(eModifierKeys::SHIFT);
                    if (shiftPressed == true && (transformType == Selectable::TransformType::Translation))
                    {
                        cloneState = CLONE_NEED;
                    }
                }
            }
        }
        else
        {
            mouseOverSelection.Emit(static_cast<SceneEditor2*>(GetScene()), nullptr);
        }
    }
    // or we are already in modification state
    else
    {
        // phase still continue
        if (event->phase == UIEvent::Phase::DRAG)
        {
            Vector3 moveOffset;
            float32 rotateAngle;
            float32 scaleForce;

            switch (transformType)
            {
            case Selectable::TransformType::Translation:
            {
                Vector3 newPos3d = CamCursorPosToModifPos(camera, event->point);
                moveOffset = Move(newPos3d);
                modified = true;
            }
            break;
            case Selectable::TransformType::Rotation:
            {
                rotateAngle = Rotate(event->point);
                modified = true;
            }
            break;
            case Selectable::TransformType::Scale:
            {
                scaleForce = Scale(event->point);
                modified = true;
            }
            break;
            default:
                break;
            }

            if (modified)
            {
                if (cloneState == CLONE_NEED)
                {
                    CloneBegin();
                    cloneState = CLONE_DONE;
                }

                // say to selection system, that selected items were modified
                selectionSystem->CancelSelection();

                // lock hood, so it wont process ui events, wont calc. scale depending on it current position
                hoodSystem->LockScale(true);
                hoodSystem->SetModifOffset(moveOffset);
                hoodSystem->SetModifRotate(rotateAngle);
                hoodSystem->SetModifScale(scaleForce);
            }
        }
        // phase ended
        else if (event->phase == UIEvent::Phase::ENDED)
        {
            if (event->mouseButton == eMouseButtons::LEFT)
            {
                if (modified)
                {
                    if (cloneState == CLONE_DONE)
                    {
                        CloneEnd();
                    }
                    else
                    {
                        ApplyModification();
                    }
                }

                UpdateTransformationAxes();
                hoodSystem->SetModifOffset(DAVA::Vector3(0, 0, 0));
                hoodSystem->SetModifRotate(0);
                hoodSystem->SetModifScale(0);
                hoodSystem->LockScale(false);

                EndModification();
                inModifState = false;
                modified = false;
                cloneState = CLONE_DONT;
            }
        }
    }
    return false;
}

void EntityModificationSystem::AddDelegate(EntityModificationSystemDelegate* delegate)
{
    delegates.push_back(delegate);
}

void EntityModificationSystem::RemoveDelegate(EntityModificationSystemDelegate* delegate)
{
    delegates.remove(delegate);
}

void EntityModificationSystem::ApplyValues(ST_Axis axis, const SelectableGroup& entities, const DAVA::Vector3& values)
{
    switch (transformType)
    {
    case Selectable::TransformType::Translation:
        ApplyMoveValues(axis, entities, values);
        break;
    case Selectable::TransformType::Rotation:
        ApplyRotateValues(axis, entities, values);
        break;
    case Selectable::TransformType::Scale:
        ApplyScaleValues(axis, entities, values);
        break;
    default:
        break;
    }
}

void EntityModificationSystem::BeginModification(const SelectableGroup& inputEntities)
{
    EndModification();
    if (inputEntities.IsEmpty())
        return;

    modifEntities = CreateEntityToModifyVector(inputEntities, GetScene());

    // remember axis vector we are rotating around
    switch (curAxis)
    {
    case ST_AXIS_X:
    case ST_AXIS_YZ:
        rotateAround = axisX;
        break;
    case ST_AXIS_Y:
    case ST_AXIS_XZ:
        rotateAround = axisY;
        break;
    case ST_AXIS_XY:
    case ST_AXIS_Z:
        rotateAround = axisZ;
        break;
    default:
        break;
    }

    // 2d axis projection we are rotating around
    modifEntitiesCenter = inputEntities.GetCommonWorldSpaceTranslationVector();
    Vector2 rotateAxis = Cam2dProjection(modifEntitiesCenter, modifEntitiesCenter + rotateAround);

    // axis dot products
    DAVA::Vector2 zeroPos = cameraSystem->GetScreenPos(modifEntitiesCenter);
    DAVA::Vector2 xPos = cameraSystem->GetScreenPos(modifEntitiesCenter + axisX);
    DAVA::Vector2 yPos = cameraSystem->GetScreenPos(modifEntitiesCenter + axisY);
    DAVA::Vector2 zPos = cameraSystem->GetScreenPos(modifEntitiesCenter + axisZ);

    Vector2 vx = xPos - zeroPos;
    Vector2 vy = yPos - zeroPos;
    Vector2 vz = zPos - zeroPos;

    crossXY = Abs(vx.CrossProduct(vy));
    crossXZ = Abs(vx.CrossProduct(vz));
    crossYZ = Abs(vy.CrossProduct(vz));

    // real rotate should be done in direction of 2dAxis normal,
    // so calculate this normal
    rotateNormal = Vector2(-rotateAxis.y, rotateAxis.x);
    if (!rotateNormal.IsZero())
    {
        rotateNormal.Normalize();
    }

    Camera* camera = cameraSystem->GetCurCamera();
    if (camera != nullptr)
    {
        isOrthoModif = camera->GetIsOrtho();
    }
}

void EntityModificationSystem::EndModification()
{
    modifEntitiesCenter.Set(0, 0, 0);
    modifEntities.clear();
    isOrthoModif = false;
}

bool EntityModificationSystem::ModifCanStart(const SelectableGroup& objects) const
{
    return !objects.IsEmpty() && objects.SupportsTransformType(transformType);
}

bool EntityModificationSystem::ModifCanStartByMouse(const SelectableGroup& objects) const
{
    if (ModifCanStart(objects) == false)
        return false;

    // we can start modification if mouse is over hood
    // on mouse is over one of currently selected items
    if (hoodSystem->GetPassingAxis() != ST_AXIS_NONE)
        return true;

    if (Deprecated::GetDataNode<GlobalSceneSettings>()->modificationByGizmoOnly == true)
        return false;

    // send this ray to collision system and get collision objects
    // check if one of got collision objects is intersected with selected items
    // if so - we can start modification
    SelectableGroup::CollectionType collisionEntities;
    collisionSystem->ObjectsRayTestFromCamera(collisionEntities);
    if (collisionEntities.empty())
        return false;

    for (const Selectable& collisionItem : collisionEntities)
    {
        for (const Selectable& selectedItem : objects.GetContent())
        {
            if (selectedItem == collisionItem)
                return true;

            Entity* selectedEntity = selectedItem.AsEntity();
            Entity* collisionEntity = collisionItem.AsEntity();
            if ((selectedEntity != nullptr) && (collisionEntity != nullptr) && selectedEntity->GetSolid())
            {
                if (IsEntityContainRecursive(selectedEntity, collisionEntity))
                    return true;
            }
        }
    }

    return false;
}

void EntityModificationSystem::ApplyModification()
{
    SceneEditor2* sceneEditor = ((SceneEditor2*)GetScene());

    if (sceneEditor == nullptr)
        return;

    ApplyModificationToScene(sceneEditor, modifEntities);
}

Vector3 EntityModificationSystem::CamCursorPosToModifPos(Camera* camera, Vector2 pos)
{
    Vector3 ret;

    if (NULL != camera)
    {
        if (camera->GetIsOrtho())
        {
            Vector3 dir = cameraSystem->GetPointDirection(pos);
            ret = Vector3(dir.x, dir.y, 0);
        }
        else
        {
            Vector3 planeNormal;
            Vector3 camPosition = cameraSystem->GetCameraPosition();
            Vector3 camToPointDirection = cameraSystem->GetPointDirection(pos);

            switch (curAxis)
            {
            case ST_AXIS_X:
            {
                if (crossXY > crossXZ)
                    planeNormal = axisZ;
                else
                    planeNormal = axisY;
            }
            break;
            case ST_AXIS_Y:
            {
                if (crossXY > crossYZ)
                    planeNormal = axisZ;
                else
                    planeNormal = axisX;
            }
            break;
            case ST_AXIS_Z:
            {
                if (crossXZ > crossYZ)
                    planeNormal = axisY;
                else
                    planeNormal = axisX;
            }
            break;
            case ST_AXIS_XZ:
                planeNormal = axisY;
                break;
            case ST_AXIS_YZ:
                planeNormal = axisX;
                break;
            case ST_AXIS_XY:
            default:
                planeNormal = axisZ;
                break;
            }

            Plane plane(planeNormal, modifEntitiesCenter);
            float32 distance = FLT_MAX;

            plane.IntersectByRay(camPosition, camToPointDirection, distance);
            ret = camPosition + (camToPointDirection * distance);
        }
    }

    return ret;
}

Vector2 EntityModificationSystem::Cam2dProjection(const Vector3& from, const Vector3& to)
{
    Vector2 axisBegin = cameraSystem->GetScreenPos(from);
    Vector2 axisEnd = cameraSystem->GetScreenPos(to);
    Vector2 ret = axisEnd - axisBegin;

    if (ret.IsZero())
    {
        ret = Vector2(1.0f, 1.0f);
    }

    ret.Normalize();
    return ret;
}

Vector3 EntityModificationSystem::Move(const Vector3& newPos3d)
{
    using namespace DAVA;

    Vector3 moveOffset;
    Vector3 modifPosWithLockedAxis = modifStartPos3d;
    Vector3 deltaPos3d = newPos3d - modifStartPos3d;

    switch (curAxis)
    {
    case ST_AXIS_X:
    {
        float32 advanceOnX = axisX.DotProduct(deltaPos3d);
        modifPosWithLockedAxis.x += axisX.x * advanceOnX;
        modifPosWithLockedAxis.y += axisX.y * advanceOnX;
        modifPosWithLockedAxis.z += axisX.z * advanceOnX;
        break;
    }
    case ST_AXIS_Y:
    {
        float32 advanceOnY = axisY.DotProduct(deltaPos3d);
        modifPosWithLockedAxis.x += axisY.x * advanceOnY;
        modifPosWithLockedAxis.y += axisY.y * advanceOnY;
        modifPosWithLockedAxis.z += axisY.z * advanceOnY;
        break;
    }
    case ST_AXIS_Z:
    {
        if (!isOrthoModif)
        {
            float32 advanceOnZ = axisZ.DotProduct(deltaPos3d);
            modifPosWithLockedAxis.x += axisZ.x * advanceOnZ;
            modifPosWithLockedAxis.y += axisZ.y * advanceOnZ;
            modifPosWithLockedAxis.z += axisZ.z * advanceOnZ;
        }
        break;
    }
    case ST_AXIS_XY:
        modifPosWithLockedAxis.x = newPos3d.x;
        modifPosWithLockedAxis.y = newPos3d.y;
        modifPosWithLockedAxis.z = newPos3d.z;
        break;
    case ST_AXIS_XZ:
        if (!isOrthoModif)
        {
            modifPosWithLockedAxis.x = newPos3d.x;
            modifPosWithLockedAxis.y = newPos3d.y;
            modifPosWithLockedAxis.z = newPos3d.z;
        }
        break;
    case ST_AXIS_YZ:
        if (!isOrthoModif)
        {
            modifPosWithLockedAxis.x = newPos3d.x;
            modifPosWithLockedAxis.y = newPos3d.y;
            modifPosWithLockedAxis.z = newPos3d.z;
        }
        break;
    default:
        break;
    }

    moveOffset = modifPosWithLockedAxis - modifStartPos3d;

    for (EntityToModify& etm : modifEntities)
    {
        Matrix4 moveModification = Matrix4::MakeTranslation(moveOffset * etm.inversedParentWorldTransform);
        Transform newLocalTransform = etm.originalTransform * moveModification;

        if (snapToLandscape)
        {
            newLocalTransform = newLocalTransform * SnapToLandscape(newLocalTransform.GetTranslation(), etm.originalParentWorldTransform);
        }

        etm.object.SetLocalTransform(newLocalTransform);
    }

    return moveOffset;
}

float32 EntityModificationSystem::Rotate(const Vector2& newPos2d)
{
    Vector2 rotateLength = newPos2d - modifStartPos2d;
    float32 rotateForce = -(rotateNormal.DotProduct(rotateLength)) / 70.0f;

    for (EntityToModify& etm : modifEntities)
    {
        Matrix4 rotateModification = Matrix4::MakeRotation(rotateAround, -rotateForce);
        Matrix4& toZero = (curPivotPoint == Selectable::TransformPivot::ObjectCenter) ? etm.toLocalZero : etm.toWorldZero;
        Matrix4& fromZero = (curPivotPoint == Selectable::TransformPivot::ObjectCenter) ? etm.fromLocalZero : etm.fromWorldZero;
        etm.object.SetLocalTransform(etm.originalTransform * toZero * rotateModification * fromZero);
    }

    return rotateForce;
}

void EntityModificationSystem::SetPivotPoint(Selectable::TransformPivot pp)
{
    curPivotPoint = pp;
}

Selectable::TransformPivot EntityModificationSystem::GetPivotPoint() const
{
    return curPivotPoint;
}

float32 EntityModificationSystem::Scale(const Vector2& newPos2d)
{
    Vector2 scaleDir = (newPos2d - modifStartPos2d);
    float32 scaleForce = 1.0f - (scaleDir.y / 70.0f);

    if (scaleForce >= 0.0f)
    {
        for (EntityToModify& etm : modifEntities)
        {
            Vector3 scaleVector(scaleForce, scaleForce, scaleForce);
            Matrix4 scaleModification = Matrix4::MakeScale(scaleVector);
            Matrix4& toZero = (curPivotPoint == Selectable::TransformPivot::ObjectCenter) ? etm.toLocalZero : etm.toWorldZero;
            Matrix4& fromZero = (curPivotPoint == Selectable::TransformPivot::ObjectCenter) ? etm.fromLocalZero : etm.fromWorldZero;
            etm.object.SetLocalTransform(etm.originalTransform * toZero * scaleModification * fromZero);
        }
    }

    return scaleForce;
}

Matrix4 EntityModificationSystem::SnapToLandscape(const Vector3& point, const Matrix4& originalParentTransform) const
{
    Matrix4 ret;
    ret.Identity();

    Landscape* landscape = collisionSystem->GetCurrentLandscape();
    if (NULL != landscape)
    {
        Vector3 resPoint;
        Vector3 realPoint = point * originalParentTransform;

        if (landscape->PlacePoint(realPoint, resPoint))
        {
            resPoint = resPoint - realPoint;
            ret.SetTranslationVector(resPoint);
        }
    }

    return ret;
}

bool EntityModificationSystem::IsEntityContainRecursive(const Entity* entity, const Entity* child) const
{
    bool ret = false;

    if (NULL != entity && NULL != child)
    {
        for (int i = 0; !ret && i < entity->GetChildrenCount(); ++i)
        {
            if (child == entity->GetChild(i))
            {
                ret = true;
            }
            else
            {
                ret = IsEntityContainRecursive(entity->GetChild(i), child);
            }
        }
    }

    return ret;
}

void EntityModificationSystem::CloneBegin()
{
    // remove modif entities that are children for other modif entities
    for (uint32 i = 0; i < modifEntities.size(); ++i)
    {
        Entity* iEntity = modifEntities[i].object.AsEntity();
        for (uint32 j = 0; (iEntity != nullptr) && (j < modifEntities.size()); ++j)
        {
            if (i == j)
                continue;

            Entity* jEntity = modifEntities[j].object.AsEntity();
            if ((jEntity != nullptr) && SceneHelper::IsEntityChildRecursive(jEntity, iEntity))
            {
                RemoveExchangingWithLast(modifEntities, i);
                --i;
                break;
            }
        }
    }

    if (modifEntities.empty())
        return;

    uint32 count = static_cast<uint32>(modifEntities.size());
    clonedEntities.reserve(count);

    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
    sceneEditor->BeginBatch("Clone", count);
    for (const EntityToModify& item : modifEntities)
    {
        Entity* origEntity = item.object.AsEntity();
        if (origEntity == nullptr)
            continue;

        for (EntityModificationSystemDelegate* delegate : delegates)
        {
            delegate->WillClone(origEntity);
        }
        Entity* newEntity = origEntity->Clone();
        for (EntityModificationSystemDelegate* delegate : delegates)
        {
            delegate->DidCloned(origEntity, newEntity);
        }

        TransformComponent* tc = newEntity->GetComponent<TransformComponent>();
        tc->SetLocalTransform(item.originalTransform);

        Scene* scene = origEntity->GetScene();
        if (scene != nullptr)
        {
            StaticOcclusionSystem* occlusionSystem = scene->staticOcclusionSystem;
            DVASSERT(occlusionSystem);
            occlusionSystem->InvalidateOcclusionIndicesRecursively(newEntity);
        }

        EntityModificationSystemDelegate* addingDelegate = nullptr;
        for (EntityModificationSystemDelegate* d : delegates)
        {
            if (d->HasCustomClonedAddading(origEntity))
            {
                addingDelegate = d;
                break;
            }
        }

        if (addingDelegate == nullptr)
        {
            // and add it once again with command
            sceneEditor->Exec(std::unique_ptr<Command>(new EntityAddCommand(newEntity, origEntity->GetParent())));
        }
        else
        {
            addingDelegate->PerformAdding(origEntity, newEntity);
        }

        clonedEntities.push_back(newEntity);
    }

    sceneEditor->EndBatch();
}

void EntityModificationSystem::CloneEnd()
{
    if (modifEntities.size() > 0 && clonedEntities.size() == modifEntities.size())
    {
        uint32 count = static_cast<uint32>(modifEntities.size());

        // we just moved original objects. Now we should return them back
        // to there original positions and move cloned object to the new positions
        for (uint32 i = 0; i < count; ++i)
        {
            // remember new transform
            Transform newLocalTransform = modifEntities[i].object.GetLocalTransform();

            // return original entity to original pos
            modifEntities[i].object.SetLocalTransform(modifEntities[i].originalTransform);

            // move cloned entity to new pos
            TransformComponent* tc = clonedEntities[i]->GetComponent<TransformComponent>();
            tc->SetLocalTransform(newLocalTransform);

            // make cloned entity selected
            SafeRelease(clonedEntities[i]);
        }
    }

    clonedEntities.clear();
}

void EntityModificationSystem::PrepareForRemove()
{
}

void EntityModificationSystem::MovePivotZero(const SelectableGroup& entities)
{
    if (ModifCanStart(entities))
    {
        BakeGeometry(entities, BAKE_ZERO_PIVOT);
    }
}

void EntityModificationSystem::MovePivotCenter(const SelectableGroup& entities)
{
    if (ModifCanStart(entities))
    {
        BakeGeometry(entities, BAKE_CENTER_PIVOT);
    }
}

void EntityModificationSystem::LockTransform(const SelectableGroup& entities, bool lock)
{
    SceneEditor2* sceneEditor = ((SceneEditor2*)GetScene());
    if (sceneEditor == nullptr)
    {
        return;
    }

    uint32 count = static_cast<uint32>(entities.GetSize());
    sceneEditor->BeginBatch("Lock entities", count);
    for (Entity* entity : entities.ObjectsOfType<Entity>())
    {
        sceneEditor->Exec(std::unique_ptr<Command>(new EntityLockCommand(entity, lock)));
    }
    sceneEditor->EndBatch();
}

void EntityModificationSystem::BakeGeometry(const SelectableGroup& entities, BakeMode mode)
{
    SceneEditor2* sceneEditor = ((SceneEditor2*)GetScene());
    if ((sceneEditor == nullptr) && (entities.GetSize() != 1))
        return;

    Entity* entity = entities.GetFirst().AsEntity();
    if (entity == nullptr)
        return;

    const char* commandMessage = nullptr;
    switch (mode)
    {
    case BAKE_ZERO_PIVOT:
        commandMessage = "Move pivot point to zero";
        break;
    case BAKE_CENTER_PIVOT:
        commandMessage = "Move pivot point to center";
        break;
    default:
        DVASSERT(0, "Unknown bake mode");
        return;
    }

    RenderObject* ro = GetRenderObject(entity);
    if (ro != nullptr)
    {
        Set<Entity*> entityList;
        SearchEntitiesWithRenderObject(ro, sceneEditor, entityList);

        if (entityList.size() > 0)
        {
            Matrix4 bakeTransform;

            switch (mode)
            {
            case BAKE_ZERO_PIVOT:
            {
                TransformComponent* tc = entity->GetComponent<TransformComponent>();
                bakeTransform = tc->GetLocalMatrix();
            }
            break;
            case BAKE_CENTER_PIVOT:
                bakeTransform.SetTranslationVector(-ro->GetBoundingBox().GetCenter());
                break;
            }

            sceneEditor->BeginBatch(commandMessage, static_cast<uint32>(entityList.size()));

            // bake render object
            sceneEditor->Exec(std::unique_ptr<Command>(new BakeGeometryCommand(entity, ro, bakeTransform)));

            // inverse bake to be able to move object on same place
            // after it geometry was baked
            Matrix4 afterBakeTransform = bakeTransform;
            afterBakeTransform.Inverse();

            // for entities with same render object set new transform
            // to make them match their previous position
            for (Entity* en : entityList)
            {
                TransformComponent* tc = en->GetComponent<TransformComponent>();
                Matrix4 origTransform = tc->GetLocalMatrix();
                Matrix4 newTransform = afterBakeTransform * origTransform;
                sceneEditor->Exec(std::unique_ptr<Command>(new TransformCommand(Selectable(en), origTransform, newTransform)));

                // also modify childs transform to make them be at
                // right position after parent entity changed
                for (int32 i = 0; i < en->GetChildrenCount(); ++i)
                {
                    Entity* childEntity = en->GetChild(i);

                    TransformComponent* childTC = childEntity->GetComponent<TransformComponent>();
                    Matrix4 childOrigTransform = childTC->GetLocalMatrix();
                    Matrix4 childNewTransform = childOrigTransform * bakeTransform;
                    sceneEditor->Exec(std::unique_ptr<Command>(new TransformCommand(Selectable(childEntity), childOrigTransform, childNewTransform)));
                }
            }

            sceneEditor->EndBatch();
        }
    }
    else if (entity->GetChildrenCount() > 0) // just modify child entities
    {
        Vector3 newPivotPos = Vector3(0, 0, 0);

        if (mode == BAKE_CENTER_PIVOT)
        {
            SceneCollisionSystem* collisionSystem = GetScene()->GetSystem<SceneCollisionSystem>();
            AABBox3 bbox = collisionSystem->GetUntransformedBoundingBox(entity);
            DVASSERT(!bbox.IsEmpty());
            newPivotPos = bbox.GetCenter();
        }

        uint32 count = static_cast<uint32>(entity->GetChildrenCount());
        sceneEditor->BeginBatch(commandMessage, count + 1);

        // transform parent entity
        TransformComponent* tc = entity->GetComponent<TransformComponent>();
        Transform transform;
        transform.SetTranslation(newPivotPos - tc->GetLocalTransform().GetTranslation());
        sceneEditor->Exec(std::unique_ptr<Command>(new TransformCommand(Selectable(entity), tc->GetLocalTransform(), tc->GetLocalTransform() * transform)));

        // transform child entities with inversed parent transformation
        transform.Inverse();
        for (uint32 i = 0; i < count; ++i)
        {
            Entity* childEntity = entity->GetChild(i);
            TransformComponent* childTC = childEntity->GetComponent<TransformComponent>();

            sceneEditor->Exec(std::unique_ptr<Command>(new TransformCommand(Selectable(childEntity), childTC->GetLocalTransform(), childTC->GetLocalTransform() * transform)));
        }

        sceneEditor->EndBatch();
    }
}

void EntityModificationSystem::SearchEntitiesWithRenderObject(RenderObject* ro, Entity* root, Set<Entity*>& result)
{
    if (NULL != root)
    {
        int32 count = root->GetChildrenCount();
        for (int32 i = 0; i < count; ++i)
        {
            Entity* en = root->GetChild(i);
            RenderObject* enRenderObject = GetRenderObject(en);

            bool isSame = false;
            if (NULL != enRenderObject && ro->GetRenderBatchCount() == enRenderObject->GetRenderBatchCount())
            {
                // if renderObjects has same number of render batches we also should
                // check if polygon groups used inside that render batches are completely identical
                // but we should deal with the fact, that polygon groups order can differ
                for (uint32 j = 0; j < enRenderObject->GetRenderBatchCount(); ++j)
                {
                    bool found = false;
                    PolygonGroup* pg = enRenderObject->GetRenderBatch(j)->GetPolygonGroup();

                    for (uint32 k = 0; k < ro->GetRenderBatchCount(); ++k)
                    {
                        if (ro->GetRenderBatch(k)->GetPolygonGroup() == pg)
                        {
                            found = true;
                            break;
                        }
                    }

                    isSame = found;
                    if (!found)
                    {
                        break;
                    }
                }
            }

            if (isSame)
            {
                result.insert(en);
            }
            else if (en->GetChildrenCount() > 0)
            {
                SearchEntitiesWithRenderObject(ro, en, result);
            }
        }
    }
}

bool EntityModificationSystem::AllowPerformSelectionHavingCurrent(const SelectableGroup& currentSelection)
{
    return (transformType == Selectable::TransformType::Disabled) || !ModifCanStartByMouse(currentSelection);
}

bool EntityModificationSystem::AllowChangeSelectionReplacingCurrent(const SelectableGroup& currentSelection, const SelectableGroup& newSelection)
{
    return true;
}

void EntityModificationSystem::ApplyMoveValues(ST_Axis axis, const SelectableGroup& selection, const DAVA::Vector3& values)
{
    using namespace DAVA;

    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
    sceneEditor->BeginBatch("Multiple move", selection.GetSize());

    for (const Selectable& item : selection.GetContent())
    {
        Transform origMatrix = item.GetLocalTransform();
        Vector3 newPos = origMatrix.GetTranslation();

        if (pivotMode == PivotAbsolute)
        {
            if (axis & ST_AXIS_X)
            {
                newPos.x = values.x;
            }

            if (axis & ST_AXIS_Y)
            {
                newPos.y = values.y;
            }

            if ((axis & ST_AXIS_Z) && !snapToLandscape)
            {
                newPos.z = values.z;
            }
        }
        else
        {
            if (axis == ST_AXIS_X)
            {
                newPos = newPos + axisX * values.x;
            }

            if (axis == ST_AXIS_Y)
            {
                newPos = newPos + axisY * values.y;
            }

            if (axis == ST_AXIS_Z && snapToLandscape == false)
            {
                newPos = newPos + axisZ * values.z;
            }
        }

        Transform newMatrix = origMatrix;
        newMatrix.SetTranslation(newPos);
        sceneEditor->Exec(std::unique_ptr<Command>(new TransformCommand(item, origMatrix, newMatrix)));
    }
    sceneEditor->EndBatch();
}

void EntityModificationSystem::ApplyRotateValues(ST_Axis axis, const SelectableGroup& selection, const DAVA::Vector3& values)
{
    float32 x = DegToRad(values.x);
    float32 y = DegToRad(values.y);
    float32 z = DegToRad(values.z);

    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
    sceneEditor->BeginBatch("Multiple rotate", selection.GetSize());

    for (const Selectable& item : selection.GetContent())
    {
        Transform origTransform = item.GetLocalTransform();

        Quaternion rotation;
        Vector3 euler = origTransform.GetRotation().GetEuler();

        if (pivotMode == PivotRelative)
        {
            euler *= 0.0f;
        }

        switch (axis)
        {
        case ST_AXIS_X:
            rotation.Construct(Vector3(1.f, 0, 0), x - euler.x);
            break;
        case ST_AXIS_Y:
            rotation.Construct(Vector3(0, 1.f, 0), y - euler.y);
            break;
        case ST_AXIS_Z:
            rotation.Construct(Vector3(0, 0, 1.f), z - euler.z);
            break;
        default:
            DVASSERT(0, "Unable to rotate around several axis at once");
            break;
        }

        Transform newTransform = origTransform;
        newTransform.SetRotation(origTransform.GetRotation() * rotation);

        sceneEditor->Exec(std::unique_ptr<Command>(new TransformCommand(item, origTransform, newTransform)));
    }

    sceneEditor->EndBatch();
}

void EntityModificationSystem::ApplyScaleValues(ST_Axis axis, const SelectableGroup& selection, const DAVA::Vector3& values)
{
    float32 axisScaleValue = 1.0f;

    switch (axis)
    {
    case ST_AXIS_X:
        axisScaleValue = values.x;
        break;
    case ST_AXIS_Y:
        axisScaleValue = values.y;
        break;
    case ST_AXIS_Z:
        axisScaleValue = values.z;
        break;
    default:
        DVASSERT(0, "Scaling must be uniform, unable to scale via several axis");
        break;
    }

    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
    sceneEditor->BeginBatch("Multiple scale", selection.GetSize());

    for (const Selectable& item : selection.GetContent())
    {
        Transform origTransform = item.GetLocalTransform();

        float32 scaleValue = axisScaleValue;
        Vector3 scale = origTransform.GetScale();
        if (pivotMode == PivotAbsolute)
        {
            scaleValue = (scale.x < std::numeric_limits<float>::epsilon()) ? 0.0f : (scaleValue / scale.x);
        }

        Transform newTransform = origTransform;
        newTransform.SetScale(origTransform.GetScale() * scaleValue);

        sceneEditor->Exec(std::unique_ptr<Command>(new TransformCommand(item, origTransform, newTransform)));
    }

    sceneEditor->EndBatch();
}

void EntityModificationSystem::SetTransformableSelection(const SelectableGroup& newTransformableSelection)
{
    transformableSelection = newTransformableSelection;
    UpdateTransformationAxes();
}

void EntityModificationSystem::UpdateTransformationAxes() const
{
    if (modifyInLocalCoordinates == true)
    {
        CalculateMedianAxes(transformableSelection, axisX, axisY, axisZ);
    }
    else
    {
        axisX = DAVA::Vector3(1, 0, 0);
        axisY = DAVA::Vector3(0, 1, 0);
        axisZ = DAVA::Vector3(0, 0, 1);
    }

    hoodSystem->SetAxes(axisX, axisY, axisZ);
}

void EntityModificationSystem::CalculateMedianAxes(const SelectableGroup& selection, DAVA::Vector3& axisX, DAVA::Vector3& axisY, DAVA::Vector3& axisZ) const
{
    axisX = axisY = axisZ = { 0, 0, 0 };
    for (const Selectable& item : selection.GetContent())
    {
        DAVA::Matrix4 t = TransformUtils::ToMatrix(item.GetLocalTransform());
        axisX += DAVA::Vector3(t._data[0][0], t._data[0][1], t._data[0][2]);
        axisY += DAVA::Vector3(t._data[1][0], t._data[1][1], t._data[1][2]);
        axisZ += DAVA::Vector3(t._data[2][0], t._data[2][1], t._data[2][2]);
    }

    if (axisX.IsZero() || axisY.IsZero() || axisZ.IsZero())
    {
        axisX = DAVA::Vector3(1, 0, 0);
        axisY = DAVA::Vector3(0, 1, 0);
        axisZ = DAVA::Vector3(0, 0, 1);
    }
    else
    {
        axisX.Normalize();
        axisY.Normalize();
        axisZ.Normalize();
    }
}

const SelectableGroup& EntityModificationSystem::GetTransformableSelection() const
{
    return transformableSelection;
}
} // namespace DAVA
