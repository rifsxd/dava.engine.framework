#include "Classes/Application/QEGlobal.h"
#include "Classes/EditorSystems/ControlTransformationSettings.h"
#include "Classes/EditorSystems/EditorSystemsManager.h"
#include "Classes/EditorSystems/SelectionSystem.h"
#include "Classes/EditorSystems/UserAssetsSettings.h"
#include "Classes/Interfaces/PackageActionsInterface.h"
#include "Classes/Model/ControlProperties/RootProperty.h"
#include "Classes/Model/PackageHierarchy/ControlNode.h"
#include "Classes/Model/PackageHierarchy/PackageControlsNode.h"
#include "Classes/Model/PackageHierarchy/PackageNode.h"
#include "Classes/Model/QuickEdPackageBuilder.h"
#include "Classes/Model/YamlPackageSerializer.h"
#include "Classes/Modules/DocumentsModule/DocumentData.h"
#include "Classes/Modules/DocumentsModule/DocumentsModule.h"
#include "Classes/Modules/DocumentsModule/EditorSystemsData.h"
#include "Classes/Modules/DocumentsModule/Private/DocumentsWatcherData.h"
#include "Classes/Modules/LegacySupportModule/Private/Project.h"
#include "Classes/Modules/PackageModule/PackageWidgetSettings.h"
#include "Classes/Painter/Painter.h"
#include "Classes/QECommands/ChangePropertyValueCommand.h"
#include "Classes/UI/CommandExecutor.h"
#include "Classes/UI/Find/FindInDocumentController.h"
#include "Classes/UI/Preview/PreviewWidget.h"
#include "Classes/UI/Preview/PreviewWidgetSettings.h"
#include "Classes/UI/ProjectView.h"
#include "Classes/UI/mainwindow.h"
#include "Classes/Utils/DragNDropHelper.h"

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/UI.h>

#include <QtTools/InputDialogs/MultilineTextInputDialog.h>

#include <Base/Any.h>
#include <Command/CommandStack.h>
#include <Engine/Engine.h>
#include <Engine/PlatformApiQt.h>
#include <Engine/Qt/RenderWidget.h>
#include <Engine/Window.h>
#include <Functional/Signal.h>
#include <Particles/ParticleEmitter.h>
#include <Reflection/Reflection.h>
#include <Render/DynamicBufferAllocator.h>
#include <Render/Renderer.h>
#include <UI/Render/UIRenderSystem.h>
#include <UI/Text/UITextComponent.h>
#include <UI/UIControl.h>
#include <UI/UIPackageLoader.h>

#include <QAction>
#include <QApplication>
#include <QFileSystemWatcher>
#include <QMouseEvent>

DAVA_VIRTUAL_REFLECTION_IMPL(DocumentsModule)
{
    DAVA::ReflectionRegistrator<DocumentsModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DocumentsModule::DocumentsModule()
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SelectionSettings);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ControlTransformationSettings);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PreviewWidgetSettings);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UserAssetsSettings);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PackageWidgetSettings);
}

DocumentsModule::~DocumentsModule() = default;

void DocumentsModule::OnRenderSystemInitialized(DAVA::Window* window)
{
    using namespace DAVA;

    Renderer::SetDesiredFPS(60);
    DynamicBufferAllocator::SetPageSize(DynamicBufferAllocator::DEFAULT_PAGE_SIZE);

    //we can create graphic items only when render is initialized
    EditorSystemsData* systemsData = GetAccessor()->GetGlobalContext()->GetData<EditorSystemsData>();
    systemsData->painter = std::make_unique<Painting::Painter>();
    Vector<Window*> windows = Engine::Instance()->GetWindows();
    DVASSERT(windows.size() == 1);
    windows.front()->draw.Connect(systemsData->painter.get(), static_cast<void (Painting::Painter::*)(DAVA::Window*)>(&Painting::Painter::OnFrame));
}

bool DocumentsModule::CanWindowBeClosedSilently(const DAVA::WindowKey& key, DAVA::String& requestWindowText)
{
    using namespace DAVA;

    DVASSERT(DAVA::mainWindowKey == key);
    QString windowText = QObject::tr("Save changes to the following items?\n");
    QStringList unsavedDocuments;
    ContextAccessor* accessor = GetAccessor();
    accessor->ForEachContext([&unsavedDocuments](DataContext& context) {
        DocumentData* data = context.GetData<DocumentData>();
        if (data->CanSave())
        {
            unsavedDocuments << data->GetName();
        }
    });

    const int maxDisplayableDocumentsCount = 15;
    if (unsavedDocuments.size() > maxDisplayableDocumentsCount)
    {
        unsavedDocuments = unsavedDocuments.mid(0, maxDisplayableDocumentsCount);
        unsavedDocuments << "...";
    }
    windowText.append(unsavedDocuments.join("\n"));
    requestWindowText = windowText.toStdString();

    return HasUnsavedDocuments() == false;
}

bool DocumentsModule::SaveOnWindowClose(const DAVA::WindowKey& key)
{
    return SaveAllDocuments();
}

void DocumentsModule::RestoreOnWindowClose(const DAVA::WindowKey& key)
{
    DiscardUnsavedChanges();
}

