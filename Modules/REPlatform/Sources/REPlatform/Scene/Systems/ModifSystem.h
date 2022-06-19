#pragma once

#include "REPlatform/Commands/RECommand.h"
#include "REPlatform/DataNodes/SelectableGroup.h"
#include "REPlatform/Scene/Systems/SystemDelegates.h"
#include "REPlatform/Scene/SceneTypes.h"

#include <Entity/SceneSystem.h>
#include <Functional/Signal.h>
#include <Math/Matrix4.h>
#include <Math/Transform.h>
#include <Render/Highlevel/RenderObject.h>
#include <Scene3D/Entity.h>
#include <UI/UIEvent.h>

namespace DAVA
{
class SceneCollisionSystem;
class SceneCameraSystem;
class HoodSystem;
class SceneEditor2;

struct EntityToModify
{
    Selectable object;

    Matrix4 inversedParentWorldTransform;
    Matrix4 originalParentWorldTransform;
    Transform originalTransform;

    Matrix4 toLocalZero;
    Matrix4 fromLocalZero;
    Matrix4 toWorldZero;
    Matrix4 fromWorldZero;
};

Vector<EntityToModify> CreateEntityToModifyVector(SelectableGroup entities, Scene* scene);
void ApplyModificationToScene(Scene* scene, const Vector<EntityToModify>& entities);

class EntityModificationSystem : public SceneSystem, public SelectionSystemDelegate
{
    friend class SceneEditor2;

public:
    EntityModificationSystem(Scene* scene);

    ST_Axis GetModifAxis() const;
    void SetModifAxis(ST_Axis axis);

    const DAVA::Vector3& GetModifAxisX() const;
    const DAVA::Vector3& GetModifAxisY() const;
    const DAVA::Vector3& GetModifAxisZ() const;

    Selectable::TransformType GetTransformType() const;
    void SetTransformType(Selectable::TransformType mode);

    bool GetModifyInLocalCoordinates() const;
    void SetModifyInLocalCoordinates(bool inLocal);

    bool GetLandscapeSnap() const;
    void SetLandscapeSnap(bool snap);

    enum PivotMode : DAVA::uint32
    {
        PivotAbsolute,
        PivotRelative,
    };

    void SetPivotMode(PivotMode);
    PivotMode GetPivotMode() const;

    void PlaceOnLandscape(const SelectableGroup& entities);
    void ResetTransform(const SelectableGroup& entities);

    void MovePivotZero(const SelectableGroup& entities);
    void MovePivotCenter(const SelectableGroup& entities);

    void LockTransform(const SelectableGroup& entities, bool lock);

    bool InModifState() const;
    bool InCloneState() const;
    bool InCloneDoneState() const;

    bool ModifCanStart(const SelectableGroup& objects) const;
    bool ModifCanStartByMouse(const SelectableGroup& objects) const;

    void PrepareForRemove() override;

    bool Input(UIEvent* event) override;

    void AddDelegate(EntityModificationSystemDelegate* delegate);
    void RemoveDelegate(EntityModificationSystemDelegate* delegate);

    void ApplyValues(ST_Axis axis, const SelectableGroup& entities, const DAVA::Vector3& values);
    void ApplyMoveValues(ST_Axis axis, const SelectableGroup& entities, const DAVA::Vector3& values);
    void ApplyRotateValues(ST_Axis axis, const SelectableGroup& entities, const DAVA::Vector3& values);
    void ApplyScaleValues(ST_Axis axis, const SelectableGroup& entities, const DAVA::Vector3& values);

    void SetTransformableSelection(const SelectableGroup&);
    const SelectableGroup& GetTransformableSelection() const;

    void SetPivotPoint(Selectable::TransformPivot pp);
    Selectable::TransformPivot GetPivotPoint() const;

    Signal<SceneEditor2*, const SelectableGroup*> mouseOverSelection;

private:
    enum CloneState : uint32
    {
        CLONE_DONT,
        CLONE_NEED,
        CLONE_DONE
    };

    enum BakeMode : uint32
    {
        BAKE_ZERO_PIVOT,
        BAKE_CENTER_PIVOT
    };

    void BeginModification(const SelectableGroup& entities);
    void EndModification();

    void CloneBegin();
    void CloneEnd();

    void ApplyModification();

    Vector3 CamCursorPosToModifPos(Camera* camera, Vector2 pos);
    Vector2 Cam2dProjection(const Vector3& from, const Vector3& to);

    Vector3 Move(const Vector3& newPos3d);
    float32 Rotate(const Vector2& newPos2d);
    float32 Scale(const Vector2& newPos2d);
    void BakeGeometry(const SelectableGroup& entities, BakeMode mode);
    void SearchEntitiesWithRenderObject(RenderObject* ro, Entity* root, Set<Entity*>& result);

    Matrix4 SnapToLandscape(const Vector3& point, const Matrix4& originalParentTransform) const;
    bool IsEntityContainRecursive(const Entity* entity, const Entity* child) const;

    bool AllowPerformSelectionHavingCurrent(const SelectableGroup& currentSelection) override;
    bool AllowChangeSelectionReplacingCurrent(const SelectableGroup& currentSelection, const SelectableGroup& newSelection) override;

    void UpdateTransformationAxes() const;

    void CalculateMedianAxes(const SelectableGroup& selection, DAVA::Vector3& axisX, DAVA::Vector3& axisY, DAVA::Vector3& axisZ) const;

private:
    SceneCollisionSystem* collisionSystem = nullptr;
    SceneCameraSystem* cameraSystem = nullptr;
    HoodSystem* hoodSystem = nullptr;

    // entities to modify
    Vector<EntityToModify> modifEntities;
    Vector<Entity*> clonedEntities;
    List<EntityModificationSystemDelegate*> delegates;

    mutable SelectableGroup currentSelection;
    mutable SelectableGroup transformableSelection;

    // values calculated, when starting modification
    Vector3 modifEntitiesCenter;
    Vector3 modifStartPos3d;
    Vector2 modifStartPos2d;
    Vector2 rotateNormal;
    Vector3 rotateAround;
    float32 crossXY = 0.0f;
    float32 crossXZ = 0.0f;
    float32 crossYZ = 0.0f;
    mutable Vector3 axisX = { 0, 0, 0 };
    mutable Vector3 axisY = { 0, 0, 0 };
    mutable Vector3 axisZ = { 0, 0, 0 };

    CloneState cloneState = CloneState::CLONE_DONT;
    Selectable::TransformType transformType = Selectable::TransformType::Disabled;
    ST_Axis curAxis = ST_Axis::ST_AXIS_NONE;
    bool modifyInLocalCoordinates = false;
    PivotMode pivotMode = PivotAbsolute;

    Selectable::TransformPivot curPivotPoint = Selectable::TransformPivot::CommonCenter;

    bool inModifState = false;
    bool isOrthoModif = false;
    bool modified = false;
    bool snapToLandscape = false;
};

inline void EntityModificationSystem::SetPivotMode(PivotMode mode)
{
    pivotMode = mode;
}

inline EntityModificationSystem::PivotMode EntityModificationSystem::GetPivotMode() const
{
    return pivotMode;
}
} // namespace DAVA
