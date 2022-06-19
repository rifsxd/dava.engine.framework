#pragma once

#include "TArc/Qt/QtString.h"

#include <Engine/EngineTypes.h>
#include <FileSystem/FilePath.h>
#include <Debug/DVAssert.h>
#include <Math/Color.h>
#include <Base/BaseTypes.h>

#include <QPixmap>
#include <QColor>
#include <QRegularExpression>

namespace DAVA
{
// Different string utilities.
// Truncate the file extension.
QString TruncateFileExtension(const QString& fileName, const QString& extension);
bool FindAndReplace(String& str, const String& from, const String& to);

QPixmap CreateIconFromColor(const QColor& color);

Color QColorToColor(const QColor& qtColor);

QColor ColorToQColor(const Color& davaColor);

QString EscapeString(const QString& str);
QString UnescapeString(const QString& str);

Vector<float32> ParseFloatList(const String& str);
Vector<float32> ParseFloatList(const QString& str);

template <typename T>
T StringToVector(const QString& str)
{
    static_assert(std::is_same<T, Vector2>::value ||
                  std::is_same<T, Vector3>::value ||
                  std::is_same<T, Vector4>::value,
                  "this function works only for types Vector2, Vector3 and Vector4"
                  );

    Vector<float32> result = ParseFloatList(str);
    T vector;
    int32 count = Min(static_cast<int32>(result.size()), static_cast<int32>(T::AXIS_COUNT));
    for (int32 i = 0; i < count; i++)
    {
        vector.data[i] = result[i];
    }
    return vector;
}

bool IsKeyPressed(eModifierKeys modifier);

const QIcon& SharedIcon(const char*);
} // namespace DAVA
