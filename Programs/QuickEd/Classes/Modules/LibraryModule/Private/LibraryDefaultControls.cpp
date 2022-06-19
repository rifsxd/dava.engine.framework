#include "Classes/Modules/LibraryModule/Private/LibraryDefaultControls.h"
#include "Classes/Model/ControlProperties/AbstractProperty.h"
#include "Classes/Model/PackageHierarchy/ControlNode.h"
#include "Classes/Model/ControlProperties/RootProperty.h"

#include <Base/ObjectFactory.h>
#include <Math/Vector.h>
#include <UI/UIControl.h>

namespace LibraryDefaultControls
{
using namespace DAVA;

namespace Details
{
Array<std::pair<String, bool>, 15> controlDescrs =
{ {
{ "UIControl", false },
{ "UIStaticText", false },
{ "UITextField", false },
{ "UISlider", true },
{ "UIList", false },
{ "UIListCell", false },
{ "UIScrollBar", true },
{ "UIScrollView", true },
{ "UISpinner", true },
{ "UISwitch", true },
{ "UIParticles", false },
{ "UIWebView", false },
{ "UIMovieView", false },
{ "UI3DView", false },
{ "UIJoypad", true }
} };
} // namespace Details

Vector<RefPtr<ControlNode>> CreateDefaultControls()
{
    using namespace Details;

    DAVA::Vector<DAVA::RefPtr<ControlNode>> defaultControls;
    defaultControls.reserve(controlDescrs.size());

    for (const std::pair<String, bool>& descr : controlDescrs)
    {
        ScopedPtr<UIControl> control(ObjectFactory::Instance()->New<UIControl>(descr.first));
        DVASSERT(control);

        control->SetName(descr.first);
        if (descr.second)
        {
            defaultControls.emplace_back(ControlNode::CreateFromControlWithChildren(control));
        }
        else
        {
            defaultControls.emplace_back(ControlNode::CreateFromControl(control));
        }

        AbstractProperty* prop = defaultControls.back()->GetRootProperty()->FindPropertyByName("size");
        prop->SetValue(Vector2(32.0f, 32.0f));
    }

    return defaultControls;
}
}