#pragma once

#include <Base/BaseTypes.h>
#include <Input/InputElements.h>

#include <Qt>

namespace DAVA
{
class Sprite;
class FilePath;
class Image;
class Texture;

String SizeInBytesToString(float32 size);

bool IsKeyModificatorPressed(eInputElements key);
bool IsKeyModificatorsPressed();

#ifdef __DAVAENGINE_WIN32__
const Qt::WindowFlags WINDOWFLAG_ON_TOP_OF_APPLICATION = Qt::Window;
#else
const Qt::WindowFlags WINDOWFLAG_ON_TOP_OF_APPLICATION = Qt::Tool;
#endif

String ReplaceInString(const String& sourceString, const String& what, const String& on);

// Method for debugging. Save image to file
void SaveSpriteToFile(Sprite* sprite, const FilePath& path);
void SaveTextureToFile(Texture* texture, const FilePath& path);
void SaveImageToFile(Image* image, const FilePath& path);

Texture* CreateSingleMipTexture(const FilePath& pngPathname);
const FilePath& DefaultCursorPath();
} // namespace DAVA