void DocumentsModule::PostInit()
{
    using namespace DAVA;

    packageListenerProxy.Init(this, GetAccessor());

    InitGlobalData();

    InitCentralWidget();

    RegisterOperations();
    CreateDocumentsActions();
    CreateEditActions();
    CreateViewActions();
    CreateFindActions();

    EditorSystemsManager* systemsManager = GetAccessor()->GetGlobalContext()->GetData<EditorSystemsData>()->systemsManager.get();
    RegisterInterface(static_cast<Interfaces::EditorSystemsManagerInterface*>(systemsManager));
}

void DocumentsModule::OnWindowClosed(const DAVA::WindowKey& key)
{
    using namespace DAVA;

    DeleteAllDocuments();

    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetGlobalContext();
    context->DeleteData<DocumentsWatcherData>();
}

void DocumentsModule::OnInterfaceRegistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<Interfaces::PackageActionsInterface>())
    {
        Interfaces::PackageActionsInterface* packageActions = QueryInterface<Interfaces::PackageActionsInterface>();
        previewWidget->RegisterPackageActions(packageActions);
    }
}

void DocumentsModule::OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<Interfaces::PackageActionsInterface>() && previewWidget.isNull() == false)
    {
        previewWidget->UnregisterPackageActions();
    }
}

void DocumentsModule::OnContextCreated(DAVA::DataContext* context)
{
    using namespace DAVA;
    DocumentData* data = context->GetData<DocumentData>();
    DVASSERT(nullptr != data);
    QString path = data->GetPackageAbsolutePath();
    DataContext* globalContext = GetAccessor()->GetGlobalContext();
    DocumentsWatcherData* watcherData = globalContext->GetData<DocumentsWatcherData>();
    watcherData->Watch(path);
}

void DocumentsModule::OnContextDeleted(DAVA::DataContext* context)
{
    using namespace DAVA;
    DocumentData* data = context->GetData<DocumentData>();
    QString path = data->GetPackageAbsolutePath();
    DataContext* globalContext = GetAccessor()->GetGlobalContext();
    DocumentsWatcherData* watcherData = globalContext->GetData<DocumentsWatcherData>();
    watcherData->Unwatch(path);
}

void DocumentsModule::OnContextWillBeChanged(DAVA::DataContext* current, DAVA::DataContext* newOne)
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    EditorSystemsManager* systemsManager = accessor->GetGlobalContext()->GetData<EditorSystemsData>()->systemsManager.get();
    systemsManager->Invalidate();
}

void DocumentsModule::InitCentralWidget()
{
    using namespace DAVA;

    UI* ui = GetUI();
    ContextAccessor* accessor = GetAccessor();

    RenderWidget* renderWidget = GetContextManager()->GetRenderWidget();

    EditorSystemsManager* systemsManager = accessor->GetGlobalContext()->GetData<EditorSystemsData>()->systemsManager.get();
    previewWidget = new PreviewWidget(accessor, GetInvoker(), ui, renderWidget, systemsManager);
    previewWidget->requestCloseTab.Connect(this, &DocumentsModule::CloseDocument);
    previewWidget->requestChangeTextInNode.Connect(this, &DocumentsModule::ChangeControlText);
    previewWidget->droppingFile.Connect(this, &DocumentsModule::OnDroppingFile);
    connections.AddConnection(previewWidget.data(), &PreviewWidget::OpenPackageFiles, MakeFunction(this, &DocumentsModule::OpenPackageFiles));

    PanelKey panelKey(QStringLiteral("CentralWidget"), CentralPanelInfo());
    ui->AddView(DAVA::mainWindowKey, panelKey, previewWidget.data());

    //legacy part. Remove it when package will be refactored
    MainWindow* mainWindow = qobject_cast<MainWindow*>(ui->GetWindow(DAVA::mainWindowKey));
    connections.AddConnection(mainWindow, &MainWindow::EmulationModeChanged, MakeFunction(this, &DocumentsModule::OnEmulationModeChanged));
}

void DocumentsModule::InitGlobalData()
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();

    std::unique_ptr<DocumentsWatcherData> watcherData(new DocumentsWatcherData());
    connections.AddConnection(watcherData->watcher.get(), &QFileSystemWatcher::fileChanged, MakeFunction(this, &DocumentsModule::OnFileChanged));
    QApplication* app = PlatformApi::Qt::GetApplication();
    connections.AddConnection(app, &QApplication::applicationStateChanged, MakeFunction(this, &DocumentsModule::OnApplicationStateChanged));
    globalContext->CreateData(std::move(watcherData));

    std::unique_ptr<EditorSystemsData> editorData(new EditorSystemsData());
    editorData->systemsManager = std::make_unique<EditorSystemsManager>(GetAccessor());
    globalContext->CreateData(std::move(editorData));

    EditorSystemsManager* systemsManager = globalContext->GetData<EditorSystemsData>()->systemsManager.get();
    systemsManager->InitSystems();
}

