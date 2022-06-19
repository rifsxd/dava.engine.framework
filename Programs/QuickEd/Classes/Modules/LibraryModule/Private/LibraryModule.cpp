#include "Classes/Modules/LibraryModule/LibraryModule.h"
#include "Classes/Modules/LibraryModule/Private/LibraryWidget.h"
#include "Classes/Modules/LibraryModule/Private/LibraryDefaultControls.h"
#include "Classes/Modules/LibraryModule/Private/LibraryHelpers.h"

#include "Classes/Modules/DocumentsModule/DocumentData.h"
#include "Classes/Modules/ProjectModule/ProjectData.h"

#include "Classes/Application/QEGlobal.h"

#include "Classes/Model/QuickEdPackageBuilder.h"
#include "Classes/Model/PackageHierarchy/PackageControlsNode.h"
#include "Classes/Model/PackageHierarchy/ImportedPackagesNode.h"

#include "Classes/UI/IconHelper.h"

#include <TArc/Core/FieldBinder.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/Utils/ModuleCollection.h>

#include <Engine/Engine.h>
#include <UI/UIPackageLoader.h>
#include <UI/UIControl.h>

#include <QToolButton>
#include <QTreeView>
#include <QMenu>

namespace LibraryModuleDetails
{
QString CreateMenuName(QString name)
{
#if defined __DAVAENGINE_MACOS__
    // toolbar buttons with menu do look differently on Mac and Win. On Mac, button text overlaps with dropdown arrow symbol.
    // Adding spaces should fix this overlap issue.
    return name + QStringLiteral("  ");
#else
    return name;
#endif
}

const QString controlsToolbarName = "Library Controls Toolbar";
const QString menuNameLibrary = CreateMenuName("Library");
const QString menuNamePrototypes = CreateMenuName("Prototypes");
};

DAVA_VIRTUAL_REFLECTION_IMPL(LibraryModule)
{
    DAVA::ReflectionRegistrator<LibraryModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void LibraryModule::PostInit()
{
    InitData();
    InitUI();
    BindFields();
    CreateActions();
}

void LibraryModule::InitData()
{
    std::unique_ptr<LibraryData> data = std::make_unique<LibraryData>();
    data->defaultControls = LibraryDefaultControls::CreateDefaultControls();
    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));
}

void LibraryModule::InitUI()
{
    using namespace DAVA;
    using namespace LibraryModuleDetails;

    { // create Library dock panel
        QString dockPanelTitle = "Library";
        DockPanelInfo dockPanelInfo;
        dockPanelInfo.title = dockPanelTitle;
        dockPanelInfo.area = Qt::LeftDockWidgetArea;
        PanelKey dockPanelKey(dockPanelTitle, dockPanelInfo);

        GetLibraryData()->libraryWidget = new LibraryWidget(GetAccessor(), GetUI());
        GetUI()->AddView(DAVA::mainWindowKey, dockPanelKey, GetLibraryData()->libraryWidget);
    }

    { // create toolbar
        ActionPlacementInfo toolbarTogglePlacement(CreateMenuPoint(QList<QString>() << "View"
                                                                                    << "Toolbars"));
        GetUI()->DeclareToolbar(DAVA::mainWindowKey, toolbarTogglePlacement, LibraryModuleDetails::controlsToolbarName);
    }
}

void LibraryModule::BindFields()
{
    using namespace DAVA;

    fieldBinder.reset(new FieldBinder(GetAccessor()));

    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<ProjectData>();
        fieldDescr.fieldName = FastName(ProjectData::projectPathPropertyName);
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &LibraryModule::OnProjectPathChanged));
    }

    packageListenerProxy.Init(this, GetAccessor());
}

