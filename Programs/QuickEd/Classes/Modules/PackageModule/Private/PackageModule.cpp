#include "Classes/Modules/PackageModule/PackageModule.h"

#include "Classes/Application/QEGlobal.h"
#include "Classes/EditorSystems/SelectionContainer.h"
#include "Classes/Model/PackageHierarchy/PackageControlsNode.h"
#include "Classes/Model/PackageHierarchy/PackageIterator.h"
#include "Classes/Model/PackageHierarchy/StyleSheetNode.h"
#include "Classes/Model/PackageHierarchy/StyleSheetsNode.h"
#include "Classes/Model/YamlPackageSerializer.h"
#include "Classes/Modules/DocumentsModule/DocumentData.h"
#include "Classes/Modules/PackageModule/PackageData.h"
#include "Classes/Modules/PackageModule/PackageWidgetSettings.h"
#include "Classes/Modules/PackageModule/Private/PackageModel.h"
#include "Classes/Modules/PackageModule/Private/PackageTreeView.h"
#include "Classes/Modules/PackageModule/Private/UIViewerDialog.h"
#include "Classes/Modules/ProjectModule/ProjectData.h"
#include "Classes/UI/CommandExecutor.h"
#include "Classes/UI/Find/Filters/FindFilter.h"
#include "Classes/UI/Find/Filters/PrototypeUsagesFilter.h"

#include <QtHelpers/ProcessHelper.h>
#include <QtTools/FileDialogs/FileDialog.h>

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/Utils/CommonFieldNames.h>

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Engine/Window.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/LocalizationSystem.h>
#include <Reflection/ReflectedTypeDB.h>
#include <UI/UIControlSystem.h>

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QProcess>

namespace PackageModuleDetails
{
using namespace DAVA;

template <typename NodeType>
void CollectSelectedNodes(const SelectedNodes& selectedNodes, Vector<NodeType*>& nodes, bool forCopy, bool forRemove)
{
    SortedPackageBaseNodeSet sortedNodes(CompareByLCA);
    std::copy_if(selectedNodes.begin(), selectedNodes.end(), std::inserter(sortedNodes, sortedNodes.end()), [](typename SelectedNodes::value_type node) {
        return (dynamic_cast<NodeType*>(node) != nullptr);
    });
    for (PackageBaseNode* node : sortedNodes)
    {
        DVASSERT(nullptr != node);
        if (node->GetParent() != nullptr)
        {
            if ((!forCopy || node->CanCopy()) &&
                (!forRemove || node->CanRemove()))
            {
                PackageBaseNode* parent = node->GetParent();
                while (nullptr != parent && sortedNodes.find(parent) == sortedNodes.end())
                {
                    parent = parent->GetParent();
                }
                if (nullptr == parent)
                {
                    nodes.push_back(DynamicTypeCheck<NodeType*>(node));
                }
            }
        }
    }
}

void CollectSelectedControls(const SelectedNodes& selectedNodes, Vector<ControlNode*>& nodes, bool forCopy, bool forRemove)
{
    CollectSelectedNodes(selectedNodes, nodes, forCopy, forRemove);
}

void CollectSelectedImportedPackages(const SelectedNodes& selectedNodes, Vector<PackageNode*>& nodes, bool forCopy, bool forRemove)
{
    CollectSelectedNodes(selectedNodes, nodes, forCopy, forRemove);
}

void CollectSelectedStyles(const SelectedNodes& selectedNodes, Vector<StyleSheetNode*>& nodes, bool forCopy, bool forRemove)
{
    CollectSelectedNodes(selectedNodes, nodes, forCopy, forRemove);
}

void CopyNodesToClipboard(PackageNode* package, const Vector<ControlNode*>& controls, const Vector<StyleSheetNode*>& styles)
{
    QClipboard* clipboard = QApplication::clipboard();
    if (!controls.empty() || !styles.empty())
    {
        YamlPackageSerializer serializer;
        serializer.SerializePackageNodes(package, controls, styles);
        DAVA::String str = serializer.WriteToString();
        QMimeData* data = new QMimeData();
        data->setText(QString(str.c_str()));
        clipboard->setMimeData(data);
    }
}

bool CanInsertControlOrStyle(const PackageBaseNode* dest, PackageBaseNode* node, int32 destIndex)
{
    if (dynamic_cast<ControlNode*>(node))
    {
        return dest->CanInsertControl(static_cast<ControlNode*>(node), destIndex);
    }
    if (dynamic_cast<StyleSheetNode*>(node))
    {
        return dest->CanInsertStyle(static_cast<StyleSheetNode*>(node), destIndex);
    }
    else
    {
        return false;
    }
}

String BuildControlPath(ControlNode* targetNode, ControlNode* rootNode)
{
    static const String PATH_SEPARATOR("/");

    UIControl* control = targetNode->GetControl();
    UIControl* rootControl = rootNode->GetControl();

    if (!control)
        return "";

    String controlPath = "";
    while (control != rootControl)
    {
        if (controlPath.empty())
        {
            controlPath = String(control->GetName().c_str());
        }
        else
        {
            controlPath = String(control->GetName().c_str()) + PATH_SEPARATOR + controlPath;
        }
        control = control->GetParent();
    }

    return controlPath;
}
} // namespace PackageModuleDetails

