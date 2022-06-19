#pragma once

#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>

class ControlNode;

namespace LibraryDefaultControls
{
DAVA::Vector<DAVA::RefPtr<ControlNode>> CreateDefaultControls();
}