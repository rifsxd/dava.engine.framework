#include "REPlatform/Commands/MetaObjModifyCommand.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
MetaObjModifyCommand::MetaObjModifyCommand(const DAVA::MetaInfo* _info, void* _object, const DAVA::VariantType& _newValue)
    : RECommand("Modify value")
    , info(_info)
    , object(_object)
    , newValue(_newValue)
{
    if (NULL != info && NULL != object)
    {
        oldValue = DAVA::VariantType::LoadData(object, info);
    }
}

MetaObjModifyCommand::~MetaObjModifyCommand()
{
}

void MetaObjModifyCommand::Undo()
{
    if (NULL != info && NULL != object)
    {
        DAVA::VariantType::SaveData(object, info, oldValue);
    }
}

void MetaObjModifyCommand::Redo()
{
    if (NULL != info && NULL != object)
    {
        DAVA::VariantType::SaveData(object, info, newValue);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(MetaObjModifyCommand)
{
    ReflectionRegistrator<MetaObjModifyCommand>::Begin()
    .End();
}
} // namespace DAVA