void LibraryModule::CreateActions()
{
    using namespace DAVA;

    LibraryData* data = GetLibraryData();
    LibraryWidget* libraryWidget = data->libraryWidget;

    {
        QtAction* action = new QtAction(GetAccessor(), QObject::tr("Collapse all"), libraryWidget);

        KeyBindableActionInfo info;
        info.blockName = "Library";
        info.context = Qt::WidgetWithChildrenShortcut;
        MakeActionKeyBindable(action, info);

        QTreeView* libraryWidgetTreeView = libraryWidget->GetTreeView();
        QObject::connect(action, &QAction::triggered, libraryWidgetTreeView, &QTreeView::collapseAll);

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<ProjectData>();
        fieldDescr.fieldName = FastName(ProjectData::projectPathPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any {
            return fieldValue.Cast<FilePath>(FilePath()).IsEmpty() == false;
        });
        libraryWidgetTreeView->addAction(action);
        GetUI()->AddAction(DAVA::mainWindowKey, ActionPlacementInfo(CreateInvisiblePoint()), action);
    }
}

void LibraryModule::ActivePackageNodeWasChanged(PackageNode* activePackageNode)
{
    RemovePackagePrototypes();

    LibraryData* libraryData = GetLibraryData();
    libraryData->currentPackageNode = activePackageNode;

    if (activePackageNode != nullptr)
    {
        AddPackagePrototypes(activePackageNode);
    }

    libraryData->libraryWidget->SetCurrentPackage(activePackageNode);
}

void LibraryModule::OnProjectPathChanged(const DAVA::Any& projectPath)
{
    using namespace DAVA;

    RemoveProjectControls();

    Vector<RefPtr<PackageNode>> libraryPackages;

    if (projectPath.Cast<FilePath>(FilePath()).IsEmpty() == false)
    {
        const DataContext* globalContext = GetAccessor()->GetGlobalContext();
        ProjectData* projectData = globalContext->GetData<ProjectData>();
        DVASSERT(projectData != nullptr);
        libraryPackages = LoadLibraryPackages(projectData);

        AddProjectControls(projectData, libraryPackages);
    }

    GetLibraryData()->libraryWidget->SetLibraryPackages(libraryPackages);
}

void LibraryModule::OnControlCreateTriggered(ControlNode* node, bool makePrototype)
{
    DocumentData* documentData = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
    DVASSERT(documentData != nullptr);
    PackageNode* currentPackage = documentData->GetPackageNode();
    DVASSERT(currentPackage != nullptr);

    DAVA::String yamlString = LibraryHelpers::SerializeToYamlString(currentPackage, node, makePrototype);

    InvokeOperation(QEGlobal::CreateByClick.ID, yamlString);
}

DAVA::Vector<DAVA::RefPtr<PackageNode>> LibraryModule::LoadLibraryPackages(ProjectData* projectData)
{
    using namespace DAVA;

    const EngineContext* engineContext = GetEngineContext();

    DAVA::Vector<DAVA::RefPtr<PackageNode>> libraryPackages;

    for (const ProjectData::LibrarySection& section : projectData->GetLibrarySections())
    {
        QuickEdPackageBuilder builder(engineContext);
        PackageNode* package = nullptr;
        if (UIPackageLoader(projectData->GetPrototypes()).LoadPackage(section.packagePath.absolute, &builder))
        {
            RefPtr<PackageNode> libraryPackage = builder.BuildPackage();
            if (builder.GetResults().HasErrors())
            {
                NotificationParams params;
                params.title = "Can't load library package";
                params.message.type = Result::RESULT_ERROR;
                params.message.message = Format("Package '%s' has problems...", section.packagePath.absolute.GetFilename().c_str());
                GetUI()->ShowNotification(DAVA::mainWindowKey, params);
            }
            else
            {
                libraryPackages.emplace_back(libraryPackage);
            }
        }
    }

    return libraryPackages;
}

LibraryData* LibraryModule::GetLibraryData()
{
    return GetAccessor()->GetGlobalContext()->GetData<LibraryData>();
}

QString LibraryModule::GenerateUniqueName()
{
    return QString("object%1").arg(uniqueNumber++);
}