void DocumentsModule::CreateDocumentsActions()
{
    using namespace DAVA;

    const QString toolBarName("Main Toolbar");

    const QString saveDocumentActionName("Save document");
    const QString saveAllDocumentsActionName("Force save all");
    const QString reloadDocumentActionName("Reload document");
    const QString toolBarSeparatorName("documents separator");

    ContextAccessor* accessor = GetAccessor();

    UI* ui = GetUI();
    //action save document
    {
        QtAction* action = new QtAction(accessor, QIcon(":/Icons/savescene.png"), saveDocumentActionName);
        action->setShortcut(QKeySequence("Ctrl+S"));
        action->setShortcutContext(Qt::ApplicationShortcut);

        connections.AddConnection(action, &QAction::triggered, Bind(&DocumentsModule::SaveCurrentDocument, this));

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::canSavePropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any {
            return fieldValue.Cast<bool>(false);
        });

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuFile, { InsertionParams::eInsertionMethod::AfterItem }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName));

        ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
    }

    //action save all documents
    {
        QAction* action = new QAction(QIcon(":/Icons/savesceneall.png"), saveAllDocumentsActionName, nullptr);
        action->setShortcut(QKeySequence("Ctrl+Shift+S"));
        action->setShortcutContext(Qt::ApplicationShortcut);

        connections.AddConnection(action, &QAction::triggered, Bind(&DocumentsModule::SaveAllDocuments, this));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuFile, { InsertionParams::eInsertionMethod::AfterItem, saveDocumentActionName }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName));

        ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
    }

    // action reload document
    {
        QtAction* action = new QtAction(accessor, reloadDocumentActionName);
        action->setShortcuts(QList<QKeySequence>()
                             << QKeySequence("Ctrl+R")
                             << QKeySequence("F5"));

        action->setShortcutContext(Qt::ApplicationShortcut);

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any {
            return fieldValue.CanCast<PackageNode*>() && fieldValue.Cast<PackageNode*>() != nullptr;
        });

        connections.AddConnection(action, &QAction::triggered, Bind(&DocumentsModule::ReloadCurrentDocument, this));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuFile, { InsertionParams::eInsertionMethod::AfterItem, saveAllDocumentsActionName }));
        ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
    }

    // Separator
    {
        QAction* separator = new QAction(toolBarSeparatorName, nullptr);
        separator->setObjectName(toolBarSeparatorName);
        separator->setSeparator(true);
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName));
        ui->AddAction(DAVA::mainWindowKey, placementInfo, separator);
    }
}

void DocumentsModule::CreateEditActions()
{
    using namespace DAVA;

    const QString undoActionName("Undo");
    const QString redoActionName("Redo");

    const QString toolBarName("Main Toolbar");
    const QString editMenuSeparatorName("undo redo separator");

    Function<Any(QString, const Any&)> makeActionName = [](QString baseName, const Any& actionText) {
        if (actionText.CanCast<QString>())
        {
            QString text = actionText.Cast<QString>();
            if (text.isEmpty() == false)
            {
                baseName += ": " + text;
            }
        }
        return Any(baseName);
    };

    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    //Undo
    {
        QtAction* action = new QtAction(accessor, QIcon(":/Icons/edit_undo.png"), undoActionName);
        action->setShortcutContext(Qt::ApplicationShortcut);
        action->setShortcut(QKeySequence("Ctrl+Z"));

        FieldDescriptor fieldDescrCanUndo;
        fieldDescrCanUndo.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescrCanUndo.fieldName = FastName(DocumentData::canUndoPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescrCanUndo, [](const Any& fieldValue) -> Any {
            return fieldValue.Cast<bool>(false);
        });

        FieldDescriptor fieldDescrUndoText;
        fieldDescrUndoText.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescrUndoText.fieldName = FastName(DocumentData::undoTextPropertyName);
        action->SetStateUpdationFunction(QtAction::Text, fieldDescrUndoText, Bind(makeActionName, "Undo", _1));
        action->SetStateUpdationFunction(QtAction::Tooltip, fieldDescrUndoText, Bind(makeActionName, "Undo", _1));

        connections.AddConnection(action, &QAction::triggered, MakeFunction(this, &DocumentsModule::OnUndo));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuEdit, { InsertionParams::eInsertionMethod::BeforeItem }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName));

        ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
    }

    //Redo
    {
        QtAction* action = new QtAction(accessor, QIcon(":/Icons/edit_redo.png"), redoActionName);
        action->setShortcutContext(Qt::ApplicationShortcut);
        action->setShortcuts(QList<QKeySequence>()
                             << QKeySequence("Ctrl+Y")
                             << QKeySequence("Ctrl+Shift+Z"));

        FieldDescriptor fieldDescrCanRedo;
        fieldDescrCanRedo.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescrCanRedo.fieldName = FastName(DocumentData::canRedoPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescrCanRedo, [](const Any& fieldValue) -> Any {
            return fieldValue.Cast<bool>(false);
        });

        FieldDescriptor fieldDescrUndoText;
        fieldDescrUndoText.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescrUndoText.fieldName = FastName(DocumentData::redoTextPropertyName);
        action->SetStateUpdationFunction(QtAction::Text, fieldDescrUndoText, Bind(makeActionName, "Redo", _1));
        action->SetStateUpdationFunction(QtAction::Tooltip, fieldDescrUndoText, Bind(makeActionName, "Redo", _1));

        connections.AddConnection(action, &QAction::triggered, MakeFunction(this, &DocumentsModule::OnRedo));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuEdit, { InsertionParams::eInsertionMethod::AfterItem, undoActionName }));
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName));

        ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
    }

    // Separator
    {
        QAction* separator = new QAction(editMenuSeparatorName, nullptr);
        separator->setSeparator(true);
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateToolbarPoint(toolBarName));
        ui->AddAction(DAVA::mainWindowKey, placementInfo, separator);
    }
}

void DocumentsModule::OnUndo()
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetActiveContext();
    DVASSERT(context != nullptr);
    DocumentData* data = context->GetData<DocumentData>();
    DVASSERT(data != nullptr);
    data->commandStack->Undo();
}

