#pragma once

#include <TArc/DataProcessing/TArcDataNode.h>

class CreatingControlsSystem;

class CreatingControlsData : public DAVA::TArcDataNode
{
public:
    CreatingControlsData();
    ~CreatingControlsData();

private:
    friend class CreatingControlsModule;
    std::unique_ptr<CreatingControlsSystem> creatingControlsSystem;

    DAVA_VIRTUAL_REFLECTION(CreatingControlsData, DAVA::TArcDataNode);
};