DAVA_VIRTUAL_REFLECTION_IMPL(PackageModule)
{
    DAVA::ReflectionRegistrator<PackageModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void PackageModule::PostInit()
{
    InitData();
    CreatePackageWidget();
    CreateActions();
    RegisterGlobalOperation();
    RegisterInterface(static_cast<Interfaces::PackageActionsInterface*>(this));
    RegisterOperation(QEGlobal::Duplicate.ID, this, &PackageModule::OnDuplicate);
}

void PackageModule::InitData()
{
    std::unique_ptr<PackageData> packageData = std::make_unique<PackageData>();
    GetAccessor()->GetGlobalContext()->CreateData(std::move(packageData));

    documentDataWrapper = GetAccessor()->CreateWrapper(DAVA::ReflectedTypeDB::Get<DocumentData>());
    documentDataWrapper.SetListener(this);
}

void PackageModule::CreateActions()
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    PackageData* packageData = accessor->GetGlobalContext()->GetData<PackageData>();
    PackageWidget* packageWidget = packageData->packageWidget;

    enum Readonly : bool
    {
        READONLY = true,
        MODIFIABLE = false
    };
    auto makeBindable = [](QString blockName, QtAction* action, Readonly isReadonly)
    {
        KeyBindableActionInfo info;
        info.blockName = blockName;
        info.context = action->shortcutContext();
        info.defaultShortcuts << action->shortcuts();
        info.readOnly = isReadonly;
        MakeActionKeyBindable(action, info);
    };

    auto addActionIntoTreeViewOnly = [packageWidget, ui](QtAction* action)
    {
        packageWidget->treeView->addAction(action);

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateInvisiblePoint());
        ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
    };

    auto addSeparatorIntoTreeView = [packageWidget]()
    {
        QAction* separator = new QAction(packageWidget->treeView);
        separator->setSeparator(true);
        packageWidget->treeView->addAction(separator);
    };

    // Import package
    {
        const QString actionName = "Import package";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcut(QKeySequence::New);
        action->setShortcutContext(Qt::WidgetShortcut);

        packageData->importPackageAction = action;

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any
                                         {
                                             return fieldValue.Cast<PackageNode*>(nullptr) != nullptr;
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnImport, this));

        makeBindable("Package", action, MODIFIABLE);
        addActionIntoTreeViewOnly(action);
    }

    // Add Style
    {
        const QString actionName = "Add Style";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcutContext(Qt::WidgetShortcut);

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionHashPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [this](const Any& fieldValue) -> Any
                                         {
                                             const SelectedNodes& selectedNodes = GetSelection();
                                             return (selectedNodes.size() == 1 && (*selectedNodes.begin())->IsInsertingStylesSupported());
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnAddStyle, this));

        makeBindable("Package", action, MODIFIABLE);
        addActionIntoTreeViewOnly(action);
    }

    addSeparatorIntoTreeView();

    // Cut
    {
        const QString actionName = "Cut";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcut(QKeySequence::Cut);
        action->setShortcutContext(Qt::WidgetShortcut);

        packageData->cutAction = action;

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionHashPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [this](const Any& fieldValue) -> Any
                                         {
                                             const SelectedNodes& selectedNodes = GetSelection();
                                             return std::any_of(selectedNodes.begin(), selectedNodes.end(), [](PackageBaseNode* node)
                                                                {
                                                                    return node->CanCopy() && node->CanRemove();
                                                                });
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnCut, this));

        makeBindable("Package/CopyPaste", action, READONLY);
        addActionIntoTreeViewOnly(action);
    }

    // Copy
    {
        const QString actionName = "Copy";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcut(QKeySequence::Copy);
        action->setShortcutContext(Qt::WidgetShortcut);

        packageData->copyAction = action;

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionHashPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [this](const Any& fieldValue) -> Any
                                         {
                                             const SelectedNodes& selectedNodes = GetSelection();
                                             return std::any_of(selectedNodes.begin(), selectedNodes.end(), [](PackageBaseNode* node)
                                                                {
                                                                    return node->CanCopy();
                                                                });
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnCopy, this));

        makeBindable("Package/CopyPaste", action, READONLY);
        addActionIntoTreeViewOnly(action);
    }

    // Paste
    {
        const QString actionName = "Paste";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcut(QKeySequence::Paste);
        action->setShortcutContext(Qt::WidgetShortcut);

        packageData->pasteAction = action;

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionHashPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [this](const Any& fieldValue) -> Any
                                         {
                                             const SelectedNodes& selectedNodes = GetSelection();
                                             if (selectedNodes.empty() == true)
                                             {
                                                 return false;
                                             }
                                             PackageBaseNode* node = (*selectedNodes.begin());
                                             return (node->IsInsertingStylesSupported() || node->IsInsertingControlsSupported());
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnPaste, this));

        makeBindable("Package/CopyPaste", action, READONLY);
        addActionIntoTreeViewOnly(action);
    }

    // Duplicate
    {
        const QString actionName = "Duplicate";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
        action->setShortcutContext(Qt::WidgetShortcut);

        packageData->duplicateAction = action;

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionHashPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [this](const Any& fieldValue) -> Any
                                         {
                                             const SelectedNodes& selectedNodes = GetSelection();
                                             if (selectedNodes.empty())
                                             {
                                                 return false;
                                             }

                                             PackageBaseNode* parent = (*selectedNodes.begin())->GetParent();
                                             if (parent != nullptr && parent->IsInsertingControlsSupported() == true)
                                             {
                                                 return std::all_of(std::next(selectedNodes.begin()), selectedNodes.end(), [parent](const PackageBaseNode* node)
                                                                    {
                                                                        return node->GetParent() == parent;
                                                                    });
                                             }
                                             else
                                             {
                                                 return false;
                                             }
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnDuplicate, this));

        makeBindable("Package/CopyPaste", action, MODIFIABLE);
        addActionIntoTreeViewOnly(action);
    }

    addSeparatorIntoTreeView();

    // Copy Control Path
    {
        const QString actionName = "Copy Control Path";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcutContext(Qt::WidgetShortcut);

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionHashPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [this](const Any& fieldValue) -> Any
                                         {
                                             const SelectedNodes& selectedNodes = GetSelection();
                                             return std::any_of(selectedNodes.begin(), selectedNodes.end(), [](PackageBaseNode* node)
                                                                {
                                                                    return (node->GetControl() != nullptr);
                                                                });
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnCopyControlPath, this));

        makeBindable("Package", action, MODIFIABLE);
        addActionIntoTreeViewOnly(action);
    }

    addSeparatorIntoTreeView();

    // Rename
    {
        const QString actionName = "Rename";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcutContext(Qt::WidgetShortcut);

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionHashPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [this](const Any& fieldValue) -> Any
                                         {
                                             const SelectedNodes& selectedNodes = GetSelection();
                                             return (selectedNodes.size() == 1 && (*selectedNodes.begin())->IsEditingSupported());
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnRename, this));

        makeBindable("Package", action, MODIFIABLE);
        addActionIntoTreeViewOnly(action);
    }

    addSeparatorIntoTreeView();

    // Delete
    {
        const QString actionName = "Delete";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcutContext(Qt::WidgetShortcut);
        action->setShortcut(QKeySequence::Delete);
#if defined Q_OS_MAC
        action->setShortcuts({ QKeySequence::Delete, QKeySequence(Qt::Key_Backspace) });
#endif // platform

        packageData->deleteAction = action;

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionHashPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [this](const Any& fieldValue) -> Any
                                         {
                                             const SelectedNodes& selectedNodes = GetSelection();
                                             return std::any_of(selectedNodes.begin(), selectedNodes.end(), [](PackageBaseNode* node)
                                                                {
                                                                    return node->CanRemove();
                                                                });
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnDelete, this));

        makeBindable("Package/CopyPaste", action, MODIFIABLE);
        addActionIntoTreeViewOnly(action);
    }

    addSeparatorIntoTreeView();

    // Move up
    {
        const QString actionName = "Move up";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcut(Qt::ControlModifier + Qt::Key_Up);
        action->setShortcutContext(Qt::WidgetShortcut);

        packageData->moveUpAction = action;

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionHashPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [this](const Any& fieldValue) -> Any
                                         {
                                             const SelectedNodes& selectedNodes = GetSelection();
                                             return IsMoveUpActionEnabled(selectedNodes);
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnMoveUp, this));

        makeBindable("Package/Move", action, MODIFIABLE);
        addActionIntoTreeViewOnly(action);
    }

    // Move down
    {
        const QString actionName = "Move down";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcut(Qt::ControlModifier + Qt::Key_Down);
        action->setShortcutContext(Qt::WidgetShortcut);

        packageData->moveDownAction = action;

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionHashPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [this](const Any& fieldValue) -> Any
                                         {
                                             const SelectedNodes& selectedNodes = GetSelection();
                                             return IsMoveDownActionEnabled(selectedNodes);
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnMoveDown, this));

        makeBindable("Package/Move", action, MODIFIABLE);
        addActionIntoTreeViewOnly(action);
    }

    // Move left
    {
        const QString actionName = "Move left";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcut(Qt::ControlModifier + Qt::Key_Left);
        action->setShortcutContext(Qt::WidgetShortcut);

        packageData->moveLeftAction = action;

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionHashPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [this](const Any& fieldValue) -> Any
                                         {
                                             const SelectedNodes& selectedNodes = GetSelection();
                                             return IsMoveLeftActionEnabled(selectedNodes);
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnMoveLeft, this));

        makeBindable("Package/Move", action, MODIFIABLE);
        addActionIntoTreeViewOnly(action);
    }

    // Move right
    {
        const QString actionName = "Move right";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcut(Qt::ControlModifier + Qt::Key_Right);
        action->setShortcutContext(Qt::WidgetShortcut);

        packageData->moveRightAction = action;

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionHashPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [this](const Any& fieldValue) -> Any
                                         {
                                             const SelectedNodes& selectedNodes = GetSelection();
                                             return IsMoveRightActionEnabled(selectedNodes);
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnMoveRight, this));

        makeBindable("Package/Move", action, MODIFIABLE);
        addActionIntoTreeViewOnly(action);
    }

    addSeparatorIntoTreeView();

    // Run UIViewer Fast
    {
        const QString actionName = "Run UIViewer Fast";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcutContext(Qt::WidgetShortcut);

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionHashPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [this](const Any& fieldValue) -> Any
                                         {
                                             const SelectedNodes& selectedNodes = GetSelection();
                                             if (selectedNodes.size() != 1)
                                             {
                                                 return false;
                                             }

                                             PackageBaseNode* node = (*selectedNodes.begin());
                                             if (node->GetControl() != nullptr)
                                             {
                                                 PackageNode* package = node->GetPackage();
                                                 DVASSERT(package != nullptr);
                                                 return (package->IsImported() == false);
                                             }
                                             else
                                             {
                                                 return false;
                                             }
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnRunUIViewerFast, this));

        makeBindable("UIViewer", action, MODIFIABLE);
        addActionIntoTreeViewOnly(action);
    }

    // Run UIViewer
    {
        const QString actionName = "Run UIViewer";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcutContext(Qt::WidgetShortcut);

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionHashPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [this](const Any& fieldValue) -> Any
                                         {
                                             const SelectedNodes& selectedNodes = GetSelection();
                                             if (selectedNodes.size() != 1)
                                             {
                                                 return false;
                                             }

                                             PackageBaseNode* node = (*selectedNodes.begin());
                                             if (node->GetControl() != nullptr)
                                             {
                                                 PackageNode* package = node->GetPackage();
                                                 DVASSERT(package != nullptr);
                                                 return (package->IsImported() == false);
                                             }
                                             else
                                             {
                                                 return false;
                                             }
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnRunUIViewer, this));

        makeBindable("UIViewer", action, MODIFIABLE);
        addActionIntoTreeViewOnly(action);
    }

    addSeparatorIntoTreeView();

    // Jump to Prototype
    const QString jumpToPrototypeActionName = "Jump to Prototype";
    {
        QtAction* action = new QtAction(accessor, jumpToPrototypeActionName);
        action->setShortcut(Qt::ControlModifier + Qt::Key_J);
        action->setShortcutContext(Qt::WidgetShortcut);

        packageData->jumpToPrototypeAction = action;

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionHashPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [this](const Any& fieldValue) -> Any
                                         {
                                             const SelectedNodes& selectedNodes = GetSelection();
                                             return (selectedNodes.size() == 1);
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnJumpToPrototype, this));

        makeBindable("Package", action, MODIFIABLE);

        packageWidget->treeView->addAction(action);

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint("Find", { InsertionParams::eInsertionMethod::AfterItem, "Find file in project..." }));
        ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
    }

    // Find Prototype Instances
    {
        const QString actionName = "Find Prototype Instances";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcut(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_J);
        action->setShortcutContext(Qt::WidgetShortcut);

        packageData->findPrototypeInstancesAction = action;

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::selectionHashPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [this](const Any& fieldValue) -> Any
                                         {
                                             const SelectedNodes& selectedNodes = GetSelection();
                                             return (selectedNodes.size() == 1);
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnFindPrototypeInstances, this));

        makeBindable("Package", action, MODIFIABLE);

        packageWidget->treeView->addAction(action);

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint("Find", { InsertionParams::eInsertionMethod::AfterItem, jumpToPrototypeActionName }));
        ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
    }

    addSeparatorIntoTreeView();

    // Collapse all
    {
        const QString actionName = "Collapse all";
        QtAction* action = new QtAction(accessor, actionName);
        action->setShortcutContext(Qt::WidgetShortcut);

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any
                                         {
                                             return fieldValue.Cast<PackageNode*>(nullptr) != nullptr;
                                         });

        connections.AddConnection(action, &QAction::triggered, Bind(&PackageModule::OnCollapseAll, this));

        makeBindable("Package", action, MODIFIABLE);
        addActionIntoTreeViewOnly(action);
    }
}