void DocumentsModule::OnRedo()
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetActiveContext();
    DVASSERT(context != nullptr);
    DocumentData* data = context->GetData<DocumentData>();
    DVASSERT(data != nullptr);
    data->commandStack->Redo();
}

void DocumentsModule::CreateViewActions()
{
    using namespace DAVA;

    const QString zoomInActionName("Zoom In");
    const QString zoomOutActionName("Zoom Out");
    const QString actualZoomActionName("Actual zoom");
    const QString zoomSeparator("zoom separator");

    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    // Separator
    {
        QAction* separator = new QAction(nullptr);
        separator->setObjectName(zoomSeparator);
        separator->setSeparator(true);
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuView, { InsertionParams::eInsertionMethod::AfterItem, "Dock" }));
        ui->AddAction(DAVA::mainWindowKey, placementInfo, separator);
    }

    //Zoom in
    {
        QtAction* action = new QtAction(accessor, zoomInActionName, nullptr);
        action->setShortcutContext(Qt::WindowShortcut);
        action->setShortcuts(QList<QKeySequence>()
                             << QKeySequence("Ctrl+=")
                             << QKeySequence("Ctrl++"));

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any {
            return fieldValue.CanCast<PackageNode*>() && fieldValue.Cast<PackageNode*>() != nullptr;
        });

        connections.AddConnection(action, &QAction::triggered, MakeFunction(previewWidget.data(), &PreviewWidget::OnIncrementScale));
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuView, { InsertionParams::eInsertionMethod::AfterItem, zoomSeparator }));

        ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
    }

    //Zoom out
    {
        QtAction* action = new QtAction(accessor, zoomOutActionName, nullptr);
        action->setShortcutContext(Qt::WindowShortcut);
        action->setShortcut(QKeySequence("Ctrl+-"));

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any {
            return fieldValue.CanCast<PackageNode*>() && fieldValue.Cast<PackageNode*>() != nullptr;
        });

        connections.AddConnection(action, &QAction::triggered, MakeFunction(previewWidget.data(), &PreviewWidget::OnDecrementScale));

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuView, { InsertionParams::eInsertionMethod::AfterItem, zoomInActionName }));

        ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
    }

    //Actual zoom
    {
        QtAction* action = new QtAction(accessor, actualZoomActionName, nullptr);
        action->setShortcutContext(Qt::WindowShortcut);
        action->setShortcut(QKeySequence("Ctrl+0"));

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any {
            return fieldValue.CanCast<PackageNode*>() && fieldValue.Cast<PackageNode*>() != nullptr;
        });

        connections.AddConnection(action, &QAction::triggered, MakeFunction(previewWidget.data(), &PreviewWidget::SetActualScale));

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuView, { InsertionParams::eInsertionMethod::AfterItem, zoomOutActionName }));

        ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
    }
}

void DocumentsModule::CreateFindActions()
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();
    {
        QtAction* action = new QtAction(accessor, "Select Current Document in File System");

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any {
            return fieldValue.CanCast<PackageNode*>() && fieldValue.Cast<PackageNode*>() != nullptr;
        });

        connections.AddConnection(action, &QAction::triggered, Bind(&DocumentsModule::OnSelectInFileSystem, this));

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuFind, { InsertionParams::eInsertionMethod::AfterItem }));

        ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
    }
}

void DocumentsModule::OpenPackageFiles(const QStringList& links)
{
    using namespace DAVA;

    ResultList wrongExtensionResults;
    ResultList wrongSourceResults;
    for (const QString& str : links)
    {
        QUrl url(str);
        if (url.isLocalFile())
        {
            QString path = url.toLocalFile();

            if (DragNDropHelper::IsExtensionSupported(path) == false)
            {
                wrongExtensionResults.AddResult(Result::RESULT_WARNING, Format("%s", path.toStdString().c_str()));
            }
            else if (DragNDropHelper::IsFileFromProject(GetAccessor(), path) == false)
            {
                wrongSourceResults.AddResult(Result::RESULT_WARNING, Format("%s", path.toStdString().c_str()));
            }
            else
            {
                OpenDocument(path);
            }
        }
    }

    if (wrongExtensionResults.HasWarnings())
    {
        DAVA::NotificationParams notificationParams;
        notificationParams.title = "can not drop";
        notificationParams.message = Result(Result::RESULT_WARNING, Format("next files have unsupported extension:\n%s", wrongExtensionResults.GetResultMessages().c_str()));
        GetUI()->ShowNotification(DAVA::mainWindowKey, notificationParams);
    }

    if (wrongSourceResults.HasWarnings())
    {
        DAVA::NotificationParams notificationParams;
        notificationParams.title = "can not drop";
        notificationParams.message = Result(Result::RESULT_WARNING, Format("next files are not from project:\n%s", wrongSourceResults.GetResultMessages().c_str()));
        GetUI()->ShowNotification(DAVA::mainWindowKey, notificationParams);
    }
}

void DocumentsModule::RegisterOperations()
{
    RegisterOperation(QEGlobal::OpenDocumentByPath.ID, this, &DocumentsModule::OpenDocument);
    RegisterOperation(QEGlobal::CloseAllDocuments.ID, this, &DocumentsModule::CloseAllDocuments);
    RegisterOperation(QEGlobal::SelectControl.ID, this, &DocumentsModule::SelectControl);
}

