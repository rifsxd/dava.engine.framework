#pragma once

#include <TArc/DataProcessing/TArcDataNode.h>

class PixelGrid;

class PixelGridData : public DAVA::TArcDataNode
{
public:
    ~PixelGridData() override;

private:
    friend class PixelGridModule;
    std::unique_ptr<PixelGrid> pixelGrid;

    DAVA_VIRTUAL_REFLECTION(PixelGridData, DAVA::TArcDataNode);
};
