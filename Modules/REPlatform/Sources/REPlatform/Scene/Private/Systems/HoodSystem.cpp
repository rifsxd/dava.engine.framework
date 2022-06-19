#include "REPlatform/Scene/Systems/HoodSystem.h"
#include "REPlatform/Scene/Systems/ModifSystem.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Systems/CameraSystem.h"
#include "REPlatform/Scene/Systems/TextDrawSystem.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/DataNodes/Settings/GlobalSceneSettings.h"

#include <TArc/Core/Deprecated.h>

#include <Base/AlignedAllocator.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
class SceneCollisionDebugDrawer final : public btIDebugDraw
{
public:
    SceneCollisionDebugDrawer(DAVA::RenderHelper* drawer_)
        : dbgMode(0)
        , drawer(drawer_)
    {
    }

    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override
    {
        DAVA::Vector3 davaFrom(from.x(), from.y(), from.z());
        DAVA::Vector3 davaTo(to.x(), to.y(), to.z());
        DAVA::Color davaColor(color.x(), color.y(), color.z(), 1.0f);

        drawer->DrawLine(davaFrom, davaTo, davaColor, DAVA::RenderHelper::DRAW_WIRE_DEPTH);
    }

    void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override
    {
        DAVA::Color davaColor(color.x(), color.y(), color.z(), 1.0f);
        drawer->DrawIcosahedron(DAVA::Vector3(PointOnB.x(), PointOnB.y(), PointOnB.z()), distance / 20.f, davaColor, DAVA::RenderHelper::DRAW_SOLID_DEPTH);
    }

    void reportErrorWarning(const char* warningString) override
    {
    }
    void draw3dText(const btVector3& location, const char* textString) override
    {
    }
    void setDebugMode(int debugMode) override
    {
        dbgMode = debugMode;
    }
    int getDebugMode() const override
    {
        return dbgMode;
    }

protected:
    int dbgMode;
    DAVA::RenderHelper* drawer;
};

HoodSystem::HoodSystem(Scene* scene)
    : SceneSystem(scene)
    , moveHood(MoveHood::Create())
    , rotateHood(RotateHood::Create())
    , scaleHood(ScaleHood::Create())
    , normalHood(NormalHood::Create())
{
    btVector3 worldMin(-1000, -1000, -1000);
    btVector3 worldMax(1000, 1000, 1000);

    collConfiguration = new btDefaultCollisionConfiguration();
    collDispatcher = CreateObjectAligned<btCollisionDispatcher, 16>(collConfiguration);
    collBroadphase = new btAxisSweep3(worldMin, worldMax);
    collDebugDraw = new SceneCollisionDebugDrawer(scene->GetRenderSystem()->GetDebugDrawer());
    collDebugDraw->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
    collWorld = new btCollisionWorld(collDispatcher, collBroadphase, collConfiguration);
    collWorld->setDebugDrawer(collDebugDraw);

    SetModifAxis(ST_AXIS_X);
    SetTransformType(Selectable::TransformType::Translation);

    moveHood->colorX = Color(1, 0, 0, 1);
    moveHood->colorY = Color(0, 1, 0, 1);
    moveHood->colorZ = Color(0, 0, 1, 1);
    moveHood->colorS = Color(1, 1, 0, 1);

    rotateHood->colorX = Color(1, 0, 0, 1);
    rotateHood->colorY = Color(0, 1, 0, 1);
    rotateHood->colorZ = Color(0, 0, 1, 1);
    rotateHood->colorS = Color(1, 1, 0, 1);

    scaleHood->colorX = Color(1, 0, 0, 1);
    scaleHood->colorY = Color(0, 1, 0, 1);
    scaleHood->colorZ = Color(0, 0, 1, 1);
    scaleHood->colorS = Color(1, 1, 0, 1);

    normalHood->colorX = Color(0.7f, 0.3f, 0.3f, 1);
    normalHood->colorY = Color(0.3f, 0.7f, 0.3f, 1);
    normalHood->colorZ = Color(0.3f, 0.3f, 0.7f, 1);
    normalHood->colorS = Color(0, 0, 0, 1);

    cameraSystem = scene->GetSystem<DAVA::SceneCameraSystem>();
}

HoodSystem::~HoodSystem()
{
    delete collWorld;
    delete collDebugDraw;
    delete collBroadphase;
    DestroyObjectAligned(collDispatcher);
    delete collConfiguration;
}

