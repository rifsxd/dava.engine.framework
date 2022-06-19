#include "Modules/GroupingControlsModule/GroupingControlsModule.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/ValueProperty.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "QECommands/ChangePropertyValueCommand.h"
#include "UI/CommandExecutor.h"

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/UI.h>

#include <Reflection/ReflectedTypeDB.h>
#include <UI/UIControl.h>

namespace GroupingControlsModuleDetails
{
using namespace DAVA;

Rect GetConstraintBox(const Vector<ControlNode*> nodes)
{
    UIControl* firstControl = nodes.front()->GetControl();
    Rect constraintBox(firstControl->GetRect());

    std::for_each(std::next(nodes.begin()), nodes.end(), [&constraintBox](ControlNode* node)
                  {
                      constraintBox = constraintBox.Combine(node->GetControl()->GetRect());
                  });
    return constraintBox;
}

void ShiftPositions(const Vector<ControlNode*>& nodes, const Vector2& offsetPoint, DocumentData* data)
{
    for (ControlNode* node : nodes)
    {
        Vector2 newPositionValue = node->GetControl()->GetPosition() - offsetPoint;

        std::unique_ptr<ChangePropertyValueCommand> command = data->CreateCommand<ChangePropertyValueCommand>();
        RootProperty* rootProperty = node->GetRootProperty();
        AbstractProperty* positionProperty = rootProperty->FindPropertyByName("position");
        command->AddNodePropertyValue(node, positionProperty, newPositionValue);
        data->ExecCommand(std::move(command));
    }
}
}

DAVA_VIRTUAL_REFLECTION_IMPL(GroupingControlsModule)
{
    DAVA::ReflectionRegistrator<GroupingControlsModule>::Begin()
    .ConstructorByPointer()
    .End();
}

GroupingControlsModule::GroupingControlsModule()
{
    DAVA::RefPtr<DAVA::UIControl> sampleControl(new DAVA::UIControl);
    sampleGroupNode.Set(ControlNode::CreateFromControl(sampleControl.Get()));
}

void GroupingControlsModule::PostInit()
{
    using namespace DAVA;

    documentDataWrapper = GetAccessor()->CreateWrapper(DAVA::ReflectedTypeDB::Get<DocumentData>());

    FieldDescriptor selectionField;
    selectionField.type = ReflectedTypeDB::Get<DocumentData>();
    selectionField.fieldName = FastName(DocumentData::selectionPropertyName);

    // Group
    const QString groupActionName("Group");
    {
        QtAction* action = new QtAction(GetAccessor(), groupActionName, nullptr);
        action->setShortcutContext(Qt::WindowShortcut);
        action->setShortcut(QKeySequence("Ctrl+G"));
        action->SetStateUpdationFunction(QtAction::Enabled, selectionField, [&](const Any& fieldValue) -> Any
                                         {
                                             return (fieldValue.Cast<SelectedNodes>(SelectedNodes()).empty() == false);
                                         });

        connections.AddConnection(action, &QAction::triggered, MakeFunction(this, &GroupingControlsModule::DoGroup));

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuEdit, { InsertionParams::eInsertionMethod::AfterItem }));
        GetUI()->AddAction(DAVA::mainWindowKey, placementInfo, action);
    }

    // Ungroup
    const QString ungroupActionName("Ungroup");
    {
        QtAction* action = new QtAction(GetAccessor(), ungroupActionName, nullptr);
        action->setShortcutContext(Qt::WindowShortcut);
        action->setShortcut(QKeySequence("Ctrl+Shift+G"));
        action->SetStateUpdationFunction(QtAction::Enabled, selectionField, [&](const Any& fieldValue) -> Any
                                         {
                                             return (fieldValue.Cast<SelectedNodes>(SelectedNodes()).size() == 1);
                                         });

        connections.AddConnection(action, &QAction::triggered, MakeFunction(this, &GroupingControlsModule::DoUngroup));

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuEdit, { InsertionParams::eInsertionMethod::AfterItem, groupActionName }));
        GetUI()->AddAction(DAVA::mainWindowKey, placementInfo, action);
    }

    // Separator
    {
        QAction* separator = new QAction(nullptr);
        separator->setObjectName("group actions separator");
        separator->setSeparator(true);
        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuEdit, { InsertionParams::eInsertionMethod::AfterItem, ungroupActionName }));
        GetUI()->AddAction(DAVA::mainWindowKey, placementInfo, separator);
    }
}