void PackageModule::CreatePackageWidget()
{
    using namespace DAVA;

    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();

    packageData->packageWidget = new PackageWidget(GetAccessor(), GetUI());

    connections.AddConnection(packageData->packageWidget, &PackageWidget::SelectedNodesChanged, [this](const SelectedNodes& selection)
                              {
                                  documentDataWrapper.SetFieldValue(DocumentData::selectionPropertyName, selection);
                              });

    DockPanelInfo panelInfo;
    panelInfo.title = "Package";
    panelInfo.area = Qt::LeftDockWidgetArea;
    PanelKey panelKey("Package", panelInfo);

    GetUI()->AddView(DAVA::mainWindowKey, panelKey, packageData->packageWidget);
}

void PackageModule::RegisterGlobalOperation()
{
    RegisterOperation(QEGlobal::DropIntoPackageNode.ID, this, &PackageModule::OnDropIntoPackageNode);
}

void PackageModule::OnDataChanged(const DAVA::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    using namespace DAVA;

    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();
    PackageWidget* packageWidget = packageData->packageWidget;
    DAVA::Map<PackageNode*, PackageContext>& packageWidgetContexts = packageData->packageWidgetContexts;

    if (wrapper.HasData() == false)
    {
        packageWidget->OnSelectionChanged(Any());
        packageWidget->OnPackageChanged(nullptr, nullptr);
        return;
    }

    DocumentData* documentData = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
    PackageNode* package = documentData->GetPackageNode();
    PackageContext& context = packageWidgetContexts[package];

    Any selectionValue = wrapper.GetFieldValue(DocumentData::selectionPropertyName);

    if (fields.empty() == true)
    {
        packageWidget->OnSelectionChanged(Any());
        packageWidget->OnPackageChanged(&context, package);
        packageWidget->OnSelectionChanged(selectionValue);
    }
    else
    {
        //event-based code require selectionChange first, packageChange second and than another selecitonChanged
        bool selectionWasChanged = std::find(fields.begin(), fields.end(), DocumentData::selectionPropertyName) != fields.end();
        bool packageWasChanged = std::find(fields.begin(), fields.end(), DocumentData::packagePropertyName) != fields.end();

        if (selectionWasChanged == false && packageWasChanged == false)
        {
            return;
        }

        packageWidget->OnSelectionChanged(Any());

        if (packageWasChanged)
        {
            packageWidget->OnPackageChanged(&context, package);

            for (DAVA::Map<PackageNode*, PackageContext>::iterator iter = packageWidgetContexts.begin(); iter != packageWidgetContexts.end();)
            {
                bool packageExists = false;
                GetAccessor()->ForEachContext([&packageExists, iter](const DataContext& context) {
                    if (context.GetData<DocumentData>()->GetPackageNode() == iter->first)
                    {
                        DVASSERT(packageExists == false);
                        packageExists = true;
                    }
                });
                if (packageExists == false)
                {
                    iter = packageWidgetContexts.erase(iter);
                }
                else
                {
                    ++iter;
                }
            }
        }
        if (selectionWasChanged)
        {
            packageWidget->OnSelectionChanged(selectionValue);
        }
    }
}

