#include "Modules/ModernPropertiesModule/Editors/ModernPropertyPathEditor.h"

#include "Application/QEGlobal.h"
#include "Model/ControlProperties/ValueProperty.h"
#include "Modules/ProjectModule/ProjectData.h"
#include "Utils/QtDavaConvertion.h"

#include <Base/Any.h>

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Core/OperationInvoker.h>
#include <TArc/DataProcessing/PropertiesHolder.h>
#include <TArc/Utils/Utils.h>

#include <QtTools/FileDialogs/FileDialog.h>

#include <Engine/Engine.h>
#include <FileSystem/FileSystem.h>

#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLineEdit>
#include <QMap>
#include <QMenu>
#include <QToolButton>

ModernPropertyPathEditor::ModernPropertyPathEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property,
                                                   const QList<QString>& extensions_, const QString& resourceSubDir_, bool allowAnyExtension_)
    : ModernPropertyDefaultEditor(context, property)
    , resourceExtensions(extensions_)
    , resourceSubDir(resourceSubDir_)
    , allowAnyExtension(allowAnyExtension_)
{
    using namespace DAVA;

    line = new QLineEdit(GetParentWidget());
    line->setProperty("property", true);
    QObject::connect(line, &QLineEdit::editingFinished, this, &ModernPropertyPathEditor::OnEditingFinished);
    QObject::connect(line, &QLineEdit::textChanged, this, &ModernPropertyPathEditor::OnTextChanged);

    chooseAction = new QAction("Choose file", GetParentWidget());
    QObject::connect(chooseAction, &QAction::triggered, this, &ModernPropertyPathEditor::OnChooseAction);
    openFileAction = new QAction("Open file", GetParentWidget());
    QObject::connect(openFileAction, &QAction::triggered, this, &ModernPropertyPathEditor::OnOpenFileAction);
    openFolderAction = new QAction("Open folder", GetParentWidget());
    QObject::connect(openFolderAction, &QAction::triggered, this, &ModernPropertyPathEditor::OnOpenFolderAction);

    menu = new QMenu();
    menu->addAction(chooseAction);
    menu->addAction(openFileAction);
    menu->addAction(openFolderAction);

    button = new QToolButton(GetParentWidget());
    button->setProperty("property", true);
    button->setPopupMode(QToolButton::ToolButtonPopupMode::MenuButtonPopup);
    button->setMenu(menu);
    button->setDefaultAction(chooseAction);
    button->setText("...");

    layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(4);
    layout->addWidget(line);
    layout->addWidget(button);

    OnPropertyChanged();
}

ModernPropertyPathEditor::~ModernPropertyPathEditor() = default;

void ModernPropertyPathEditor::AddToGrid(QGridLayout* grid, int row, int col, int colSpan)
{
    grid->addWidget(propertyName, row, col);
    grid->addLayout(layout, row, col + 1, 1, colSpan);
}

void ModernPropertyPathEditor::OnPropertyChanged()
{
    ModernPropertyDefaultEditor::OnPropertyChanged();

    line->setDisabled(property->IsReadOnly());
    button->setDisabled(property->IsReadOnly());

    QSignalBlocker blockLineSignals(line);
    DAVA::String path = property->GetValue().Cast<DAVA::FilePath>().GetStringValue();
    line->setText(QString::fromStdString(path));
    ApplyStyleToWidget(line);
}

void ModernPropertyPathEditor::ResetProperty()
{
    ClearError();
    ModernPropertyDefaultEditor::ResetProperty();
}

void ModernPropertyPathEditor::OnOpenFileAction()
{
    using namespace DAVA;

    QString pathText = line->text();

    DAVA::FilePath filePath = QStringToString(pathText);

    QString path = QString::fromStdString(filePath.GetAbsolutePathname());

    if (path.endsWith(".yaml", Qt::CaseInsensitive))
    {
        GetInvoker()->Invoke(QEGlobal::OpenDocumentByPath.ID, path);
    }
    else
    {
        QDesktopServices::openUrl(QUrl("file:///" + path, QUrl::TolerantMode));
    }
}

void ModernPropertyPathEditor::OnOpenFolderAction()
{
    using namespace DAVA;

    QString pathText = line->text();

    DAVA::FilePath filePath = QStringToString(pathText);

    QString path = QString::fromStdString(filePath.GetDirectory().GetAbsolutePathname());

    QDesktopServices::openUrl(QUrl("file:///" + path, QUrl::TolerantMode));
}

void ModernPropertyPathEditor::OnChooseAction()
{
    using namespace DAVA;

    const String propertiesItemKey = "ResourceFilePropertyDelegate-" + resourceExtensions.join("-").toStdString();
    const String propertiesDirKey = "Dir";

    DataContext* globalContext = GetAccessor()->GetGlobalContext();
    ProjectData* projectData = globalContext->GetData<ProjectData>();

    QString dir;
    QString pathText = line->text();
    if (!pathText.isEmpty())
    {
        FilePath filePath = QStringToString(pathText);
        dir = StringToQString(filePath.GetDirectory().GetAbsolutePathname());
    }
    else
    {
        PropertiesItem properties = projectData->CreatePropertiesNode(propertiesItemKey);
        String dirStr = properties.Get(propertiesDirKey, String(""));
        if (dirStr.empty())
        {
            QString projectResourceDir = QString::fromStdString(projectData->GetResourceDirectory().absolute.GetStringValue());
            dir = projectResourceDir + resourceSubDir;
            QFileInfo fi(dir);
            if (fi.exists() == false)
            {
                dir = projectResourceDir;
                fi = QFileInfo(dir);
                DVASSERT(fi.exists(), (dir.toStdString() + " not exists!").c_str());
            }
        }
        else
        {
            dir = QString::fromStdString(dirStr);
        }
    }

    QString filters;
    for (QString& filter : resourceExtensions)
    {
        filters += "*" + filter + " ";
    }
    QString filePathText = FileDialog::getOpenFileName(GetParentWidget(), tr("Select resource file"), dir, filters);

    filePathText = projectData->RestoreResourcesSymLinkInFilePath(filePathText);

    if (!filePathText.isEmpty())
    {
        PropertiesItem properties = projectData->CreatePropertiesNode(propertiesItemKey);
        properties.Set(propertiesDirKey, filePathText.toStdString());

        FilePath absoluteFilePath = QStringToString(filePathText);
        FilePath frameworkFilePath = absoluteFilePath.GetFrameworkPath();

        ChangeProperty(frameworkFilePath.GetStringValue());

        QSignalBlocker blockLineSignals(line);
        line->setText(StringToQString(frameworkFilePath.GetStringValue()));
    }
}

void ModernPropertyPathEditor::OnEditingFinished()
{
    if (line->isModified())
    {
        line->setProperty("error", false);
        DAVA::FilePath filePath = DAVA::FilePath(line->text().toStdString());
        if (filePath.Exists() || line->text().isEmpty())
        {
            ChangeProperty(filePath.GetStringValue());
        }
        else
        {
            OnPropertyChanged();
        }
    }
}

void ModernPropertyPathEditor::OnTextChanged(const QString& text)
{
    line->setProperty("error", !IsPathValid(text));
    ApplyStyleToWidget(line);
}

bool ModernPropertyPathEditor::IsPathValid(const QString& path) const
{
    DAVA::FilePath filePath(QStringToString(path));
    DAVA::FileSystem* fileSystem = DAVA::Engine::Instance()->GetContext()->fileSystem;
    return fileSystem->Exists(filePath);
}

void ModernPropertyPathEditor::ClearError()
{
    line->setText("");
    line->setProperty("error", false);
}
