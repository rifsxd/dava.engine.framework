#include "Modules/LibraryModule/Private/LibraryHelpers.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/YamlPackageSerializer.h"

namespace LibraryHelpers
{
DAVA::String SerializeToYamlString(PackageNode* package, ControlNode* control, bool isPrototype)
{
    using namespace DAVA;

    DVASSERT(package != nullptr);
    DVASSERT(control != nullptr);

    Vector<ControlNode*> controls;
    Vector<StyleSheetNode*> styles;

    RefPtr<ControlNode> resultControl;
    if (isPrototype)
    {
        resultControl = RefPtr<ControlNode>(ControlNode::CreateFromPrototype(control));
    }
    else
    {
        resultControl = control;
    }

    controls.push_back(resultControl.Get());

    YamlPackageSerializer serializer;
    serializer.SerializePackageNodes(package, controls, styles);
    return serializer.WriteToString();
}
};