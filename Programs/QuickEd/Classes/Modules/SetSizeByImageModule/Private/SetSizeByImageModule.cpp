#include "Modules/SetSizeByImageModule/SetSizeByImageModule.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Model/ControlProperties/RootProperty.h"
#include "QECommands/ChangePropertyValueCommand.h"

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/ActionUtils.h>

#include <Reflection/ReflectedTypeDB.h>
#include <Render/2D/Sprite.h>
#include <UI/UIControl.h>
#include <UI/UIControlBackground.h>

DAVA_VIRTUAL_REFLECTION_IMPL(SetSizeByImageModule)
{
    DAVA::ReflectionRegistrator<SetSizeByImageModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void SetSizeByImageModule::PostInit()
{
    using namespace DAVA;

    const QString actionName("Set size from image");

    QtAction* action = new QtAction(GetAccessor(), actionName, nullptr);
    action->setShortcutContext(Qt::WindowShortcut);
    action->setShortcut(QKeySequence("Ctrl+I"));

    FieldDescriptor fieldDescr;
    fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
    fieldDescr.fieldName = FastName(DocumentData::selectionPropertyName);
    action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [&](const Any& fieldValue) -> Any
                                     {
                                         return (fieldValue.Cast<SelectedNodes>(SelectedNodes()).size() == 1);
                                     });

    connections.AddConnection(action, &QAction::triggered, MakeFunction(this, &SetSizeByImageModule::OnSetSizeFromImage));

    ActionPlacementInfo placementInfo;
    placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuEdit, { InsertionParams::eInsertionMethod::AfterItem }));

    GetUI()->AddAction(DAVA::mainWindowKey, placementInfo, action);
}

void SetSizeByImageModule::OnSetSizeFromImage()
{
    using namespace DAVA;

    DocumentData* documentData = GetAccessor()->GetActiveContext()->GetData<DocumentData>();
    const SelectedNodes& nodes = documentData->GetSelectedNodes();
    ControlNode* controlNode = dynamic_cast<ControlNode*>(*(nodes.begin()));
    if (controlNode != nullptr)
    {
        UIControlBackground* bg = controlNode->GetControl()->GetComponent<UIControlBackground>();
        if (bg != nullptr && bg->GetSprite() != nullptr)
        {
            Vector2 spriteSize = bg->GetSprite()->GetSize();
            std::unique_ptr<ChangePropertyValueCommand> command = documentData->CreateCommand<ChangePropertyValueCommand>();
            AbstractProperty* sizeProperty = controlNode->GetRootProperty()->FindPropertyByName("size");
            command->AddNodePropertyValue(controlNode, sizeProperty, Any(spriteSize));
            documentData->ExecCommand(std::move(command));
        }
    }
}

DECL_TARC_MODULE(SetSizeByImageModule);
