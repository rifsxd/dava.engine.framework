#include "TArc/Utils/DavaQtKeyboard.h"

#include "Engine/Engine.h"
#include "DeviceManager/DeviceManager.h"
#include "Input/Keyboard.h"

namespace DAVA
{
void DavaQtKeyboard::ClearAllKeys()
{
    Keyboard* keyboard = GetEngineContext()->deviceManager->GetKeyboard();
    if (keyboard != nullptr)
    {
        keyboard->ResetState(GetPrimaryWindow());
    }
}
}