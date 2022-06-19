#include "TArc/Utils/QtMessageHandler.h"
#include "TArc/Qt/QtString.h"

#include <Logger/Logger.h>
#include <Debug/DVAssert.h>

namespace DAVA
{
void DAVAMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type)
    {
    case QtDebugMsg:
        Logger::Debug("Qt debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        Logger::Warning("Qt Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
    {
        Vector<QString> ignoredStrings =
        {
          "QFileSystemWatcher: FindNextChangeNotification failed"
        };

        auto it = std::find_if(ignoredStrings.begin(), ignoredStrings.end(), [&msg](const QString ignored)
                               {
                                   return msg.contains(ignored);
                               });

        if (it == ignoredStrings.end())
        { //we should log only not ignored messages
            Logger::Error("Qt Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        }

        break;
    }
    case QtFatalMsg:
        Logger::Error("Qt Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        DVASSERT(false);
        break;
    default:
        Logger::Info("Qt Unknown: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    }
}
} // namespace DAVA
