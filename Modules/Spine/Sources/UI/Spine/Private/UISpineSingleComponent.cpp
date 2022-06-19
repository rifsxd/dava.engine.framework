#include "UI/Spine/UISpineSingleComponent.h"

#include <UI/UIControl.h>

namespace DAVA
{
void UISpineSingleComponent::ResetState()
{
    spineModified.clear();
    spineNeedReload.clear();
    spineBonesModified.clear();
}
}