DAVA::DataContext::ContextID DocumentsModule::OpenDocument(const QString& path)
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DataContext::ContextID id = DataContext::Empty;
    accessor->ForEachContext([&id, path](const DAVA::DataContext& context) {
        DocumentData* data = context.GetData<DocumentData>();
        if (data->GetPackageAbsolutePath() == path)
        {
            DVASSERT(id == DataContext::Empty);
            id = context.GetID();
        }
    });

    ContextManager* contextManager = GetContextManager();

    if (id == DataContext::Empty)
    {
        RefPtr<PackageNode> package = CreatePackage(path);
        if (package != nullptr)
        {
            DAVA::Vector<std::unique_ptr<DAVA::TArcDataNode>> initialData;
            initialData.emplace_back(new DocumentData(package));
            id = contextManager->CreateContext(std::move(initialData));
        }
    }
    if (id != DataContext::Empty)
    {
        contextManager->ActivateContext(id);
    }
    return id;
}

DAVA::RefPtr<PackageNode> DocumentsModule::CreatePackage(const QString& path)
{
    using namespace DAVA;

    QString canonicalFilePath = QFileInfo(path).canonicalFilePath();
    FilePath davaPath(canonicalFilePath.toStdString());

    ProjectData* projectData = GetAccessor()->GetGlobalContext()->GetData<ProjectData>();
    DVASSERT(nullptr != projectData);

    QuickEdPackageBuilder builder(GetAccessor()->GetEngineContext());
    UIPackageLoader packageLoader(projectData->GetPrototypes());
    bool packageLoaded = packageLoader.LoadPackage(davaPath, &builder);

    if (packageLoaded == true)
    {
        bool canLoadPackage = true;

        if (builder.GetResults().HasErrors() == true)
        {
            ModalMessageParams params;
            params.icon = ModalMessageParams::Question;
            params.title = QObject::tr("Document was loaded with errors.");

            QString message = QObject::tr("Document by path:\n%1 was loaded with next errors:\n").arg(path);
            message.append(QString::fromStdString(builder.GetResults().GetResultMessages()));
            message.append("Would you like to load this document?");
            params.message = message;
            params.buttons = ModalMessageParams::Yes | ModalMessageParams::Cancel;
            canLoadPackage = GetUI()->ShowModalMessage(DAVA::mainWindowKey, params) == ModalMessageParams::Yes;
        }

        if (canLoadPackage)
        {
            RefPtr<PackageNode> packageRef = builder.BuildPackage();
            DVASSERT(packageRef.Get() != nullptr);
            return packageRef;
        }
    }
    else
    {
        //we need to continue current operation and not stop it with a modal message
        delayedExecutor.DelayedExecute([this, path]() {
            ModalMessageParams params;
            params.icon = ModalMessageParams::Warning;
            params.title = QObject::tr("Can not create document");
            params.message = QObject::tr("Can not create document by path:\n%1").arg(path);
            params.buttons = ModalMessageParams::Ok;
            GetUI()->ShowModalMessage(DAVA::mainWindowKey, params);
        });
    }
    return RefPtr<PackageNode>();
}

void DocumentsModule::SelectControl(const QString& documentPath, const QString& controlPath)
{
    using namespace DAVA;

    DataContext::ContextID id = OpenDocument(documentPath);
    if (id == DataContext::Empty)
    {
        return;
    }
    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetContext(id);
    DVASSERT(id == activeContext->GetID());
    DocumentData* data = activeContext->GetData<DocumentData>();
    const PackageNode* package = data->GetPackageNode();
    String controlPathStr = controlPath.toStdString();
    Set<PackageBaseNode*> foundNodes;
    package->GetPrototypes()->FindAllNodesByPath(controlPathStr, foundNodes);
    package->GetPackageControlsNode()->FindAllNodesByPath(controlPathStr, foundNodes);
    if (!foundNodes.empty())
    {
        data->SetSelectedNodes(foundNodes);
    }
}

void DocumentsModule::OnEmulationModeChanged(bool mode)
{
    GetAccessor()->GetGlobalContext()->GetData<EditorSystemsData>()->emulationMode = mode;
}

//TODO: generalize changes in central widget
void DocumentsModule::ChangeControlText(ControlNode* node)
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    EditorSystemsData* editorSystemsData = globalContext->GetData<EditorSystemsData>();
    if (editorSystemsData->systemsManager->GetDisplayState() == eDisplayState::Emulation)
    {
        return;
    }

    DVASSERT(node != nullptr);

    UIControl* control = node->GetControl();

    UITextComponent* textComponent = control->GetComponent<UITextComponent>();
    DVASSERT(textComponent != nullptr);

    RootProperty* rootProperty = node->GetRootProperty();
    AbstractProperty* textProperty = rootProperty->FindPropertyByName("text");
    DVASSERT(textProperty != nullptr);

    String text = textProperty->GetValue().Cast<String>();

    QString label = QObject::tr("Enter new text, please");
    bool ok;
    QString inputText = MultilineTextInputDialog::GetMultiLineText(GetUI(), label, label, QString::fromStdString(text), &ok);
    if (ok)
    {
        DataContext* activeContext = accessor->GetActiveContext();
        DVASSERT(nullptr != activeContext);
        DocumentData* data = activeContext->GetData<DocumentData>();
        CommandStack* stack = data->commandStack.get();
        stack->BeginBatch("change text by user");
        AbstractProperty* multilineProperty = rootProperty->FindPropertyByName("multiline");
        DVASSERT(multilineProperty != nullptr);

        // Update "multiline" property if need
        if (inputText.contains('\n'))
        {
            if (textComponent != nullptr && (multilineProperty->GetValue().Cast<UITextComponent::eTextMultiline>() == UITextComponent::MULTILINE_DISABLED))
            {
                std::unique_ptr<ChangePropertyValueCommand> command = data->CreateCommand<ChangePropertyValueCommand>();
                command->AddNodePropertyValue(node, multilineProperty, Any(UITextComponent::MULTILINE_ENABLED));
                data->ExecCommand(std::move(command));
            }
        }

        std::unique_ptr<ChangePropertyValueCommand> command = data->CreateCommand<ChangePropertyValueCommand>();
        command->AddNodePropertyValue(node, textProperty, Any(inputText.toStdString()));
        data->ExecCommand(std::move(command));
        stack->EndBatch();
    }
}