Vector3 HoodSystem::GetPosition() const
{
    return (curPos + modifOffset);
}

void HoodSystem::SetPosition(const Vector3& pos)
{
    if (!IsLocked() && !lockedScale)
    {
        if (curPos != pos || !modifOffset.IsZero())
        {
            curPos = pos;
            ResetModifValues();

            if (NULL != curHood)
            {
                curHood->UpdatePos(curPos);
                normalHood->UpdatePos(curPos);

                collWorld->updateAabbs();
            }
        }
    }
}

void HoodSystem::SetModifOffset(const Vector3& offset)
{
    if (!IsLocked())
    {
        moveHood->modifOffset = offset;

        if (modifOffset != offset)
        {
            modifOffset = offset;

            if (NULL != curHood)
            {
                curHood->UpdatePos(curPos + modifOffset);
                normalHood->UpdatePos(curPos + modifOffset);

                collWorld->updateAabbs();
            }
        }
    }
}

void HoodSystem::SetModifRotate(const float32& angle)
{
    if (!IsLocked())
    {
        static const DAVA::float32 eps = 0.001f;
        rotateHood->modifRotate = (fabs(angle) < eps) ? 0.f : angle;
    }
}

void HoodSystem::SetModifScale(const float32& scale)
{
    if (!IsLocked())
    {
        scaleHood->modifScale = scale;
    }
}

void HoodSystem::SetScale(float32 scale)
{
    if (!IsLocked())
    {
        scale = scale * Deprecated::GetDataNode<GlobalSceneSettings>()->gizmoScale;

        if (curScale != scale && 0 != scale)
        {
            curScale = scale;

            if (NULL != curHood)
            {
                curHood->UpdateScale(curScale);
                normalHood->UpdateScale(curScale);

                collWorld->updateAabbs();
            }
        }
    }
}

float32 HoodSystem::GetScale() const
{
    return curScale;
}

void HoodSystem::SetTransformType(Selectable::TransformType mode)
{
    if (!IsLocked())
    {
        if (curMode != mode)
        {
            if (NULL != curHood)
            {
                RemCollObjects(curHood->collObjects);
            }

            curMode = mode;
            switch (mode)
            {
            case Selectable::TransformType::Translation:
                curHood = moveHood.get();
                break;
            case Selectable::TransformType::Scale:
                curHood = scaleHood.get();
                break;
            case Selectable::TransformType::Rotation:
                curHood = rotateHood.get();
                break;
            default:
                curHood = normalHood.get();
                break;
            }

            if (curHood)
            {
                curHood->UpdatePos(curPos + modifOffset);
                curHood->UpdateScale(curScale);
                AddCollObjects(curHood->collObjects);
            }

            collWorld->updateAabbs();
        }
    }
}

Selectable::TransformType HoodSystem::GetTransformType() const
{
    if (lockedModif)
    {
        return Selectable::TransformType::Disabled;
    }

    return curMode;
}

void HoodSystem::SetVisible(bool visible)
{
    if (!IsLocked())
    {
        isVisible = visible;
    }
}

bool HoodSystem::IsVisible() const
{
    return isVisible;
}

void HoodSystem::AddCollObjects(const Vector<std::unique_ptr<HoodCollObject>>& objects)
{
    for (const std::unique_ptr<HoodCollObject>& object : objects)
    {
        collWorld->addCollisionObject(object->btObject);
    }
}

void HoodSystem::RemCollObjects(const Vector<std::unique_ptr<HoodCollObject>>& objects)
{
    for (const std::unique_ptr<HoodCollObject>& object : objects)
    {
        collWorld->removeCollisionObject(object->btObject);
    }
}

void HoodSystem::ResetModifValues()
{
    modifOffset = Vector3(0, 0, 0);

    rotateHood->modifRotate = 0;
    scaleHood->modifScale = 0;
}

void HoodSystem::Process(float32 timeElapsed)
{
    InputLockGuard guard(GetScene(), this);
    if (guard.IsLockAcquired() == false)
    {
        return;
    }

    if (!IsLocked() && !lockedScale)
    {
        // scale hood depending on current camera position
        Camera* curCamera = cameraSystem->GetCurCamera();
        if (NULL != curCamera)
        {
            float32 camToHoodDist = (GetPosition() - curCamera->GetPosition()).Length();
            if (curCamera->GetIsOrtho())
            {
                SetScale(30.0f);
            }
            else
            {
                SetScale(camToHoodDist / 20.f);
            }
        }
    }
}