void LibraryModule::AddProjectControls(const ProjectData* projectData, const DAVA::Vector<DAVA::RefPtr<PackageNode>>& libraryPackages)
{
    using namespace DAVA;
    using namespace LibraryModuleDetails;

    { // create menu "Controls", insert before "Help"
        QMenu* controlsMenu = new QMenu(QStringLiteral("Controls"), nullptr);
        ActionPlacementInfo controlsMenuPlacement(CreateMenuPoint("", { DAVA::InsertionParams::eInsertionMethod::BeforeItem, MenuItems::menuHelp }));
        GetUI()->AddAction(mainWindowKey, controlsMenuPlacement, controlsMenu->menuAction());
    }

    { // create menu "Library", insert into Controls menu and Controls toolbar
        QMenu* libraryMenu = new QMenu(menuNameLibrary, nullptr);
        ActionPlacementInfo placement;
        placement.AddPlacementPoint(CreateMenuPoint("Controls"));
        GetUI()->AddAction(mainWindowKey, placement, libraryMenu->menuAction());
    }

    AddProjectPinnedControls(projectData, libraryPackages);
    AddProjectLibraryControls(projectData, libraryPackages);
    AddDefaultControls();
}

void LibraryModule::AddProjectPinnedControls(const ProjectData* projectData, const DAVA::Vector<DAVA::RefPtr<PackageNode>>& libraryPackages)
{
    using namespace DAVA;

    using namespace LibraryModuleDetails;

    FieldDescriptor packageField;
    packageField.type = ReflectedTypeDB::Get<DocumentData>();
    packageField.fieldName = FastName(DocumentData::packagePropertyName);

    LibraryData* libraryData = GetLibraryData();

    const Vector<ProjectData::PinnedControl>& pinnedControls = projectData->GetPinnedControls();
    for (const ProjectData::PinnedControl& pinnedControl : pinnedControls)
    {
        auto pkgFound = std::find_if(libraryPackages.begin(), libraryPackages.end(), [&pinnedControl](const RefPtr<PackageNode>& pkg)
                                     {
                                         return (pkg->GetPath() == pinnedControl.packagePath.absolute);
                                     });
        if (pkgFound != libraryPackages.end())
        {
            PackageControlsNode* packageControls = pkgFound->Get()->GetPackageControlsNode();
            ControlNode* controlNode = packageControls->FindChildByName(pinnedControl.controlName);
            if (controlNode != nullptr)
            {
                QString controlIconPath;
                if (pinnedControl.iconPath.absolute.IsEmpty() == false)
                {
                    controlIconPath = QString::fromStdString(pinnedControl.iconPath.absolute.GetAbsolutePathname());
                }
                else if (controlNode->GetPrototype() != nullptr)
                {
                    controlIconPath = IconHelper::GetCustomIconPath();
                }
                else
                {
                    QString className = QString::fromStdString(controlNode->GetControl()->GetClassName());
                    controlIconPath = IconHelper::GetIconPathForClassName(className);
                }

                LibraryData::ActionInfo actionInfo;
                actionInfo.action = new QtAction(GetAccessor(), QIcon(controlIconPath), QString::fromStdString(pinnedControl.controlName));
                actionInfo.action->setEnabled(false);
                actionInfo.action->setObjectName(GenerateUniqueName());
                InsertionParams insertionParams = { InsertionParams::eInsertionMethod::BeforeItem, menuNameLibrary };
                actionInfo.placement.AddPlacementPoint(CreateMenuPoint("Controls", insertionParams));
                actionInfo.placement.AddPlacementPoint(CreateToolbarPoint(LibraryModuleDetails::controlsToolbarName, insertionParams));
                actionInfo.action->SetStateUpdationFunction(QtAction::Enabled, packageField, [](const Any& fieldValue) -> Any {
                    PackageNode* package = fieldValue.Cast<PackageNode*>(nullptr);
                    return (package != nullptr);
                });
                connections.AddConnection(actionInfo.action, &QAction::triggered, DAVA::Bind(&LibraryModule::OnControlCreateTriggered, this, controlNode, false));
                GetUI()->AddAction(DAVA::mainWindowKey, actionInfo.placement, actionInfo.action);

                libraryData->controlsActions.emplace(controlNode, std::move(actionInfo));
            }
            else
            {
                NotificationParams params;
                params.title = "Project file contains errors";
                params.message.type = Result::RESULT_ERROR;
                params.message.message = Format("Can't find pinned control '%s' described in Controls section", pinnedControl.controlName.c_str());
                GetUI()->ShowNotification(DAVA::mainWindowKey, params);
            }
        }
    }
}