void DocumentsModule::CloseDocument(DAVA::uint64 id)
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    ContextManager* contextManager = GetContextManager();

    DataContext* context = accessor->GetContext(id);
    DVASSERT(context != nullptr);
    DocumentData* data = context->GetData<DocumentData>();
    DVASSERT(nullptr != data);
    if (data->CanClose() == false)
    {
        return;
    }

    if (data->CanSave())
    {
        QString status = data->documentExists ? "modified" : "renamed or removed";
        ModalMessageParams params;
        params.title = QObject::tr("Save changes");
        params.message = QObject::tr("The file %1 has been %2.\n"
                                     "Do you want to save it?")
                         .arg(data->GetName())
                         .arg(status);
        params.defaultButton = ModalMessageParams::Save;
        params.buttons = ModalMessageParams::Save | ModalMessageParams::Discard | ModalMessageParams::Cancel;
        ModalMessageParams::Button ret = GetUI()->ShowModalMessage(DAVA::mainWindowKey, params);

        if (ret == ModalMessageParams::Save)
        {
            if (!SaveDocument(id))
            {
                return;
            }
        }
        else if (ret == ModalMessageParams::Cancel)
        {
            return;
        }
    }
    contextManager->DeleteContext(id);
}

void DocumentsModule::CloseAllDocuments()
{
    using namespace DAVA;
    bool hasUnsaved = HasUnsavedDocuments();

    if (hasUnsaved)
    {
        ModalMessageParams params;
        params.title = QObject::tr("Save changes");
        params.message = QObject::tr("Some files has been modified.\n"
                                     "Do you want to save your changes?");
        params.buttons = ModalMessageParams::SaveAll | ModalMessageParams::NoToAll | ModalMessageParams::Cancel;
        params.icon = ModalMessageParams::Question;
        ModalMessageParams::Button button = GetUI()->ShowModalMessage(DAVA::mainWindowKey, params);
        if (button == ModalMessageParams::SaveAll)
        {
            hasUnsaved = (SaveAllDocuments() == false);
        }
        else if (button == ModalMessageParams::NoToAll)
        {
            DiscardUnsavedChanges();
            hasUnsaved = false;
        }
        else
        {
            return;
        }
    }

    if (!hasUnsaved)
    {
        DeleteAllDocuments();
    }
}

void DocumentsModule::DeleteAllDocuments()
{
    using namespace DAVA;
    ContextAccessor* accessor = GetAccessor();
    ContextManager* contextManager = GetContextManager();
    DAVA::Vector<DataContext::ContextID> contexts;
    accessor->ForEachContext([&contexts](DataContext& context)
                             {
                                 contexts.push_back(context.GetID());
                             });
    for (DataContext::ContextID id : contexts)
    {
        contextManager->DeleteContext(id);
    }
}

void DocumentsModule::CloseDocuments(const DAVA::Set<DAVA::DataContext::ContextID>& ids)
{
    using namespace DAVA;
    ContextAccessor* accessor = GetAccessor();
    ContextManager* contextManager = GetContextManager();
    for (const DataContext::ContextID& id : ids)
    {
        contextManager->ActivateContext(id);
        const DataContext* context = accessor->GetContext(id);
        DVASSERT(nullptr != context);
        const DocumentData* data = context->GetData<DocumentData>();
        DVASSERT(data != nullptr);
        ModalMessageParams::Button button = ModalMessageParams::No;
        ModalMessageParams params;
        params.title = QObject::tr("File %1 is renamed or removed").arg(data->GetName());
        params.icon = ModalMessageParams::Warning;
        params.message = QObject::tr("%1\n\nThis file has been renamed or removed. Do you want to close it?")
                         .arg(data->GetPackageAbsolutePath());
        params.buttons = ModalMessageParams::Yes | ModalMessageParams::No;
        params.defaultButton = ModalMessageParams::No;
        button = GetUI()->ShowModalMessage(DAVA::mainWindowKey, params);
        if (button == ModalMessageParams::Yes)
        {
            contextManager->DeleteContext(id);
        }
    }
}

void DocumentsModule::ReloadCurrentDocument()
{
    using namespace DAVA;
    ContextAccessor* accessor = GetAccessor();
    DataContext* active = accessor->GetActiveContext();
    DVASSERT(active != nullptr);
    if (active != nullptr)
    {
        ReloadDocument(active->GetID());
    }
}

