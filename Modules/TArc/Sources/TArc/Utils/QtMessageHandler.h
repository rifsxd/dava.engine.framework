#pragma once

#include <QtGlobal>

class QString;
namespace DAVA
{
void DAVAMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
} // namespace DAVA