void PackageModule::OnContextDeleted(DAVA::DataContext* context)
{
    DocumentData* documentData = context->GetData<DocumentData>();
    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();

    packageData->packageWidgetContexts.erase(documentData->GetPackageNode());
}

QAction* PackageModule::GetImportPackageAction()
{
    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();
    DVASSERT(packageData != nullptr);
    DVASSERT(packageData->importPackageAction != nullptr);
    return packageData->importPackageAction;
}

QAction* PackageModule::GetCutAction()
{
    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();
    DVASSERT(packageData != nullptr);
    DVASSERT(packageData->cutAction != nullptr);
    return packageData->cutAction;
}

QAction* PackageModule::GetCopyAction()
{
    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();
    DVASSERT(packageData != nullptr);
    DVASSERT(packageData->copyAction != nullptr);
    return packageData->copyAction;
}

QAction* PackageModule::GetPasteAction()
{
    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();
    DVASSERT(packageData != nullptr);
    DVASSERT(packageData->pasteAction != nullptr);
    return packageData->pasteAction;
}

QAction* PackageModule::GetDuplicateAction()
{
    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();
    DVASSERT(packageData != nullptr);
    DVASSERT(packageData->duplicateAction != nullptr);
    return packageData->duplicateAction;
}

QAction* PackageModule::GetDeleteAction()
{
    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();
    DVASSERT(packageData != nullptr);
    DVASSERT(packageData->deleteAction != nullptr);
    return packageData->deleteAction;
}

QAction* PackageModule::GetJumpToPrototypeAction()
{
    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();
    DVASSERT(packageData != nullptr);
    DVASSERT(packageData->jumpToPrototypeAction != nullptr);
    return packageData->jumpToPrototypeAction;
}

QAction* PackageModule::GetFindPrototypeInstancesAction()
{
    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();
    DVASSERT(packageData != nullptr);
    DVASSERT(packageData->findPrototypeInstancesAction != nullptr);
    return packageData->findPrototypeInstancesAction;
}