void LibraryModule::AddProjectLibraryControls(const ProjectData* projectData, const DAVA::Vector<DAVA::RefPtr<PackageNode>>& libraryPackages)
{
    using namespace DAVA;

    using namespace LibraryModuleDetails;

    FieldDescriptor packageField;
    packageField.type = ReflectedTypeDB::Get<DocumentData>();
    packageField.fieldName = FastName(DocumentData::packagePropertyName);

    LibraryData* libraryData = GetLibraryData();

    const Vector<ProjectData::LibrarySection> librarySections = projectData->GetLibrarySections();
    for (const ProjectData::LibrarySection& section : librarySections)
    {
        auto pkgFound = std::find_if(libraryPackages.begin(), libraryPackages.end(), [&section](const RefPtr<PackageNode>& pkg)
                                     {
                                         return (pkg->GetPath() == section.packagePath.absolute);
                                     });
        if (pkgFound != libraryPackages.end())
        {
            QString sectionName = CreateMenuName(QString::fromStdString(section.packagePath.absolute.GetBasename()));
            QString sectionIconPath = QString::fromStdString(section.iconPath.absolute.GetAbsolutePathname());

            QUrl menuPoint = CreateMenuPoint(QList<QString>() << "Controls" << menuNameLibrary << sectionName);
            QUrl toolbarMenuPoint = CreateToolbarMenuPoint(LibraryModuleDetails::controlsToolbarName, QList<QString>() << menuNameLibrary << sectionName);

            QUrl pinnedMenuPoint;
            QUrl pinnedToolbarMenuPoint;

            if (section.pinned == true)
            {
                { // create menu, insert into Controls menu and Controls toolbar
                    QAction* sectionMenu = new QAction(sectionName, nullptr);
                    ActionPlacementInfo placement;
                    InsertionParams insertionParams = { DAVA::InsertionParams::eInsertionMethod::BeforeItem, menuNameLibrary };
                    placement.AddPlacementPoint(CreateMenuPoint("Controls", insertionParams));
                    placement.AddPlacementPoint(CreateToolbarPoint(controlsToolbarName, insertionParams));
                    GetUI()->AddAction(mainWindowKey, placement, sectionMenu);
                }

                pinnedMenuPoint = CreateMenuPoint(QList<QString>() << "Controls" << sectionName);
                pinnedToolbarMenuPoint = CreateToolbarMenuPoint(LibraryModuleDetails::controlsToolbarName, QList<QString>() << sectionName);
            }

            PackageControlsNode* packageControls = pkgFound->Get()->GetPackageControlsNode();
            for (ControlNode* node : *packageControls)
            {
                QString iconPath;
                if (node->GetPrototype() != nullptr)
                {
                    iconPath = IconHelper::GetCustomIconPath();
                }
                else
                {
                    QString className = QString::fromStdString(node->GetControl()->GetClassName());
                    iconPath = IconHelper::GetIconPathForClassName(className);
                }

                QString controlName = QString::fromStdString(node->GetName());

                LibraryData::ActionInfo actionInfo;
                actionInfo.action = new QtAction(GetAccessor(), QIcon(iconPath), controlName);
                actionInfo.action->setEnabled(false);
                actionInfo.action->setObjectName(GenerateUniqueName());
                actionInfo.placement.AddPlacementPoint(menuPoint);
                actionInfo.placement.AddPlacementPoint(toolbarMenuPoint);
                if (section.pinned == true)
                {
                    actionInfo.placement.AddPlacementPoint(pinnedMenuPoint);
                    actionInfo.placement.AddPlacementPoint(pinnedToolbarMenuPoint);
                }
                actionInfo.action->SetStateUpdationFunction(QtAction::Enabled, packageField, [](const Any& fieldValue) -> Any {
                    PackageNode* package = fieldValue.Cast<PackageNode*>(nullptr);
                    return (package != nullptr);
                });
                connections.AddConnection(actionInfo.action, &QAction::triggered, DAVA::Bind(&LibraryModule::OnControlCreateTriggered, this, node, false));

                GetUI()->AddAction(DAVA::mainWindowKey, actionInfo.placement, actionInfo.action);
                libraryData->controlsActions.emplace(node, std::move(actionInfo));
            }
        }
    }
}

