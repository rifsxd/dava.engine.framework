#pragma once

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyDefaultEditor.h"

#include <QPointer>

class QLineEdit;
class QToolButton;
class QHBoxLayout;
class QAction;
class QMenu;

class ModernPropertyPathEditor : public ModernPropertyDefaultEditor
{
    Q_OBJECT

public:
    ModernPropertyPathEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property,
                             const QList<QString>& extensions, const QString& resourceSubDir, bool allowAnyExtension);
    ~ModernPropertyPathEditor() override;

    void AddToGrid(QGridLayout* layout, int row, int col, int colSpan) override;

protected:
    void OnPropertyChanged() override;
    void ResetProperty() override;

private:
    void OnChooseAction();
    void OnOpenFileAction();
    void OnOpenFolderAction();
    void OnEditingFinished();
    void OnTextChanged(const QString& text);
    bool IsPathValid(const QString& path) const;
    void ClearError();

    QPointer<QLineEdit> line;
    QPointer<QToolButton> button;
    QPointer<QMenu> menu;
    QPointer<QAction> chooseAction;
    QPointer<QAction> openFileAction;
    QPointer<QAction> openFolderAction;

    QList<QString> resourceExtensions;
    QString resourceSubDir;
    bool allowAnyExtension = false;

    QPointer<QHBoxLayout> layout;
};
