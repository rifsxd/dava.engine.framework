#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Math/Rect.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class CustomColorsProxy;
class Image;

class ModifyCustomColorsCommand : public RECommand
{
public:
    ModifyCustomColorsCommand(Image* originalImage, Image* currentImage, CustomColorsProxy* customColorsProxy,
                              const Rect& updatedRect, bool clearTexture);
    ~ModifyCustomColorsCommand() override;

    void Undo() override;
    void Redo() override;

private:
    void ApplyImage(Image* image, bool disableBlend);

private:
    CustomColorsProxy* customColorsProxy = nullptr;
    Image* undoImage = nullptr;
    Image* redoImage = nullptr;
    Rect updatedRect;
    bool shouldClearTexture = false;

    DAVA_VIRTUAL_REFLECTION(ModifyCustomColorsCommand, RECommand);
};
} // namespace DAVA
