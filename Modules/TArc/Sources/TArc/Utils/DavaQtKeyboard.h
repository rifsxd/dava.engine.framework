#pragma once

#include "Input/InputElements.h"

namespace DAVA
{
// we have to create this wrapper inside DAVA namespace for friend keyworkd works on private keyboard field
class DavaQtKeyboard
{
public:
    static void ClearAllKeys();
};
} // end namespace DAVA