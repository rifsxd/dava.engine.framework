#include "TArc/Utils/Utils.h"
#include "TArc/Qt/QtIcon.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <DeviceManager/DeviceManager.h>
#include <Input/Keyboard.h>
#include <FileSystem/FileSystem.h>

#include <QPainter>
#include <QProcess>
#include <QDir>
#include <QApplication>

namespace DAVA
{
// Truncate the file extension.
QString TruncateFileExtension(const QString& fileName, const QString& extension)
{
    // Just wrap around the particular DAVA engine functions.

    String truncatedName = fileName.toStdString();

    size_t truncatedStringLen = truncatedName.length() - extension.length();
    bool endsWithExtension = false;
    if (fileName.length() >= extension.length())
    {
        endsWithExtension = (truncatedName.compare(truncatedStringLen, extension.length(), extension.toStdString()) == 0);
    }

    if (endsWithExtension)
    {
        truncatedName.resize(truncatedStringLen);
    }

    return QString::fromStdString(truncatedName);
}

bool FindAndReplace(String& str, const String& from, const String& to)
{
    size_t startPos = str.find(from);
    if (startPos == String::npos)
        return false;
    str.replace(startPos, from.length(), to);
    return true;
}

QPixmap CreateIconFromColor(const QColor& color)
{
    QPixmap pix(16, 16);
    QPainter p(&pix);

    if (color.alpha() < 255)
    {
        // QtDocumentation QPainter::drawRect : A filled rectangle has a size of rectangle.size()
        p.fillRect(QRect(0, 0, 16, 16), QColor(250, 250, 250));
        p.fillRect(QRect(0, 0, 8, 8), QColor(150, 150, 150));
        p.fillRect(QRect(8, 8, 16, 16), QColor(150, 150, 150));
    }

    p.fillRect(QRect(0, 0, 16, 16), color);
    return pix;
}

Color QColorToColor(const QColor& qtColor)
{
    return Color(qtColor.redF(), qtColor.greenF(), qtColor.blueF(), qtColor.alphaF());
}

QColor ColorToQColor(const Color& davaColor)
{
    float32 maxC = std::max({ 1.0f, davaColor.r, davaColor.g, davaColor.b });

    return QColor::fromRgbF(davaColor.r / maxC, davaColor.g / maxC, davaColor.b / maxC, Clamp(davaColor.a, 0.0f, 1.0f));
}

namespace StringPropertyDelegateDetails
{
//we need to store sequence in order
Vector<std::pair<QChar, QString>> escapeSequences = {
    { '\\', QStringLiteral("\\\\") },
    { '\n', QStringLiteral("\\n") },
    { '\r', QStringLiteral("\\r") },
    { '\t', QStringLiteral("\\t") },
};
}

//replace strings with escape characters
QString EscapeString(const QString& str)
{
    QString stringToReplace(str);
    for (const auto& pair : StringPropertyDelegateDetails::escapeSequences)
    {
        stringToReplace.replace(pair.second, pair.first);
    }
    return stringToReplace;
}

//replace escape characters with their string form
QString UnescapeString(const QString& str)
{
    QString stringToReplace(str);
    for (const auto& pair : StringPropertyDelegateDetails::escapeSequences)
    {
        stringToReplace.replace(pair.first, pair.second);
    }
    return stringToReplace;
}

Vector<float32> ParseFloatList(const String& str)
{
    return ParseFloatList(QString::fromStdString(str));
}

Vector<float32> ParseFloatList(const QString& str)
{
    QRegularExpression expr("[+-]?[\\d*]*[.]?[\\d*]+");
    QRegularExpressionMatchIterator iter = expr.globalMatch(str);
    Vector<float32> result;
    result.reserve(8);
    while (iter.hasNext())
    {
        QRegularExpressionMatch match(iter.next());
        if (match.hasMatch())
        {
            QString matchedStr = match.captured(0);
            bool ok;
            float32 value = matchedStr.toFloat(&ok);
            if (ok == true)
            {
                result.push_back(matchedStr.toFloat(&ok));
            }
        }
    }

    return result;
}

bool IsKeyPressed(eModifierKeys modifier)
{
    const Keyboard* keyboard = GetEngineContext()->deviceManager->GetKeyboard();
    if (keyboard == nullptr)
    {
        return false;
    }

    switch (modifier)
    {
    case eModifierKeys::ALT:
        return keyboard->GetKeyState(eInputElements::KB_LALT).IsPressed() || keyboard->GetKeyState(eInputElements::KB_RALT).IsPressed();
    case eModifierKeys::CONTROL:
#ifdef __DAVAENGINE_WINDOWS__
        return keyboard->GetKeyState(eInputElements::KB_LCTRL).IsPressed() || keyboard->GetKeyState(eInputElements::KB_RCTRL).IsPressed();
#elif defined __DAVAENGINE_MACOS__
        return keyboard->GetKeyState(eInputElements::KB_LCMD).IsPressed() || keyboard->GetKeyState(eInputElements::KB_RCMD).IsPressed();
#else
#error "non supported platform";
#endif //platform
    case eModifierKeys::SHIFT:
        return keyboard->GetKeyState(eInputElements::KB_LSHIFT).IsPressed() || keyboard->GetKeyState(eInputElements::KB_RSHIFT).IsPressed();
    default:
        DVASSERT(false, "unsupported key");
        return false;
    }
}

namespace UtilsDetail
{
UnorderedMap<String, QIcon> sharedMap;
struct CleanUpRegistrator
{
    CleanUpRegistrator()
    {
        /*qAddPostRoutine([]()
                        {
                            sharedMap.clear();
                        });*/
    }
} cleanUpRegistrator;
}

const QIcon& SharedIcon(const char* path)
{
    using namespace UtilsDetail;

    String stringPath(path);
    auto iconIter = sharedMap.find(stringPath);
    if (iconIter != sharedMap.end())
        return iconIter->second;

    return sharedMap.emplace(std::move(stringPath), QIcon(path)).first->second;
}
} // namespace DAVA
