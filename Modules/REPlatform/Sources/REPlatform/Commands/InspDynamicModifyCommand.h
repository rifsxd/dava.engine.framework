#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Base/FastName.h>
#include <Base/Introspection.h>
#include <FileSystem/VariantType.h>

namespace DAVA
{
class InspDynamicModifyCommand : public RECommand
{
public:
    InspDynamicModifyCommand(InspInfoDynamic* dynamicInfo, const InspInfoDynamic::DynamicData& ddata, FastName key, const VariantType& value);

    void Undo() override;
    void Redo() override;

    InspInfoDynamic* dynamicInfo;
    FastName key;

    InspInfoDynamic::DynamicData ddata;

    VariantType oldValue;
    VariantType newValue;

private:
    DAVA_VIRTUAL_REFLECTION(InspDynamicModifyCommand, RECommand);
};
} // namespace DAVA
