#pragma once

#include <Base/BaseTypes.h>

class PackageNode;
class ControlNode;

namespace LibraryHelpers
{
DAVA::String SerializeToYamlString(PackageNode* package, ControlNode* control, bool isPrototype = false);
};