void DocumentsModule::ReloadDocument(const DAVA::DataContext::ContextID& contextID)
{
    using namespace DAVA;

    ContextManager* contextManager = GetContextManager();
    contextManager->ActivateContext(contextID);

    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetContext(contextID);
    DVASSERT(context != nullptr);
    DVASSERT(contextID == accessor->GetActiveContext()->GetID());
    DocumentData* currentData = context->GetData<DocumentData>();
    if (currentData->CanClose() == false)
    {
        return;
    }

    QString path = currentData->GetPackageAbsolutePath();

    EditorSystemsManager* systemsManager = GetAccessor()->GetGlobalContext()->GetData<EditorSystemsData>()->systemsManager.get();
    systemsManager->Invalidate();

    RefPtr<PackageNode> package = CreatePackage(path);
    //if document was created successfully - delete previous data and create new one with new package.
    //this required because current program modules storing selection and another data as pointers to package children
    //else, if document was broken or damaged, can not use this context any more
    if (package != nullptr)
    {
        context->DeleteData<DocumentData>();
        context->CreateData(std::make_unique<DocumentData>(package));
    }
    else
    {
        contextManager->DeleteContext(contextID);
    }
}

void DocumentsModule::ReloadDocuments(const DAVA::Set<DAVA::DataContext::ContextID>& ids)
{
    using namespace DAVA;
    ContextAccessor* accessor = GetAccessor();

    bool yesToAll = false;
    bool noToAll = false;

    int changedCount = std::count_if(ids.begin(), ids.end(), [accessor](const DataContext::ContextID& id) {
        const DataContext* context = accessor->GetContext(id);
        DVASSERT(nullptr != context);
        const DocumentData* data = context->GetData<DocumentData>();
        DVASSERT(data != nullptr);
        return data->CanSave();
    });
    ContextManager* manager = GetContextManager();

    for (const DataContext::ContextID& id : ids)
    {
        const DataContext* context = accessor->GetContext(id);
        DVASSERT(nullptr != context);

        const DocumentData* data = context->GetData<DocumentData>();
        DVASSERT(data != nullptr);

        ModalMessageParams::Button button = ModalMessageParams::No;
        if (!data->CanSave())
        {
            button = ModalMessageParams::Yes;
        }
        else
        {
            if (!yesToAll && !noToAll)
            {
                //activate this document to show it to a user
                manager->ActivateContext(id);

                ModalMessageParams params;
                params.title = QObject::tr("File %1 changed").arg(data->GetName());
                params.message = QObject::tr("%1\n\nThis file has been modified outside of the editor. Do you want to reload it?").arg(data->GetPackageAbsolutePath());
                params.icon = ModalMessageParams::Warning;
                if (changedCount == 1)
                {
                    params.buttons = ModalMessageParams::Yes | ModalMessageParams::No | ModalMessageParams::Yes;
                }
                else
                {
                    params.buttons = ModalMessageParams::Yes | ModalMessageParams::YesToAll | ModalMessageParams::No | ModalMessageParams::NoToAll;
                }

                button = GetUI()->ShowModalMessage(DAVA::mainWindowKey, params);
                yesToAll = (button == ModalMessageParams::YesToAll);
                noToAll = (button == ModalMessageParams::NoToAll);
            }
            if (yesToAll)
            {
                button = ModalMessageParams::Yes;
            }
        }
        if (button == ModalMessageParams::Yes)
        {
            ReloadDocument(id);
        }
    }
}

bool DocumentsModule::HasUnsavedDocuments() const
{
    bool hasUnsaved = false;
    const DAVA::ContextAccessor* accessor = GetAccessor();
    accessor->ForEachContext([&hasUnsaved](const DAVA::DataContext& context) {
        DocumentData* data = context.GetData<DocumentData>();
        hasUnsaved |= (data->CanSave());
    });
    return hasUnsaved;
}

bool DocumentsModule::SaveDocument(const DAVA::DataContext::ContextID& contextID)
{
    using namespace DAVA;
    ContextAccessor* accessor = GetAccessor();
    DataContext* context = accessor->GetContext(contextID);
    DVASSERT(nullptr != context);
    DataContext* globalContext = accessor->GetGlobalContext();
    DocumentsWatcherData* watcherData = globalContext->GetData<DocumentsWatcherData>();
    DVASSERT(nullptr != watcherData);
    DocumentData* data = context->GetData<DocumentData>();

    QString path = data->GetPackageAbsolutePath();
    watcherData->Unwatch(path);
    YamlPackageSerializer serializer;
    serializer.SerializePackage(data->GetPackageNode());
    if (serializer.HasErrors())
    {
        ModalMessageParams params;
        params.title = QString::fromStdString(DAVA::Format("Can't save %s", data->package->GetPath().GetFilename().c_str()));
        params.message = QString::fromStdString(serializer.GetResults().GetResultMessages());

        params.buttons = ModalMessageParams::Ok;
        params.icon = ModalMessageParams::Warning;
        GetUI()->ShowModalMessage(DAVA::mainWindowKey, params);
        return false;
    }

    serializer.WriteToFile(data->package->GetPath());
    data->commandStack->SetClean();
    watcherData->Watch(path);

    return true;
}

bool DocumentsModule::SaveAllDocuments()
{
    bool savedOk = true;
    GetAccessor()->ForEachContext([&](DAVA::DataContext& context)
                                  {
                                      savedOk = SaveDocument(context.GetID()) && savedOk;
                                  });
    return savedOk;
}

