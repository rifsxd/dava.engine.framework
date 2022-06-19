#pragma once

#include <TArc/DataProcessing/TArcDataNode.h>

class EditorInput;

class InputModuleData : public DAVA::TArcDataNode
{
public:
    InputModuleData();
    ~InputModuleData() override;

private:
    friend class InputModule;
    std::unique_ptr<EditorInput> system;

    DAVA_VIRTUAL_REFLECTION(InputModuleData, DAVA::TArcDataNode);
};