void PackageModule::OnImport()
{
    using namespace DAVA;

    DocumentData* documentData = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
    QStringList fileNames = FileDialog::getOpenFileNames(qApp->activeWindow(),
                                                         QObject::tr("Select one or move files to import"),
                                                         QString::fromStdString(documentData->GetPackagePath().GetDirectory().GetStringValue()),
                                                         "Packages (*.yaml)");
    if (fileNames.isEmpty() == false)
    {
        Vector<FilePath> packages;
        packages.reserve(fileNames.size());
        foreach (const QString& fileName, fileNames)
        {
            packages.push_back(FilePath(fileName.toStdString()));
        }
        DVASSERT(packages.empty() == false);
        CommandExecutor commandExecutor(GetAccessor(), GetUI());
        commandExecutor.AddImportedPackagesIntoPackage(packages, documentData->GetPackageNode());
    }
}

void PackageModule::OnAddStyle()
{
    using namespace DAVA;

    Vector<UIStyleSheetSelectorChain> selectorChains;
    selectorChains.push_back(UIStyleSheetSelectorChain("?"));
    const Vector<UIStyleSheetProperty> properties;

    DocumentData* documentData = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
    ScopedPtr<StyleSheetNode> style(new StyleSheetNode(UIStyleSheetSourceInfo(documentData->GetPackagePath()), selectorChains, properties));
    PackageNode* package = documentData->GetPackageNode();
    StyleSheetsNode* styleSheets = package->GetStyleSheets();

    CommandExecutor commandExecutor(GetAccessor(), GetUI());
    commandExecutor.InsertStyle(style, styleSheets, styleSheets->GetCount());
}

void PackageModule::OnCut()
{
    using namespace PackageModuleDetails;

    DocumentData* documentData = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
    const SelectedNodes& selectedNodes = documentData->GetSelectedNodes();

    Vector<ControlNode*> controls;
    CollectSelectedControls(selectedNodes, controls, true, true);

    Vector<StyleSheetNode*> styles;
    CollectSelectedStyles(selectedNodes, styles, true, true);

    PackageNode* package = documentData->GetPackageNode();
    CopyNodesToClipboard(package, controls, styles);

    CommandExecutor executor(GetAccessor(), GetUI());
    executor.Remove(controls, styles);
}

void PackageModule::OnCopy()
{
    using namespace PackageModuleDetails;

    DocumentData* documentData = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
    const SelectedNodes& selectedNodes = documentData->GetSelectedNodes();

    Vector<ControlNode*> controls;
    CollectSelectedControls(selectedNodes, controls, true, false);

    Vector<StyleSheetNode*> styles;
    CollectSelectedStyles(selectedNodes, styles, true, false);

    PackageNode* package = documentData->GetPackageNode();
    CopyNodesToClipboard(package, controls, styles);
}

void PackageModule::OnPaste()
{
    DAVA::DataContext* activeContext = GetAccessor()->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* documentData = activeContext->GetData<DocumentData>();
    DVASSERT(nullptr != documentData);
    PackageBaseNode* baseNode = *(documentData->GetSelectedNodes().begin());

    QClipboard* clipboard = QApplication::clipboard();

    if (baseNode->IsReadOnly() == false && clipboard != nullptr && clipboard->mimeData())
    {
        DAVA::String string = clipboard->mimeData()->text().toStdString();
        PackageNode* package = documentData->GetPackageNode();
        CommandExecutor executor(GetAccessor(), GetUI());
        PackageBaseNode* parent = baseNode->GetParent();
        SelectedNodes selection;
        if (baseNode->GetParent() == package)
        {
            selection = executor.Paste(package, baseNode, baseNode->GetCount(), string);
        }
        else
        {
            DAVA::int32 index = static_cast<DAVA::int32>(parent->GetIndex(baseNode));
            selection = executor.Paste(package, parent, index + 1, string);
        }
        if (selection.empty() == false)
        {
            SetNewSelection(selection);
        }
    }
}

void PackageModule::OnDuplicate()
{
    using namespace DAVA;

    DocumentData* documentData = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
    SelectedNodes nodes = documentData->GetSelectedNodes();

    QApplication::clipboard()->clear();

    OnCopy();

    QClipboard* clipboard = QApplication::clipboard();

    if (clipboard != nullptr && clipboard->mimeData())
    {
        Vector<PackageBaseNode*> sortedSelection(nodes.begin(), nodes.end());
        std::sort(sortedSelection.begin(), sortedSelection.end(), CompareByLCA);
        PackageBaseNode* parent = sortedSelection.front()->GetParent();
        if (parent->IsReadOnly() == false)
        {
            String string = clipboard->mimeData()->text().toStdString();

            PackageNode* package = documentData->GetPackageNode();
            CommandExecutor executor(GetAccessor(), GetUI());

            PackageBaseNode* lastSelected = sortedSelection.back();
            int index = parent->GetIndex(lastSelected);
            SelectedNodes selection = executor.Paste(package, parent, index + 1, string);
            if (selection.empty() == false)
            {
                SetNewSelection(selection);
            }
        }
    }
}

void PackageModule::OnCopyControlPath()
{
    using namespace DAVA;

    using namespace PackageModuleDetails;

    DocumentData* documentData = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
    const SelectedNodes& selection = documentData->GetSelectedNodes();

    Vector<ControlNode*> controlNodes;
    CollectSelectedControls(selection, controlNodes, false, false);

    QClipboard* clipboard = QApplication::clipboard();
    QMimeData* data = new QMimeData();

    QString str;
    for (ControlNode* controlNode : controlNodes)
    {
        PackageControlsNode* controlsRoot = controlNode->GetPackage()->GetPackageControlsNode();
        PackageControlsNode* prototypesRoot = controlNode->GetPackage()->GetPrototypes();
        QString path;
        for (PackageBaseNode* node = controlNode; node != controlsRoot && node != prototypesRoot && node != nullptr; node = node->GetParent())
        {
            if (!path.isEmpty())
            {
                path.prepend("/");
            }
            path.prepend(QString::fromStdString(node->GetName()));
        }
        str += path;
        str += "\n";
    }

    data->setText(str);
    clipboard->setMimeData(data);
}

