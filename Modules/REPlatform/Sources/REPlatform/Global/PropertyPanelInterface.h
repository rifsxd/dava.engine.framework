#pragma once

#include <TArc/Controls/PropertyPanel/PropertyModelExtensions.h>

namespace DAVA
{
class PropertyPanelInterface
{
public:
    virtual void RegisterExtension(const std::shared_ptr<DAVA::ExtensionChain>& extension) = 0;
    virtual void UnregisterExtension(const std::shared_ptr<DAVA::ExtensionChain>& extension) = 0;
};
} // namespace DAVA
