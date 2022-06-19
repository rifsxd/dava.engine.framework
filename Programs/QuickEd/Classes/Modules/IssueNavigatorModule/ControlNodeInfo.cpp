#include "Classes/Modules/IssueNavigatorModule/ControlNodeInfo.h"

#include "Classes/Model/ControlProperties/NameProperty.h"
#include "Classes/Model/ControlProperties/RootProperty.h"
#include "Classes/Model/PackageHierarchy/PackageNode.h"
#include "Classes/Model/PackageHierarchy/PackageControlsNode.h"
#include "Classes/Model/PackageHierarchy/ControlNode.h"

bool ControlNodeInfo::IsRootControl(const ControlNode* node)
{
    return ((node->GetParent() == node->GetPackage()->GetPackageControlsNode()) || (node->GetParent() == node->GetPackage()->GetPrototypes()));
}

DAVA::String ControlNodeInfo::GetPathToControl(const ControlNode* node)
{
    auto getParentNode = [](const ControlNode* node) -> ControlNode*
    {
        return IsRootControl(node) ? nullptr : dynamic_cast<ControlNode*>(node->GetParent());
    };

    DAVA::String pathToControl = node->GetName();
    for (const ControlNode* nextNode = getParentNode(node);
         nextNode != nullptr;
         nextNode = getParentNode(nextNode))
    {
        pathToControl = nextNode->GetName() + "/" + pathToControl;
    }

    return pathToControl;
}