void PackageModule::OnRename()
{
    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();
    PackageWidget* packageWidget = packageData->packageWidget;
    packageWidget->OnRename();
}

void PackageModule::OnDelete()
{
    using namespace DAVA;

    using namespace PackageModuleDetails;

    DocumentData* documentData = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
    const SelectedNodes& selection = documentData->GetSelectedNodes();

    Vector<ControlNode*> controls;
    CollectSelectedControls(selection, controls, false, true);

    Vector<StyleSheetNode*> styles;
    CollectSelectedStyles(selection, styles, false, true);

    CommandExecutor executor(GetAccessor(), GetUI());

    if (!controls.empty() || !styles.empty())
    {
        executor.Remove(controls, styles);
    }
    else
    {
        Vector<PackageNode*> packages;
        CollectSelectedImportedPackages(selection, packages, false, true);

        PackageNode* package = documentData->GetPackageNode();
        executor.RemoveImportedPackagesFromPackage(packages, package);
    }
}

void PackageModule::OnMoveUp()
{
    using namespace PackageModuleDetails;

    DocumentData* documentData = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
    const SelectedNodes& selection = documentData->GetSelectedNodes();
    PackageBaseNode* node = *(selection.begin());

    MoveNode(node, UP);
}

void PackageModule::OnMoveDown()
{
    using namespace PackageModuleDetails;

    DocumentData* documentData = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
    const SelectedNodes& selection = documentData->GetSelectedNodes();
    PackageBaseNode* node = *(selection.begin());

    MoveNode(node, DOWN);
}

void PackageModule::OnMoveLeft()
{
    using namespace PackageModuleDetails;

    DocumentData* documentData = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
    const SelectedNodes& selection = documentData->GetSelectedNodes();
    PackageBaseNode* node = *(selection.begin());

    PackageBaseNode* parentNode = node->GetParent();
    DVASSERT(parentNode != nullptr);
    PackageBaseNode* grandParentNode = parentNode->GetParent();
    DVASSERT(grandParentNode != nullptr);
    int destIndex = grandParentNode->GetIndex(parentNode) + 1;

    MoveNode(node, grandParentNode, destIndex);
}

void PackageModule::OnMoveRight()
{
    using namespace PackageModuleDetails;

    DocumentData* documentData = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
    const SelectedNodes& selection = documentData->GetSelectedNodes();
    PackageBaseNode* node = *(selection.begin());

    PackageIterator iterUp(node, [this, node](const PackageBaseNode* dest) {
        return CanMove(dest, node, UP);
    });
    --iterUp;

    DVASSERT(iterUp.IsValid());

    PackageBaseNode* dest = *iterUp;

    MoveNode(node, dest, dest->GetCount());
}

void PackageModule::OnRunUIViewerFast()
{
    using namespace DAVA;

    using namespace PackageModuleDetails;

    DataContext* globalContext = GetAccessor()->GetGlobalContext();
    ProjectData* projectData = globalContext->GetData<ProjectData>();
    PackageWidgetSettings* settings = globalContext->GetData<PackageWidgetSettings>();
    DocumentData* documentData = GetAccessor()->GetActiveContext()->GetData<DocumentData>();

    const Vector<ProjectData::Device>& devices = projectData->GetDevices();
    const Vector<ProjectData::Blank>& blanks = projectData->GetBlanks();
    if (settings->selectedDevice < devices.size() && settings->selectedBlank < blanks.size())
    {
        const ProjectData::Blank& blank = blanks[settings->selectedBlank];
        const ProjectData::Device& device = devices[settings->selectedDevice];

        Window* primaryWindow = GetPrimaryWindow();
        DVASSERT(primaryWindow != nullptr);

        Size2i screenSize(static_cast<int32>(primaryWindow->GetSize().dx), static_cast<int32>(primaryWindow->GetSize().dx));
        if (device.params.count(FastName("screenSize")) > 0)
        {
            screenSize = device.params.at(FastName("screenSize")).Get<Size2i>();
        }

        Size2i virtualSize = screenSize;
        if (device.params.count(FastName("virtualScreenSize")) > 0)
        {
            virtualSize = device.params.at(FastName("virtualScreenSize")).Get<Size2i>();
        }

        int32 virtualKeyboardHeight = 0;
        if (device.params.count(FastName("virtualKeyboardHeight")) > 0)
        {
            virtualKeyboardHeight = device.params.at(FastName("virtualKeyboardHeight")).Get<int32>();
        }

        const EngineContext* engineContext = GetEngineContext();
        String lang = engineContext->localizationSystem->GetCurrentLocale();
        if (device.params.count(FastName("lang")) > 0)
        {
            lang = device.params.at(FastName("lang")).Get<String>();
        }

        bool isRtl = engineContext->uiControlSystem->IsRtl();
        if (device.params.count(FastName("isRtl")) > 0)
        {
            isRtl = device.params.at(FastName("isRtl")).Get<bool>();
        }

        FileSystem* fs = GetAccessor()->GetEngineContext()->fileSystem;
        FilePath appPath;
        if (settings->useCustomUIViewerPath)
        {
            appPath = settings->customUIViewerPath;
        }
        else
        {
#if defined(__DAVAENGINE_MACOS__)
            appPath = fs->GetCurrentExecutableDirectory() + "/../../../UIViewer.app";
#else //win
            appPath = fs->GetCurrentExecutableDirectory() + "/UIViewer.exe";
#endif //
        }

        String fontsPath = "~res:/" + projectData->GetFontsConfigsDirectory().relative;

        Vector<ControlNode*> controlNodes;
        CollectSelectedControls(documentData->GetSelectedNodes(), controlNodes, false, false);
        for (ControlNode* node : controlNodes)
        {
            ControlNode* rootNode = GetRootControlNode(node);

            QStringList args;
            args << "options";

            args << "-project";
            args << projectData->GetProjectDirectory().GetStringValue().c_str();

            args << "-blankYaml";
            args << blank.path.GetFrameworkPath().c_str();
            args << "-blankRoot";
            args << blank.controlName.c_str();

            if (blank.controlPath != FastName(""))
            {
                args << "-blankPath";
                args << blank.controlPath.c_str();
            }

            args << "-testedYaml";
            args << documentData->GetPackagePath().GetFrameworkPath().c_str();
            args << "-testedCtrl";
            args << rootNode->GetName().c_str();

            String testedPath = BuildControlPath(node, rootNode);
            if (!testedPath.empty())
            {
                args << "-testedPath";
                args << testedPath.c_str();
            }

            args << "-screenWidth";
            args << Format("%d", screenSize.dx).c_str();
            args << "-screenHeight";
            args << Format("%d", screenSize.dy).c_str();

            args << "-virtualWidth";
            args << Format("%d", virtualSize.dx).c_str();
            args << "-virtualHeight";
            args << Format("%d", virtualSize.dy).c_str();

            args << "-virtualKeyboardHeight";
            args << Format("%d", virtualKeyboardHeight).c_str();

            args << "-fontsDir";
            args << fontsPath.c_str();

            args << "-locale";
            args << lang.c_str();

            if (isRtl)
            {
                args << "-isRtl";
            }

            if (settings->flowFlag)
            {
                args << "-isFlow";
            }

            Logger::Info("Launch UIViewer with arguments: %s", args.join(" ").toStdString().c_str());

            bool state = ProcessHelper::OpenApplication(QString::fromStdString(appPath.GetAbsolutePathname()), args);
            if (state == false)
            {
                PushErrorMessage(Format("Failed to run %s", appPath.GetStringValue().c_str()));
            }
        }
    }
    else
    {
        PushErrorMessage(Format("Could not select correct device or blank (device: %d from %d, blank: %d from %d)", settings->selectedDevice, static_cast<int32>(devices.size()), settings->selectedBlank, static_cast<int32>(blanks.size())).c_str());
    }
}

