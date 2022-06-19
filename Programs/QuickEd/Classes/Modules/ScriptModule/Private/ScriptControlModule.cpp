#include "Modules/ScriptModule/ScriptControlModule.h"

#include <Engine/EngineContext.h>
#include <UI/UIControlSystem.h>

#include <UI/Script/UIScriptSystem.h>

#include <TArc/Qt/QtIcon.h>
#include <TArc/Controls/QtBoxLayouts.h>
#include <TArc/Controls/ReflectedButton.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/Utils.h>

#include <QAction>
#include <QWidget>

void ScriptControlModule::OnContextCreated(DAVA::DataContext* context)
{
}

void ScriptControlModule::OnContextDeleted(DAVA::DataContext* context)
{
}

void ScriptControlModule::PostInit()
{
    using namespace DAVA;
    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    QWidget* w = new QWidget();
    QtHBoxLayout* layout = new QtHBoxLayout(w);
    layout->setMargin(0);
    layout->setSpacing(4);

    {
        ReflectedButton::Params params(accessor, ui, DAVA::mainWindowKey);
        params.fields[ReflectedButton::Fields::Enabled] = "isEnabled";
        params.fields[ReflectedButton::Fields::Clicked] = "playPause";
        params.fields[ReflectedButton::Fields::Tooltip] = "pauseButtonHint";
        params.fields[ReflectedButton::Fields::Icon] = "pauseButtonIcon";
        layout->AddControl(new ReflectedButton(params, accessor, DAVA::Reflection::Create(DAVA::ReflectedObject(this)), w));
    }

    QString toolbarName = "Script Toolbar";
    ActionPlacementInfo toolbarTogglePlacement(CreateMenuPoint(QList<QString>() << "View"
                                                                                << "Toolbars"));
    ui->DeclareToolbar(DAVA::mainWindowKey, toolbarTogglePlacement, toolbarName);

    QAction* action = new QAction(nullptr);
    AttachWidgetToAction(action, w);

    ActionPlacementInfo placementInfo(CreateToolbarPoint(toolbarName));
    ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
}

const QIcon& ScriptControlModule::GetPauseButtonIcon()
{
    DAVA::UIScriptSystem* scriptSystem = GetScriptSystem();
    if (scriptSystem != nullptr && !scriptSystem->IsPauseProcessing())
    {
        return DAVA::SharedIcon(":/Icons/pause.png");
    }
    else
    {
        return DAVA::SharedIcon(":/Icons/play.png");
    }
}

void ScriptControlModule::PlayPause()
{
    DAVA::UIScriptSystem* scriptSystem = GetScriptSystem();
    if (scriptSystem != nullptr)
    {
        scriptSystem->SetPauseProcessing(!scriptSystem->IsPauseProcessing());
    }
}

bool ScriptControlModule::IsEnabled() const
{
    return GetScriptSystem() != nullptr;
}

DAVA::UIScriptSystem* ScriptControlModule::GetScriptSystem() const
{
    using namespace DAVA;

    const ContextAccessor* accessor = GetAccessor();
    return accessor != nullptr ? accessor->GetEngineContext()->uiControlSystem->GetSystem<UIScriptSystem>() : nullptr;
}

DAVA_VIRTUAL_REFLECTION_IMPL(ScriptControlModule)
{
    DAVA::ReflectionRegistrator<ScriptControlModule>::Begin()
    .ConstructorByPointer()
    .Field("isEnabled", &ScriptControlModule::IsEnabled, nullptr)
    .Field("pauseButtonHint", &ScriptControlModule::pauseButtonHint)
    .Field("pauseButtonIcon", &ScriptControlModule::GetPauseButtonIcon, nullptr)
    .Method("playPause", &ScriptControlModule::PlayPause)
    .End();
}

DECL_TARC_MODULE(ScriptControlModule);
