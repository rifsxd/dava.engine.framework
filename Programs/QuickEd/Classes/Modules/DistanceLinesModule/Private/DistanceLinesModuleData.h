#pragma once

#include <TArc/DataProcessing/TArcDataNode.h>

class DistanceSystem;

class DistanceLinesModuleData : public DAVA::TArcDataNode
{
public:
    ~DistanceLinesModuleData() override;

private:
    friend class DistanceLinesModule;
    std::unique_ptr<DistanceSystem> system;

    DAVA_VIRTUAL_REFLECTION(DistanceLinesModuleData, DAVA::TArcDataNode);
};
