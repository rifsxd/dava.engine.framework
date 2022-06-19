#include "REPlatform/Global/SceneTree/CreateEntitySupport.h"
#include "REPlatform/Commands/EntityAddCommand.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(BaseEntityCreator)
{
    ReflectionRegistrator<BaseEntityCreator>::Begin()
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(EntityCreator)
{
    ReflectionRegistrator<EntityCreator>::Begin()
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(SimpleEntityCreator)
{
    ReflectionRegistrator<SimpleEntityCreator>::Begin()
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(EntityCreatorsGroup)
{
    ReflectionRegistrator<EntityCreatorsGroup>::Begin()
    .End();
}

BaseEntityCreator::BaseEntityCreator(const QIcon& icon_, const QString& text_)
    : icon(icon_)
    , text(text_)
{
}

BaseEntityCreator::eMenuPointOrder BaseEntityCreator::GetOrder() const
{
    return eMenuPointOrder::UNKNOWN;
}

void BaseEntityCreator::Init(ContextAccessor* accessor_, UI* ui_)
{
    accessor = accessor_;
    ui = ui_;
}

EntityCreator::EntityCreator(const QIcon& icon, const QString& text)
    : BaseEntityCreator(icon, text)
{
}

void EntityCreator::StartEntityCreation(SceneEditor2* scene_)
{
    scene = scene_;
    StartEntityCreationImpl();
}

void EntityCreator::Cancel()
{
}

void EntityCreator::FinishCreation()
{
    scene = nullptr;
    creationFinished.Emit();
}

void EntityCreator::AddEntity(Entity* entity)
{
    DVASSERT(scene != nullptr);
    scene->Exec(std::make_unique<EntityAddCommand>(entity, scene));
}

EntityCreatorsGroup::EntityCreatorsGroup(const QIcon& icon, const QString& text)
    : BaseEntityCreator(icon, text)
{
}

SimpleEntityCreator::SimpleEntityCreator(BaseEntityCreator::eMenuPointOrder order_, const QIcon& icon, const QString& text, const Function<RefPtr<Entity>()>& fn)
    : EntityCreator(icon, text)
    , creationFn(fn)
    , order(order_)
{
}

BaseEntityCreator::eMenuPointOrder SimpleEntityCreator::GetOrder() const
{
    return order;
}

void SimpleEntityCreator::StartEntityCreationImpl()
{
    RefPtr<Entity> entity = creationFn();
    AddEntity(entity.Get());
    FinishCreation();
}
} // namespace DAVA