void GroupingControlsModule::DoGroup()
{
    using namespace DAVA;
    using namespace GroupingControlsModuleDetails;

    DocumentData* data = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
    const SelectedNodes& selectedNodes = data->GetSelectedNodes();
    Vector<ControlNode*> selectedControlNodes;

    Result result = CanGroupSelection(selectedNodes);

    if (result.type != Result::RESULT_ERROR)
    {
        selectedControlNodes.reserve(selectedNodes.size());
        for (PackageBaseNode* node : selectedNodes)
        {
            ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
            if (controlNode)
            {
                selectedControlNodes.push_back(controlNode);
            }
        }

        std::sort(selectedControlNodes.begin(), selectedControlNodes.end(), CompareByLCA);

        if (data->GetSelectedNodes().size() != selectedControlNodes.size())
        {
            result = Result(Result::RESULT_ERROR, "only controls can be grouped");
        }
    }

    if (result.type == Result::RESULT_ERROR)
    {
        DAVA::NotificationParams params;
        params.title = "Can't group selected nodes";
        params.message = result;
        GetUI()->ShowNotification(DAVA::mainWindowKey, params);
        return;
    }

    data->BeginBatch("Group controls");

    Rect constraintBox = GetConstraintBox(selectedControlNodes);
    ShiftPositions(selectedControlNodes, constraintBox.GetPosition(), data);

    ScopedPtr<UIControl> control(new UIControl(constraintBox));
    control->SetName("Group");
    ControlNode* newGroupControl = ControlNode::CreateFromControl(control);

    CommandExecutor commandExecutor(GetAccessor(), GetUI());
    ControlNode* parent = dynamic_cast<ControlNode*>(selectedControlNodes.front()->GetParent());
    commandExecutor.InsertControl(newGroupControl, parent, parent->GetCount());
    commandExecutor.MoveControls(selectedControlNodes, newGroupControl, 0);

    AbstractProperty* positionProperty = newGroupControl->GetRootProperty()->FindPropertyByName("position");
    AbstractProperty* sizeProperty = newGroupControl->GetRootProperty()->FindPropertyByName("size");
    newGroupControl->GetRootProperty()->SetProperty(positionProperty, Any(newGroupControl->GetControl()->GetPosition()));
    newGroupControl->GetRootProperty()->SetPropertyForceOverride(static_cast<ValueProperty*>(positionProperty), true);
    newGroupControl->GetRootProperty()->SetProperty(sizeProperty, Any(newGroupControl->GetControl()->GetSize()));
    newGroupControl->GetRootProperty()->SetPropertyForceOverride(static_cast<ValueProperty*>(sizeProperty), true);

    data->EndBatch();

    if (newGroupControl != nullptr)
    {
        SelectedNodes newSelection = { newGroupControl };
        documentDataWrapper.SetFieldValue(DocumentData::selectionPropertyName, newSelection);
    }
}

