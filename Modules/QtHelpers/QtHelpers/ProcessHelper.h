#pragma once

#include <QString>
#include <QStringList>

namespace ProcessHelper
{
bool IsProcessRuning(const QString& path);
void SetActiveProcess(const QString& path);

//open application executable file
bool RunProcess(const QString& path);

//open application bundle file
bool OpenApplication(const QString& path, const QStringList& args = QStringList());
};
