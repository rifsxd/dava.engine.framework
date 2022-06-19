#pragma once

#include "REPlatform/Scene/SceneEditor2.h"

#include <TArc/Qt/QtIcon.h>
#include <TArc/Qt/QtString.h>

#include <Base/RefPtr.h>
#include <Functional/Signal.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
class ContextAccessor;
class UI;
class SceneEditor2;

class BaseEntityCreator : public ReflectionBase
{
public:
    enum class eMenuPointOrder
    {
        UNKNOWN = -1,
        EMPTY_ENTITY,
        LIGHT_ENTITY,
        CAMERA_ENTITY,
        USER_NODE_ENITY,
        SWITCH_ENTITY,
        PARTICLE_EFFECT_ENTITY,
        LANDSCAPE_ENTITY,
        WIND_ENTITY,
        VEGETATION_ENTITY,
        PATH_ENTITY,
        TEXT_ENTITY,
        PHYSICS_ENTITIES,
        EDITOR_SPRITE
    };

    BaseEntityCreator(const QIcon& icon, const QString& text);
    virtual ~BaseEntityCreator() = default;

    virtual eMenuPointOrder GetOrder() const;
    void Init(ContextAccessor* accessor, UI* ui);

    QIcon icon;
    QString text;

protected:
    ContextAccessor* accessor = nullptr;
    UI* ui = nullptr;

    DAVA_VIRTUAL_REFLECTION(BaseEntityCreator);
};

class EntityCreator : public BaseEntityCreator
{
public:
    EntityCreator(const QIcon& icon, const QString& text);
    void StartEntityCreation(SceneEditor2* scene);

    virtual void Cancel();

    Signal<> creationFinished;

protected:
    virtual void StartEntityCreationImpl() = 0;
    void FinishCreation();
    void AddEntity(Entity* entity);

protected:
    SceneEditor2* scene = nullptr;

    DAVA_VIRTUAL_REFLECTION(EntityCreator, BaseEntityCreator);
};

class SimpleEntityCreator : public EntityCreator
{
public:
    SimpleEntityCreator(BaseEntityCreator::eMenuPointOrder order, const QIcon& icon, const QString& text,
                        const Function<RefPtr<Entity>()>& fn);

    BaseEntityCreator::eMenuPointOrder GetOrder() const override;

protected:
    void StartEntityCreationImpl() override;

private:
    Function<RefPtr<Entity>()> creationFn;
    BaseEntityCreator::eMenuPointOrder order;

    DAVA_VIRTUAL_REFLECTION(SimpleEntityCreator, EntityCreator);
};

class EntityCreatorsGroup : public BaseEntityCreator
{
public:
    EntityCreatorsGroup(const QIcon& icon, const QString& text);
    Vector<std::unique_ptr<BaseEntityCreator>> creatorsGroup;

    DAVA_VIRTUAL_REFLECTION(EntityCreatorsGroup, BaseEntityCreator);
};
} // namespace DAVA