void GroupingControlsModule::DoUngroup()
{
    using namespace DAVA;
    using namespace GroupingControlsModuleDetails;

    DocumentData* data = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
    const SelectedNodes& selectedNodes = data->GetSelectedNodes();
    DVASSERT(selectedNodes.size() == 1);

    Result result = CanUngroupSelection(selectedNodes);

    if (result.type == Result::RESULT_ERROR)
    {
        DAVA::NotificationParams params;
        params.title = "Can't ungroup selected node";
        params.message = result;
        GetUI()->ShowNotification(DAVA::mainWindowKey, params);
        return;
    }

    ControlNode* selectedGroupControl = dynamic_cast<ControlNode*>(*selectedNodes.begin());
    DVASSERT(selectedGroupControl != nullptr);

    data->BeginBatch("Ungroup control");

    Vector<ControlNode*> ungroupedNodes;
    ungroupedNodes.reserve(selectedGroupControl->GetCount());
    std::copy(selectedGroupControl->begin(), selectedGroupControl->end(), std::back_inserter(ungroupedNodes));

    Vector2 offset = selectedGroupControl->GetControl()->GetRect().GetPosition();
    offset.x = -offset.x;
    offset.y = -offset.y;
    ShiftPositions(ungroupedNodes, offset, data);

    CommandExecutor commandExecutor(GetAccessor(), GetUI());

    ControlNode* parent = dynamic_cast<ControlNode*>(selectedGroupControl->GetParent());
    commandExecutor.MoveControls(ungroupedNodes, parent, parent->GetCount());

    DAVA::Vector<ControlNode*> removedControls = { selectedGroupControl };
    DAVA::Vector<StyleSheetNode*> styles;
    commandExecutor.Remove(removedControls, styles);

    data->EndBatch();

    if (ungroupedNodes.empty() == false)
    {
        SelectedNodes nodesToSelect;
        std::copy(ungroupedNodes.begin(), ungroupedNodes.end(), std::inserter(nodesToSelect, nodesToSelect.begin()));
        documentDataWrapper.SetFieldValue(DocumentData::selectionPropertyName, nodesToSelect);
    }
}

DAVA::Result GroupingControlsModule::CanGroupSelection(const SelectedNodes& selectedNodes) const
{
    using namespace DAVA;

    if (selectedNodes.size() < 2)
    {
        return Result(Result::RESULT_ERROR, "2 or more nodes should be selected");
    }

    PackageBaseNode* commonParent = (*selectedNodes.begin())->GetParent();
    ControlNode* commonParentControl = dynamic_cast<ControlNode*>(commonParent);
    if (commonParentControl == nullptr)
    {
        return Result(Result::RESULT_ERROR, "only children of controls can be grouped");
    }

    bool allHaveCommonParent = std::all_of(std::next(selectedNodes.begin()), selectedNodes.end(), [commonParent](PackageBaseNode* node)
                                           {
                                               return node->GetParent() == commonParent;
                                           });
    if (!allHaveCommonParent)
    {
        return Result(Result::RESULT_ERROR, "all selected nodes should have same parent");
    }

    if (!commonParent->CanInsertControl(sampleGroupNode.Get(), commonParent->GetCount()))
    {
        return Result(Result::RESULT_ERROR, "not allowed to insert into parent control");
    }

    bool allCanBeMoved = std::all_of(selectedNodes.begin(), selectedNodes.end(), [](PackageBaseNode* node)
                                     {
                                         return node->CanRemove() == true;
                                     });
    if (!allCanBeMoved)
    {
        return Result(Result::RESULT_ERROR, "all selected nodes must be movable");
    }

    return Result(Result::RESULT_SUCCESS);
}

DAVA::Result GroupingControlsModule::CanUngroupSelection(const SelectedNodes& selectedNodes) const
{
    using namespace DAVA;

    ControlNode* groupNode = dynamic_cast<ControlNode*>(*(selectedNodes.begin()));

    if (groupNode == nullptr)
    {
        return Result(Result::RESULT_ERROR, "only controls can be ungrouped");
    }

    if (groupNode->CanRemove() == false)
    {
        return Result(Result::RESULT_ERROR, "selected group node is not removable");
    }

    if (groupNode->GetCount() == 0)
    {
        return Result(Result::RESULT_ERROR, "Ungrouped control must contain children");
    }

    ControlNode* parent = dynamic_cast<ControlNode*>(groupNode->GetParent());
    if (parent == nullptr)
    {
        return Result(Result::RESULT_ERROR, "selected group should be the child of a control");
    }

    bool allChildrenCanBeMoved = std::all_of(groupNode->begin(), groupNode->end(), [parent](ControlNode* child)
                                             {
                                                 return (child->CanMoveTo(parent, parent->GetCount()));
                                             });
    if (allChildrenCanBeMoved == false)
    {
        return Result(Result::RESULT_ERROR, "all children of selected group must be movable");
    }

    return Result(Result::RESULT_SUCCESS);
}

DECL_TARC_MODULE(GroupingControlsModule);