void PackageModule::OnRunUIViewer()
{
    UIViewerDialog dlg(GetAccessor(), GetUI(), GetUI()->GetWindow(DAVA::mainWindowKey));
    PackageWidgetSettings* settings = GetAccessor()->GetGlobalContext()->GetData<PackageWidgetSettings>();
    dlg.SetDeviceIndex(static_cast<DAVA::int32>(settings->selectedDevice));
    dlg.SetBlankIndex(static_cast<DAVA::int32>(settings->selectedBlank));
    dlg.SetFlowFlag(settings->flowFlag);
    if (dlg.exec() == QDialog::Accepted)
    {
        settings->selectedDevice = dlg.GetDeviceIndex();
        settings->selectedBlank = dlg.GetBlankIndex();
        settings->flowFlag = dlg.GetFlowFlag();
        OnRunUIViewerFast();
    }
}

void PackageModule::OnJumpToPrototype()
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetActiveContext();
    const DocumentData* documentData = activeContext->GetData<DocumentData>();
    const SelectedNodes& nodes = documentData->GetSelectedNodes();
    PackageBaseNode* node = *(nodes.begin());

    ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
    if (controlNode != nullptr && controlNode->GetPrototype() != nullptr)
    {
        ControlNode* prototypeNode = controlNode->GetPrototype();
        FilePath path = prototypeNode->GetPackage()->GetPath();
        String name = prototypeNode->GetName();
        JumpToControl(path, name);
    }
}

void PackageModule::OnFindPrototypeInstances()
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetActiveContext();
    const DocumentData* documentData = activeContext->GetData<DocumentData>();
    const SelectedNodes& nodes = documentData->GetSelectedNodes();
    PackageBaseNode* node = *(nodes.begin());

    ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
    if (controlNode != nullptr)
    {
        FilePath path = controlNode->GetPackage()->GetPath();
        String name = controlNode->GetName();

        std::shared_ptr<FindFilter> filter = std::make_shared<PrototypeUsagesFilter>(path.GetFrameworkPath(), FastName(name));
        InvokeOperation(QEGlobal::FindInProject.ID, filter);
    }
}

void PackageModule::OnDropIntoPackageNode(const QMimeData* data, Qt::DropAction action, PackageBaseNode* destNode, DAVA::uint32 destIndex, const DAVA::Vector2* pos)
{
    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();
    PackageWidget* packageWidget = packageData->packageWidget;
    packageWidget->GetPackageModel()->OnDropMimeData(data, action, destNode, destIndex, pos);
}

void PackageModule::RefreshMoveActions(const SelectedNodes& selection)
{
    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();
    packageData->moveUpAction->setEnabled(IsMoveUpActionEnabled(selection));
    packageData->moveDownAction->setEnabled(IsMoveDownActionEnabled(selection));
    packageData->moveLeftAction->setEnabled(IsMoveLeftActionEnabled(selection));
    packageData->moveRightAction->setEnabled(IsMoveRightActionEnabled(selection));
}

bool PackageModule::IsMoveUpActionEnabled(const SelectedNodes& selection)
{
    if (selection.size() != 1)
    {
        return false;
    }

    PackageBaseNode* node = (*selection.begin());

    if (node->CanRemove())
    {
        PackageIterator iterUp(node, [node, this](const PackageBaseNode* dest) -> bool {
            return CanMove(dest, node, UP);
        });
        --iterUp;
        return iterUp.IsValid();
    }
    else
    {
        return false;
    }
}

bool PackageModule::IsMoveDownActionEnabled(const SelectedNodes& selectedNodes)
{
    if (selectedNodes.size() != 1)
    {
        return false;
    }

    PackageBaseNode* node = (*selectedNodes.begin());

    if (node->CanRemove())
    {
        PackageIterator iterDown(node, [node, this](const PackageBaseNode* dest) -> bool {
            return CanMove(dest, node, DOWN);
        });
        ++iterDown;
        return iterDown.IsValid();
    }
    else
    {
        return false;
    }
}

bool PackageModule::IsMoveLeftActionEnabled(const SelectedNodes& selectedNodes)
{
    if (selectedNodes.size() != 1)
    {
        return false;
    }
    PackageBaseNode* node = (*selectedNodes.begin());
    return (node->CanRemove() && CanMoveLeft(node));
}

