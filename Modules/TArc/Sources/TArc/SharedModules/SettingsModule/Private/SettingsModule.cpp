#include "TArc/SharedModules/SettingsModule/SettingsModule.h"
#include "TArc/SharedModules/SettingsModule/Private/SettingsDialog.h"
#include "TArc/SharedModules/SettingsModule/Private/SettingsManager.h"
#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/WindowSubSystem/QtAction.h"
#include "TArc/Controls/ColorPicker/ColorPickerSettings.h"
#include "TArc/Qt/QtString.h"
#include "TArc/Qt/QtIcon.h"
#include "TArc/Utils/ReflectionHelpers.h"

#include <Reflection/Reflection.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Functional/Function.h>

#include <QList>

namespace DAVA
{
namespace SettingsModuleDetails
{
class SettingsContainerNode : public TArcDataNode
{
public:
    Reflection settingsContainer;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SettingsContainerNode, TArcDataNode)
    {
        ReflectionRegistrator<SettingsContainerNode>::Begin()
        .Field("container", &SettingsContainerNode::settingsContainer)
        .End();
    }
};
}

void SettingsModule::PostInit()
{
    ContextAccessor* accessor = GetAccessor();

    manager.reset(new SettingsManager(GetAccessor()));
    manager->CreateSettings();

    SettingsModuleDetails::SettingsContainerNode* node = new SettingsModuleDetails::SettingsContainerNode();
    node->settingsContainer = Reflection::Create(ReflectedObject(manager->settings.get()));
    accessor->GetGlobalContext()->CreateData(std::unique_ptr<TArcDataNode>(node));

    executor.DelayedExecute([this]() {
        UI* ui = GetUI();
        ActionPlacementInfo placementInfo(CreateMenuPoint(QList<QString>() << "Tools"));
        {
            QtAction* settingsAction = new QtAction(GetAccessor(), QIcon(":/TArc/Resources/settings.png"), "Settings");
            settingsAction->setMenuRole(QAction::PreferencesRole);
            ui->AddAction(DAVA::mainWindowKey, placementInfo, settingsAction);
            connections.AddConnection(settingsAction, &QAction::triggered, DAVA::MakeFunction(this, &SettingsModule::ShowSettings));
        }
    });
}

void SettingsModule::ShowSettings()
{
    SettingsDialog::Params params;
    params.accessor = GetAccessor();
    params.ui = GetUI();
    params.objectsField.type = ReflectedTypeDB::Get<SettingsModuleDetails::SettingsContainerNode>();
    params.objectsField.fieldName = FastName("container");
    SettingsDialog settingsDialog(params);
    settingsDialog.resetSettings.Connect([this]() {
        manager->ResetToDefault();
    });

    GetUI()->ShowModalDialog(DAVA::mainWindowKey, &settingsDialog);
}

DAVA_VIRTUAL_REFLECTION_IMPL(SettingsModule)
{
    ReflectionRegistrator<SettingsModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void InitColorPickerOptions(bool initForHiddenUsage)
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ColorPickerSettings);

    if (initForHiddenUsage == true)
    {
        EmplaceTypeMeta<ColorPickerSettings>(M::HiddenField());
    }
}
} // namespace DAVA
