#pragma once

#include <TArc/Core/ContextAccessor.h>

#include <Base/BaseTypes.h>

#include <QDialog>
#include <QMap>
#include <memory>

namespace Ui
{
class FindFileDialog;
}

class FileSystemCache;
class QCompleter;
class QAction;

class FindFileDialog final : public QDialog
{
public:
    static QString GetFilePath(DAVA::ContextAccessor* accessor, const FileSystemCache* fileSystemCache, const QString& extension, QWidget* parent);

private:
    explicit FindFileDialog(const FileSystemCache* projectStructure, const QString& extension, const DAVA::String& lastUsedPath, QWidget* parent = nullptr);

    void Init(const QStringList& files);

    bool eventFilter(QObject* obj, QEvent* event) override;

    QString GetCommonParent(const QString& path1, const QString& path2);

    QString ToShortName(const QString& name) const;
    QString FromShortName(const QString& name) const;

    std::unique_ptr<Ui::FindFileDialog> ui;

    QString prefix;
    QCompleter* completer = nullptr;
    DAVA::String lastUsedPath;
    QStringList stringsToDisplay;
};