bool PackageModule::IsMoveRightActionEnabled(const SelectedNodes& selectedNodes)
{
    if (selectedNodes.size() != 1)
    {
        return false;
    }
    PackageBaseNode* node = (*selectedNodes.begin());
    return (node->CanRemove() && CanMoveRight(node));
}

void PackageModule::OnCollapseAll()
{
    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();
    PackageWidget* packageWidget = packageData->packageWidget;
    packageWidget->ExpandToFirstChild();
}

bool PackageModule::CanMove(const PackageBaseNode* dest, PackageBaseNode* node, eDirection direction) const
{
    DVASSERT(nullptr != node);
    DVASSERT(nullptr != dest);

    if (dest->GetParent() != node->GetParent())
    {
        return false;
    }

    const PackageBaseNode* destParent = dest->GetParent();
    DVASSERT(nullptr != destParent);

    int destIndex = destParent->GetIndex(dest);
    if (direction == DOWN)
    {
        destIndex += 1;
    }

    return PackageModuleDetails::CanInsertControlOrStyle(destParent, node, destIndex);
}

bool PackageModule::CanMoveLeft(PackageBaseNode* node) const
{
    PackageBaseNode* parentNode = node->GetParent();
    DVASSERT(parentNode != nullptr);
    PackageBaseNode* grandParentNode = parentNode->GetParent();
    if (grandParentNode != nullptr)
    {
        int destIndex = grandParentNode->GetIndex(parentNode) + 1;
        return PackageModuleDetails::CanInsertControlOrStyle(grandParentNode, node, destIndex);
    }
    else
    {
        return false;
    }
}

bool PackageModule::CanMoveRight(PackageBaseNode* node) const
{
    PackageIterator iterUp(node, [this, node](const PackageBaseNode* dest)
                           {
                               return CanMove(dest, node, UP);
                           });
    --iterUp;
    if (!iterUp.IsValid())
    {
        return false;
    }
    PackageBaseNode* dest = *iterUp;
    int destIndex = dest->GetCount();
    return PackageModuleDetails::CanInsertControlOrStyle(dest, node, destIndex);
}

void PackageModule::MoveNode(PackageBaseNode* node, eDirection direction)
{
    PackageIterator iter(node, [direction, node, this](const PackageBaseNode* dest) -> bool
                         {
                             return CanMove(dest, node, UP);
                         });
    PackageBaseNode* nextNode = direction == UP ? *(--iter) : *(++iter);
    DVASSERT(nullptr != nextNode);
    DVASSERT((dynamic_cast<ControlNode*>(node) != nullptr && dynamic_cast<const ControlNode*>(nextNode) != nullptr) || (dynamic_cast<StyleSheetNode*>(node) != nullptr && dynamic_cast<const StyleSheetNode*>(nextNode) != nullptr));
    PackageBaseNode* nextNodeParent = nextNode->GetParent();
    DVASSERT(nextNodeParent != nullptr);
    int destIndex = nextNodeParent->GetIndex(nextNode);
    if (direction == DOWN)
    {
        ++destIndex;
    }
    MoveNode(node, nextNodeParent, destIndex);
}

void PackageModule::MoveNode(PackageBaseNode* node, PackageBaseNode* dest, DAVA::uint32 destIndex)
{
    using namespace DAVA;

    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();
    PackageWidget* packageWidget = packageData->packageWidget;

    SelectedNodes selection = { node };

    CommandExecutor executor(GetAccessor(), GetUI());

    if (dynamic_cast<ControlNode*>(node) != nullptr)
    {
        Vector<ControlNode*> nodes = { static_cast<ControlNode*>(node) };
        ControlsContainerNode* nextControlNode = dynamic_cast<ControlsContainerNode*>(dest);
        packageWidget->OnBeforeProcessNodes(selection);
        executor.MoveControls(nodes, nextControlNode, destIndex);
        packageWidget->OnAfterProcessNodes(selection);
        RefreshMoveActions(selection);
    }
    else if (dynamic_cast<StyleSheetNode*>(node) != nullptr)
    {
        Vector<StyleSheetNode*> nodes = { static_cast<StyleSheetNode*>(node) };
        StyleSheetsNode* nextStyleSheetNode = dynamic_cast<StyleSheetsNode*>(dest);
        packageWidget->OnBeforeProcessNodes(selection);
        executor.MoveStyles(nodes, nextStyleSheetNode, destIndex);
        packageWidget->OnAfterProcessNodes(selection);
        RefreshMoveActions(selection);
    }
    else
    {
        DVASSERT(false && "invalid type of moved node");
    }
}

void PackageModule::PushErrorMessage(const DAVA::String& errorMessage)
{
    DAVA::Logger::Error(errorMessage.c_str());

    DAVA::NotificationParams notifParams;
    notifParams.title = "Error";
    notifParams.message.message = errorMessage;
    notifParams.message.type = DAVA::Result::RESULT_ERROR;
    GetUI()->ShowNotification(DAVA::mainWindowKey, notifParams);
}

void PackageModule::JumpToControl(const DAVA::FilePath& packagePath, const DAVA::String& controlName)
{
    using namespace DAVA;

    QString path = QString::fromStdString(packagePath.GetAbsolutePathname());
    QString name = QString::fromStdString(controlName);
    InvokeOperation(QEGlobal::SelectControl.ID, path, name);
}

void PackageModule::JumpToPackage(const DAVA::FilePath& packagePath)
{
    QString path = QString::fromStdString(packagePath.GetAbsolutePathname());
    InvokeOperation(QEGlobal::OpenDocumentByPath.ID, path);
}

void PackageModule::SetNewSelection(const SelectedNodes& selection)
{
    documentDataWrapper.SetFieldValue(DocumentData::selectionPropertyName, selection);
    PackageData* packageData = GetAccessor()->GetGlobalContext()->GetData<PackageData>();
    packageData->packageWidget->OnSelectionChanged(selection);
}

SelectedNodes PackageModule::GetSelection() const
{
    const DAVA::DataContext* activeContext = GetAccessor()->GetActiveContext();
    if (activeContext != nullptr)
    {
        DocumentData* documentData = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
        return documentData->GetSelectedNodes();
    }
    else
    {
        return SelectedNodes();
    }
}

DECL_TARC_MODULE(PackageModule);