void LibraryModule::AddDefaultControls()
{
    using namespace DAVA;

    using namespace LibraryModuleDetails;

    FieldDescriptor packageField;
    packageField.type = ReflectedTypeDB::Get<DocumentData>();
    packageField.fieldName = FastName(DocumentData::packagePropertyName);

    LibraryData* libraryData = GetLibraryData();

    QUrl menuPoint = CreateMenuPoint(QList<QString>() << "Controls" << menuNameLibrary << "Default Controls");
    QUrl toolbarMenuPoint = CreateToolbarMenuPoint(LibraryModuleDetails::controlsToolbarName, QList<QString>() << menuNameLibrary << "Default Controls");

    for (const RefPtr<ControlNode>& control : libraryData->GetDefaultControls())
    {
        QString className = QString::fromStdString(control->GetControl()->GetClassName());
        QString iconPath = IconHelper::GetIconPathForClassName(className);

        QString controlName = QString::fromStdString(control->GetName());

        LibraryData::ActionInfo actionInfo;
        actionInfo.action = new QtAction(GetAccessor(), QIcon(iconPath), controlName);
        actionInfo.action->setEnabled(false);
        actionInfo.action->setObjectName(GenerateUniqueName());
        actionInfo.placement.AddPlacementPoint(menuPoint);
        actionInfo.placement.AddPlacementPoint(toolbarMenuPoint);
        actionInfo.action->SetStateUpdationFunction(QtAction::Enabled, packageField, [](const Any& fieldValue) -> Any {
            PackageNode* package = fieldValue.Cast<PackageNode*>(nullptr);
            return (package != nullptr);
        });
        connections.AddConnection(actionInfo.action, &QAction::triggered, DAVA::Bind(&LibraryModule::OnControlCreateTriggered, this, control.Get(), false));

        GetUI()->AddAction(DAVA::mainWindowKey, actionInfo.placement, actionInfo.action);
        libraryData->controlsActions.emplace(control.Get(), std::move(actionInfo));
    }
}

void LibraryModule::RemoveProjectControls()
{
    ClearActions(GetLibraryData()->controlsActions);
}

void LibraryModule::AddControlAction(ControlNode* controlNode, bool isPrototype, const QUrl& menuPoint, const QUrl& toolbarMenuPoint, LibraryData::ActionsMap& actionsMap)
{
    QString iconPath = IconHelper::GetCustomIconPath();
    QString controlName = QString::fromStdString(controlNode->GetName());

    LibraryData::ActionInfo actionInfo;
    actionInfo.action = new DAVA::QtAction(GetAccessor(), QIcon(iconPath), controlName);
    actionInfo.action->setObjectName(GenerateUniqueName());
    actionInfo.placement.AddPlacementPoint(menuPoint);
    actionInfo.placement.AddPlacementPoint(toolbarMenuPoint);
    connections.AddConnection(actionInfo.action, &QAction::triggered, DAVA::Bind(&LibraryModule::OnControlCreateTriggered, this, controlNode, isPrototype));
    GetUI()->AddAction(DAVA::mainWindowKey, actionInfo.placement, actionInfo.action);
    actionsMap.emplace(controlNode, std::move(actionInfo));
}

void LibraryModule::AddPrototypes(const PackageNode* packageNode, const QUrl& menuPoint, const QUrl& toolbarMenuPoint)
{
    PackageControlsNode* prototypes = packageNode->GetPrototypes();
    if (prototypes != nullptr)
    {
        for (ControlNode* control : *prototypes)
        {
            AddControlAction(control, true, menuPoint, toolbarMenuPoint, GetLibraryData()->prototypesActions);
        }
    }
};

void LibraryModule::AddPackagePrototypes(PackageNode* packageNode)
{
    using namespace LibraryModuleDetails;

    QUrl menuPoint = DAVA::CreateMenuPoint(QList<QString>() << "Controls" << menuNamePrototypes);
    QUrl toolbarMenuPoint = DAVA::CreateToolbarMenuPoint(LibraryModuleDetails::controlsToolbarName, QList<QString>() << menuNamePrototypes);

    AddPrototypes(packageNode, menuPoint, toolbarMenuPoint);

    ImportedPackagesNode* importedPackages = packageNode->GetImportedPackagesNode();
    if (importedPackages != nullptr)
    {
        for (const PackageNode* package : *importedPackages)
        {
            AddImportedPrototypes(package);
        }
    }
}

