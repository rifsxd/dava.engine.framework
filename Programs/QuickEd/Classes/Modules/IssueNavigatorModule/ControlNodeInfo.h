#pragma once

#include <Base/BaseTypes.h>
#include <Base/FastName.h>

class PackageNode;
class ControlNode;

class ControlNodeInfo
{
public:
    static bool IsRootControl(const ControlNode* node);
    static DAVA::String GetPathToControl(const ControlNode* node);
};
