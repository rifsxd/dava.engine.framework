#pragma once

#include "Model/PackageHierarchy/ControlNode.h"

#include <TArc/DataProcessing/TArcDataNode.h>

class HUDSystem;

class HUDModuleData : public DAVA::TArcDataNode
{
public:
    ~HUDModuleData() override;

    static DAVA::FastName highlightedNodePropertyName;

    ControlNode* GetHighlight() const;

private:
    friend class HUDModule;
    std::unique_ptr<HUDSystem> hudSystem;
    ControlNode* highlightedNode = nullptr;

    DAVA_VIRTUAL_REFLECTION(HUDModuleData, DAVA::TArcDataNode);
};