void LibraryModule::AddImportedPrototypes(const PackageNode* package)
{
    using namespace LibraryModuleDetails;

    QList<QString> path({ "Controls", menuNamePrototypes, QString::fromStdString(package->GetName()) });
    QUrl menuPoint = DAVA::CreateMenuPoint(path);

    path.pop_front();
    QUrl toolbarMenuPoint = DAVA::CreateToolbarMenuPoint(LibraryModuleDetails::controlsToolbarName, path);

    AddPrototypes(package, menuPoint, toolbarMenuPoint);
}

void LibraryModule::RemoveImportedPrototypes(const PackageNode* importedPackage)
{
    PackageControlsNode* prototypes = importedPackage->GetPrototypes();
    for (ControlNode* controlNode : *prototypes)
    {
        RemoveControlAction(controlNode, GetLibraryData()->prototypesActions);
    }
}

void LibraryModule::RemovePackagePrototypes()
{
    ClearActions(GetLibraryData()->prototypesActions);
}

void LibraryModule::ClearActions(LibraryData::ActionsMap& actionsMap)
{
    for (const std::pair<ControlNode*, LibraryData::ActionInfo>& entry : actionsMap)
    {
        GetUI()->RemoveAction(DAVA::mainWindowKey, entry.second.placement, entry.second.action->objectName());
    }
    actionsMap.clear();
}

void LibraryModule::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    if (property->GetName() == "Name")
    {
        LibraryData* data = GetLibraryData();
        auto nodeFound = data->prototypesActions.find(node);
        if (nodeFound != data->prototypesActions.end())
        {
            QString newName = QString::fromStdString(property->GetValue().Get<DAVA::FastName>().c_str());
            nodeFound->second.action->setText(newName);
        }
    }
}

void LibraryModule::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int row)
{
    using namespace LibraryModuleDetails;

    Q_UNUSED(destination);
    Q_UNUSED(row);
    DVASSERT(nullptr != node);

    LibraryData* data = GetLibraryData();

    if (node->GetParent() == data->currentPackageNode->GetPrototypes())
    {
        QUrl menuPoint = DAVA::CreateMenuPoint(QList<QString>() << "Controls" << menuNamePrototypes);
        QUrl toolbarMenuPoint = DAVA::CreateToolbarMenuPoint(LibraryModuleDetails::controlsToolbarName, QList<QString>() << menuNamePrototypes);
        AddControlAction(node, true, menuPoint, toolbarMenuPoint, data->prototypesActions);
    }
}

void LibraryModule::ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from)
{
    Q_UNUSED(from);
    DVASSERT(nullptr != node);
    RemoveControlAction(node, GetLibraryData()->prototypesActions);
}

void LibraryModule::RemoveControlAction(ControlNode* node, LibraryData::ActionsMap& actionsMap)
{
    auto rangeFound = actionsMap.equal_range(node);
    for (auto iter = rangeFound.first; iter != rangeFound.second; ++iter)
    {
        LibraryData::ActionInfo& actionInfo = iter->second;
        GetUI()->RemoveAction(DAVA::mainWindowKey, actionInfo.placement, actionInfo.action->objectName());
    }
    actionsMap.erase(rangeFound.first, rangeFound.second);
}

void LibraryModule::ImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index)
{
    Q_UNUSED(to);
    Q_UNUSED(index);
    DVASSERT(nullptr != node);
    if (node->GetParent() == GetLibraryData()->currentPackageNode->GetImportedPackagesNode())
    {
        AddImportedPrototypes(node);
    }
}

void LibraryModule::ImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from)
{
    Q_UNUSED(from);
    DVASSERT(nullptr != node);

    if (node->GetParent() == GetLibraryData()->currentPackageNode->GetImportedPackagesNode())
    {
        RemoveImportedPrototypes(node);
    }
}

DECL_TARC_MODULE(LibraryModule);
