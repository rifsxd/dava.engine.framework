#include "Modules/ModernPropertiesModule/ModernPropertiesModule.h"

#include "Modules/ModernPropertiesModule/ModernPropertiesTab.h"
#include "Modules/DocumentsModule/DocumentData.h"

#include "Model/ControlProperties/ComponentPropertiesSection.h"

#include <Engine/Engine.h>
#include <Entity/ComponentManager.h>
#include <Reflection/ReflectedTypeDB.h>

#include <UI/Components/UIComponentUtils.h>

// layout components
#include <UI/Layouts/UILinearLayoutComponent.h>
#include <UI/Layouts/UIFlowLayoutComponent.h>
#include <UI/Layouts/UIFlowLayoutHintComponent.h>
#include <UI/Layouts/UIIgnoreLayoutComponent.h>
#include <UI/Layouts/UISizePolicyComponent.h>
#include <UI/Layouts/UIAnchorComponent.h>
#include <UI/Layouts/UILayoutIsolationComponent.h>
#include <UI/Layouts/UILayoutSourceRectComponent.h>

// content components
#include <UI/UIControlBackground.h>
#include <UI/Text/UITextComponent.h>
#include <UI/Render/UIDebugRenderComponent.h>
#include <UI/Render/UIClipContentComponent.h>
#include <UI/RichContent/UIRichContentComponent.h>
#include <UI/RichContent/UIRichContentAliasesComponent.h>
#include <UI/Spine/UISpineComponent.h>
#include <UI/Spine/UISpineAttachControlsToBonesComponent.h>

// input components
#include <UI/Input/UIModalInputComponent.h>
#include <UI/Focus/UIFocusComponent.h>
#include <UI/Focus/UIFocusGroupComponent.h>
#include <UI/Focus/UINavigationComponent.h>
#include <UI/Focus/UITabOrderComponent.h>
#include <UI/Scroll/UIScrollComponent.h>
#include <UI/Scroll/UIScrollBarDelegateComponent.h>
#include <UI/Events/UIEventBindingComponent.h>
#include <UI/Events/UIInputEventComponent.h>
#include <UI/Events/UIMovieEventComponent.h>
#include <UI/Events/UIShortcutEventComponent.h>

#include <TArc/Controls/QtBoxLayouts.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

#include <QTabWidget>
#include <QLabel>

DAVA_VIRTUAL_REFLECTION_IMPL(ModernPropertiesModule)
{
    DAVA::ReflectionRegistrator<ModernPropertiesModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void ModernPropertiesModule::PostInit()
{
    using namespace DAVA;

    const char* title = "Modern Properties";
    DockPanelInfo panelInfo;
    panelInfo.title = title;
    panelInfo.area = Qt::RightDockWidgetArea;
    PanelKey key(title, panelInfo);

    QTabWidget* widget = new QTabWidget();

    GetUI()->AddView(DAVA::mainWindowKey, key, widget);

    UnorderedMap<String, Vector<const Type*>> components;

    auto AddToGroup = [&components](const Type* t) {
        if (!UIComponentUtils::IsHidden(t))
        {
            String groupName = UIComponentUtils::GetGroupName(t);
            if (groupName == "")
            {
                groupName = "Other";
            }
            Vector<const Type*>& group = components[groupName];
            if (std::find(group.begin(), group.end(), t) == group.end())
            {
                group.push_back(t);
            }
        }
    };

    // Process predefined section orders
    AddToGroup(Type::Instance<UISizePolicyComponent>());
    AddToGroup(Type::Instance<UIAnchorComponent>());
    AddToGroup(Type::Instance<UILinearLayoutComponent>());
    AddToGroup(Type::Instance<UIFlowLayoutComponent>());
    AddToGroup(Type::Instance<UIFlowLayoutHintComponent>());
    AddToGroup(Type::Instance<UIIgnoreLayoutComponent>());

    AddToGroup(Type::Instance<UIControlBackground>());
    AddToGroup(Type::Instance<UITextComponent>());
    AddToGroup(Type::Instance<UIClipContentComponent>());
    AddToGroup(Type::Instance<UIDebugRenderComponent>());
    AddToGroup(Type::Instance<UIRichContentComponent>());
    AddToGroup(Type::Instance<UIRichContentAliasesComponent>());
    AddToGroup(Type::Instance<UISpineComponent>());
    AddToGroup(Type::Instance<UISpineAttachControlsToBonesComponent>());

    AddToGroup(Type::Instance<UIModalInputComponent>());
    AddToGroup(Type::Instance<UIFocusComponent>());
    AddToGroup(Type::Instance<UIFocusGroupComponent>());
    AddToGroup(Type::Instance<UINavigationComponent>());
    AddToGroup(Type::Instance<UITabOrderComponent>());
    AddToGroup(Type::Instance<UIScrollBarDelegateComponent>());

    // Process rest section orders
    const Vector<const Type*>& allComponents = GetEngineContext()->componentManager->GetRegisteredUIComponents();
    for (const Type* c : allComponents)
    {
        AddToGroup(c);
    }

    // Add control properties
    widget->addTab(new ModernPropertiesTab(GetAccessor(), GetInvoker(), GetUI(), Vector<const Type*>()), "General");

    // Add priority component group properties
    Vector<String> priorityGroups = { "Content", "Layout", "Input", "Data" };
    for (const String& priorityGroup : priorityGroups)
    {
        auto it = components.find(priorityGroup);
        if (it != components.end())
        {
            widget->addTab(new ModernPropertiesTab(GetAccessor(), GetInvoker(), GetUI(), it->second), it->first.c_str());
        }
    }

    // Add other component group properties
    for (auto& it : components)
    {
        if (std::find(priorityGroups.begin(), priorityGroups.end(), it.first) == priorityGroups.end())
        {
            widget->addTab(new ModernPropertiesTab(GetAccessor(), GetInvoker(), GetUI(), it.second), it.first.c_str());
        }
    }
}

DECL_TARC_MODULE(ModernPropertiesModule);
