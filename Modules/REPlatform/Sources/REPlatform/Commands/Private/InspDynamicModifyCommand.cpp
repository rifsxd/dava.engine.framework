#include "REPLatform/Commands/InspDynamicModifyCommand.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
InspDynamicModifyCommand::InspDynamicModifyCommand(InspInfoDynamic* _dynamicInfo, const InspInfoDynamic::DynamicData& _ddata, FastName _key, const VariantType& _newValue)
    : RECommand("Modify dynamic value")
    , dynamicInfo(_dynamicInfo)
    , key(_key)
    , ddata(_ddata)
    , newValue(_newValue)
{
    if (nullptr != dynamicInfo)
    {
        // if value can't be edited, it means that it was inherited
        // so don't retrieve oldValue, but leave it as uninitialized variant
        if (dynamicInfo->MemberFlags(ddata, key) & I_EDIT)
        {
            oldValue = dynamicInfo->MemberValueGet(ddata, key);
        }
    }
}

void InspDynamicModifyCommand::Undo()
{
    if (nullptr != dynamicInfo)
    {
        dynamicInfo->MemberValueSet(ddata, key, oldValue);
    }
}

void InspDynamicModifyCommand::Redo()
{
    if (nullptr != dynamicInfo)
    {
        dynamicInfo->MemberValueSet(ddata, key, newValue);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(InspDynamicModifyCommand)
{
    ReflectionRegistrator<InspDynamicModifyCommand>::Begin()
    .End();
}
} // namespace DAVA
