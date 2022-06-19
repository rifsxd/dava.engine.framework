#pragma once

#include <Base/BaseTypes.h>
#include <UI/Components/UISingleComponent.h>

namespace DAVA
{
class UIControl;

/** Single component for notification Spine system about modified components. */
struct UISpineSingleComponent : public UISingleComponent
{
    /** Set of controls with modified Spine components. */
    UnorderedSet<UIControl*> spineModified;
    /** Set of controls with modified Spine components for reloading skeleton and atlas. */
    UnorderedSet<UIControl*> spineNeedReload;
    /** Set of controls with modified Spine components for reattach bones to controls. */
    UnorderedSet<UIControl*> spineBonesModified;

    void ResetState() override;
};
}