bool DocumentsModule::SaveCurrentDocument()
{
    using namespace DAVA;
    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetActiveContext();

    return SaveDocument(activeContext->GetID());
}

void DocumentsModule::DiscardUnsavedChanges()
{
    GetAccessor()->ForEachContext([&](DAVA::DataContext& context)
                                  {
                                      DocumentData* data = context.GetData<DocumentData>();
                                      data->commandStack->SetClean();
                                  });
}

void DocumentsModule::OnFileChanged(const QString& path)
{
    using namespace DAVA;
    DataContext::ContextID id = GetContextByPath(path);

    ContextAccessor* accessor = GetAccessor();
    DocumentsWatcherData* watcherData = accessor->GetGlobalContext()->GetData<DocumentsWatcherData>();
    watcherData->changedDocuments.insert(id);
    DocumentData* data = GetAccessor()->GetContext(id)->GetData<DocumentData>();
    QFileInfo fileInfo(path);
    data->documentExists = fileInfo.exists();
    if (!data->CanSave() || qApp->applicationState() == Qt::ApplicationActive)
    {
        ApplyFileChanges();
    }
}

void DocumentsModule::OnApplicationStateChanged(Qt::ApplicationState state)
{
    if (state == Qt::ApplicationActive)
    {
        ApplyFileChanges();
    }
}

void DocumentsModule::ApplyFileChanges()
{
    using namespace DAVA;
    ContextAccessor* accessor = GetAccessor();

    DocumentsWatcherData* watcherData = accessor->GetGlobalContext()->GetData<DocumentsWatcherData>();
    DVASSERT(watcherData != nullptr);

    DAVA::Set<DAVA::DataContext::ContextID> changed;
    DAVA::Set<DAVA::DataContext::ContextID> removed;
    for (DataContext::ContextID id : watcherData->changedDocuments)
    {
        DataContext* context = accessor->GetContext(id);
        DVASSERT(nullptr != context);
        DocumentData* data = context->GetData<DocumentData>();

        if (data->documentExists)
        {
            changed.insert(id);
        }
        else
        {
            removed.insert(id);
        }
    }

    watcherData->changedDocuments.clear();

    if (!changed.empty())
    {
        ReloadDocuments(changed);
    }
    if (!removed.empty())
    {
        CloseDocuments(removed);
    }
}

DAVA::DataContext::ContextID DocumentsModule::GetContextByPath(const QString& path) const
{
    using namespace DAVA;
    DataContext::ContextID ret = DataContext::Empty;
    GetAccessor()->ForEachContext([path, &ret](const DataContext& context) {
        DocumentData* data = context.GetData<DocumentData>();
        if (data->GetPackageAbsolutePath() == path)
        {
            DVASSERT(ret == DataContext::Empty);
            ret = context.GetID();
        }
    });
    DVASSERT(ret != DataContext::Empty);
    return ret;
}

void DocumentsModule::ControlWillBeRemoved(ControlNode* nodeToRemove, ControlsContainerNode* /*from*/)
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetActiveContext();
    DocumentData* documentData = activeContext->GetData<DocumentData>();

    SortedControlNodeSet displayedRootControls = documentData->GetDisplayedRootControls();
    if (std::find(displayedRootControls.begin(), displayedRootControls.end(), nodeToRemove) != displayedRootControls.end())
    {
        displayedRootControls.erase(nodeToRemove);
    }
    documentData->SetDisplayedRootControls(displayedRootControls);

    EditorSystemsManager* systemsManager = GetAccessor()->GetGlobalContext()->GetData<EditorSystemsData>()->systemsManager.get();
    systemsManager->Invalidate(nodeToRemove);
}

void DocumentsModule::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int)
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetActiveContext();
    DocumentData* documentData = activeContext->GetData<DocumentData>();

    DataContext* globalContext = accessor->GetGlobalContext();
    EditorSystemsData* editorData = globalContext->GetData<EditorSystemsData>();
    const EditorSystemsManager* systemsManager = editorData->GetSystemsManager();

    eDisplayState displayState = systemsManager->GetDisplayState();
    if (displayState == eDisplayState::Preview || displayState == eDisplayState::Emulation)
    {
        PackageNode* package = documentData->GetPackageNode();
        PackageControlsNode* packageControlsNode = package->GetPackageControlsNode();
        if (destination == packageControlsNode)
        {
            SortedControlNodeSet displayedRootControls = documentData->GetDisplayedRootControls();
            displayedRootControls.insert(node);
            documentData->SetDisplayedRootControls(displayedRootControls);
        }
    }
}

void DocumentsModule::OnSelectInFileSystem()
{
    using namespace DAVA;

    DataContext* context = GetAccessor()->GetActiveContext();
    DVASSERT(context != nullptr);
    DocumentData* documentData = context->GetData<DocumentData>();
    QString filePath = documentData->GetPackageAbsolutePath();
    InvokeOperation(QEGlobal::SelectFile.ID, filePath);
}

void DocumentsModule::OnDroppingFile(bool droppingFile)
{
    DAVA::DataContext* globalContext = GetAccessor()->GetGlobalContext();
    EditorSystemsData* systemsData = globalContext->GetData<EditorSystemsData>();
    systemsData->highlightDisabled = droppingFile;
}
