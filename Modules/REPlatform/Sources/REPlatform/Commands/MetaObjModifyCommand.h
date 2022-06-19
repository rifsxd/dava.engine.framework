#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <FileSystem/VariantType.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
struct MetaInfo;
class MetaObjModifyCommand : public RECommand
{
public:
    MetaObjModifyCommand(const DAVA::MetaInfo* info, void* object, const DAVA::VariantType& value);
    ~MetaObjModifyCommand();

    void Undo() override;
    void Redo() override;

    const DAVA::MetaInfo* info;
    void* object;

    DAVA::VariantType oldValue;
    DAVA::VariantType newValue;

private:
    DAVA_VIRTUAL_REFLECTION(MetaObjModifyCommand, RECommand);
};
} // namespace DAVA
