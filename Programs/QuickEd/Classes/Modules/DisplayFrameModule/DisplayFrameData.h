#pragma once

#include <TArc/DataProcessing/TArcDataNode.h>

class DisplaySafeArea;

class DisplayFrameData : public DAVA::TArcDataNode
{
public:
    ~DisplayFrameData() override;

private:
    friend class DisplayFrameModule;
    std::unique_ptr<DisplaySafeArea> safeArea;

    DAVA_VIRTUAL_REFLECTION(DisplayFrameData, DAVA::TArcDataNode);
};
