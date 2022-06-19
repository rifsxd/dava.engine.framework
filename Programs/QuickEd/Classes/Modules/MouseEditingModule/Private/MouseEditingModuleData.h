#pragma once

#include <TArc/DataProcessing/TArcDataNode.h>

class MouseEditingSystem;

class MouseEditingModuleData : public DAVA::TArcDataNode
{
public:
    ~MouseEditingModuleData() override;
    std::unique_ptr<MouseEditingSystem> system;

    DAVA_VIRTUAL_REFLECTION(MouseEditingModuleData, DAVA::TArcDataNode);
};