bool HoodSystem::Input(UIEvent* event)
{
    InputLockGuard guard(GetScene(), this);
    if (guard.IsLockAcquired() == false)
    {
        return false;
    }

    if (!event->point.IsZero())
    {
        // before checking result mark that there is no hood axis under mouse
        if (!lockedScale && !lockedAxis)
        {
            mouseOverAxis = ST_AXIS_NONE;

            // if is visible and not locked check mouse over status
            if (!lockedModif && NULL != curHood)
            {
                // get intersected items in the line from camera to current mouse position
                Vector3 traceFrom;
                Vector3 traceTo;

                cameraSystem->GetRayTo2dPoint(event->point, 99999.0f, traceFrom, traceTo);

                btVector3 btFrom(traceFrom.x, traceFrom.y, traceFrom.z);
                btVector3 btTo(traceTo.x, traceTo.y, traceTo.z);

                btCollisionWorld::AllHitsRayResultCallback btCallback(btFrom, btTo);
                collWorld->rayTest(btFrom, btTo, btCallback);

                if (btCallback.hasHit())
                {
                    for (const std::unique_ptr<HoodCollObject>& hObj : curHood->collObjects)
                    {
                        if (hObj->btObject == btCallback.m_collisionObjects[0])
                        {
                            // mark that mouse is over one of hood axis
                            mouseOverAxis = hObj->axis;
                            break;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void HoodSystem::Draw()
{
    InputLockGuard guard(GetScene(), this);
    if (guard.IsLockAcquired() == false)
    {
        return;
    }

    if ((curHood == nullptr) || !IsVisible())
        return;

    TextDrawSystem* textDrawSys = GetScene()->GetSystem<TextDrawSystem>();

    // modification isn't locked and whole system isn't locked
    if (!IsLocked() && !lockedModif)
    {
        ST_Axis showAsSelected = curAxis;
        if ((GetTransformType() != Selectable::TransformType::Disabled) && (ST_AXIS_NONE != mouseOverAxis))
        {
            showAsSelected = mouseOverAxis;
        }

        curHood->Draw(showAsSelected, mouseOverAxis, GetScene()->GetRenderSystem()->GetDebugDrawer(), textDrawSys);
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(AABBox3(GetPosition(), curHood->objScale * .04f), Color::White, RenderHelper::DRAW_SOLID_NO_DEPTH);
    }
    else
    {
        normalHood->Draw(curAxis, ST_AXIS_NONE, GetScene()->GetRenderSystem()->GetDebugDrawer(), textDrawSys);
    }
}

void HoodSystem::SetModifAxis(ST_Axis axis)
{
    if (ST_AXIS_NONE != axis)
    {
        curAxis = axis;
    }
}

ST_Axis HoodSystem::GetModifAxis() const
{
    return curAxis;
}

ST_Axis HoodSystem::GetPassingAxis() const
{
    return mouseOverAxis;
}

void HoodSystem::SetAxes(const DAVA::Vector3& x, const DAVA::Vector3& y, const DAVA::Vector3& z)
{
    if (axisX != x || axisY != y || axisZ != z)
    {
        axisX = x;
        axisY = y;
        axisZ = z;

        if (curHood != nullptr)
        {
            RemCollObjects(curHood->collObjects);
        }

        normalHood->SetAxes(x, y, z);
        moveHood->SetAxes(x, y, z);
        rotateHood->SetAxes(x, y, z);
        scaleHood->SetAxes(x, y, z);

        if (curHood != nullptr)
        {
            curHood->UpdatePos(curPos);
            curHood->UpdateScale(curScale);
            AddCollObjects(curHood->collObjects);
        }

        collWorld->updateAabbs();
    }
}

void HoodSystem::LockScale(bool lock)
{
    lockedScale = lock;
}

void HoodSystem::LockModif(bool lock)
{
    lockedModif = lock;
}

void HoodSystem::LockAxis(bool lock)
{
    lockedAxis = lock;
}

bool HoodSystem::AllowPerformSelectionHavingCurrent(const SelectableGroup& currentSelection)
{
    return !IsVisible() || (GetTransformType() == Selectable::TransformType::Disabled) || (ST_AXIS_NONE == GetPassingAxis());
}

bool HoodSystem::AllowChangeSelectionReplacingCurrent(const SelectableGroup& currentSelection, const SelectableGroup& newSelection)
{
    return true;
}
} // namespace DAVA
