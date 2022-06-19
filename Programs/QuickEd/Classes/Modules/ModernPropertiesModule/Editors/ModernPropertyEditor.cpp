#include "Modules/ModernPropertiesModule/Editors/ModernPropertyEditor.h"

#include "Model/ControlProperties/ValueProperty.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "QECommands/ChangePropertyValueCommand.h"
#include "QECommands/ChangeBindingCommand.h"
#include "QECommands/ChangePropertyForceOverrideCommand.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/DataContext.h>

#include <Base/Any.h>
#include <Engine/Engine.h>
#include <UI/UIControl.h>
#include <UI/UIControlSystem.h>
#include <UI/Styles/UIStyleSheetSystem.h>
#include <UI/Layouts/UILayoutSystem.h>
#include <UI/Layouts/UILayoutIsolationComponent.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QLineEdit>
#include <QStyle>
#include <QEvent>
#include <QMenu>

ModernPropertyContext::ModernPropertyContext(RootProperty* root_, DAVA::ContextAccessor* accessor_, DAVA::OperationInvoker* invoker_, QWidget* parent_)
    : accessor(accessor_)
    , invoker(invoker_)
    , parent(parent_)
{
    root = root_;
}

ModernPropertyContext::~ModernPropertyContext()
{
}

RootProperty* ModernPropertyContext::GetRoot() const
{
    return root.Get();
}

DAVA::ContextAccessor* ModernPropertyContext::GetAccessor() const
{
    return accessor;
}

DAVA::OperationInvoker* ModernPropertyContext::GetInvoker() const
{
    return invoker;
}

QWidget* ModernPropertyContext::GetParent() const
{
    return parent;
}

ModernPropertyEditor::ModernPropertyEditor(const std::shared_ptr<ModernPropertyContext>& context_, ValueProperty* property_)
    : QObject(context_->GetParent())
    , context(context_)
{
    property = property_;

    isLayoutSensitive = property->GetName() == "size" || property->GetName() == "position";

    resetAction = new QAction("Reset", context->GetParent());
    QObject::connect(resetAction, &QAction::triggered, this, &ModernPropertyEditor::ResetProperty);

    forceOverrideAction = new QAction("Force Override", context->GetParent());
    QObject::connect(forceOverrideAction, &QAction::triggered, this, &ModernPropertyEditor::ForceOverride);

    bindAction = new QAction("Bind", context->GetParent());
    QObject::connect(bindAction, &QAction::triggered, this, &ModernPropertyEditor::BindProperty);

    GetRootProperty()->propertyChanged.Connect(this, &ModernPropertyEditor::PropertyChanged);
    DAVA::GetEngineContext()->uiControlSystem->GetStyleSheetSystem()->stylePropertiesChanged.Connect(this, &ModernPropertyEditor::OnStylePropertiesChanged);
    DAVA::GetEngineContext()->uiControlSystem->GetLayoutSystem()->controlLayouted.Connect(this, &ModernPropertyEditor::OnControlLayouted);
}

ModernPropertyEditor::~ModernPropertyEditor()
{
    GetRootProperty()->propertyChanged.Disconnect(this);
    DAVA::GetEngineContext()->uiControlSystem->GetStyleSheetSystem()->stylePropertiesChanged.Disconnect(this);
    DAVA::GetEngineContext()->uiControlSystem->GetLayoutSystem()->controlLayouted.Disconnect(this);
}

bool ModernPropertyEditor::IsBindingEditor() const
{
    return false;
}

ValueProperty* ModernPropertyEditor::GetProperty() const
{
    return property.Get();
}

void ModernPropertyEditor::ChangeProperty(const DAVA::Any& value)
{
    if (!property->IsReadOnly())
    {
        DAVA::DataContext* activeContext = context->GetAccessor()->GetActiveContext();
        if (activeContext)
        {
            DocumentData* documentData = activeContext->GetData<DocumentData>();
            DVASSERT(documentData != nullptr);

            RootProperty* root = DAVA::DynamicTypeCheck<RootProperty*>(property->GetRootProperty());
            documentData->ExecCommand<ChangePropertyValueCommand>(root->GetControlNode(), property.Get(), value);
        }
    }
}

void ModernPropertyEditor::ChangeBinding(const DAVA::String& expr, DAVA::int32 mode)
{
    if (!property->IsReadOnly())
    {
        DAVA::DataContext* activeContext = context->GetAccessor()->GetActiveContext();
        if (activeContext)
        {
            DocumentData* documentData = activeContext->GetData<DocumentData>();
            DVASSERT(documentData != nullptr);

            RootProperty* root = DAVA::DynamicTypeCheck<RootProperty*>(property->GetRootProperty());
            documentData->ExecCommand<ChangeBindingCommand>(root->GetControlNode(), property.Get(), expr, mode);
        }
    }
}

void ModernPropertyEditor::ApplyStyleToWidget(QWidget* widget)
{
    widget->setProperty("overriden", overriden);
    widget->setProperty("setByStyle", setByStyle);
    widget->setProperty("inherited", inherited);

    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
}

void ModernPropertyEditor::OnPropertyChanged()
{
    overriden = property->IsOverriddenLocally();

    setByStyle = false;
    DAVA::int32 propertyIndex = property->GetStylePropertyIndex();
    DAVA::UIControl* control = context->GetRoot()->GetControlNode()->GetControl();
    if (propertyIndex != -1 && !control->GetLocalPropertySet().test(propertyIndex))
    {
        setByStyle = control->GetStyledPropertySet().test(propertyIndex);
    }

    inherited = property->GetFlags() & AbstractProperty::EF_INHERITED;
}

RootProperty* ModernPropertyEditor::GetRootProperty() const
{
    return context->GetRoot();
}

DAVA::ContextAccessor* ModernPropertyEditor::GetAccessor() const
{
    return context->GetAccessor();
}

DAVA::OperationInvoker* ModernPropertyEditor::GetInvoker() const
{
    return context->GetInvoker();
}

QWidget* ModernPropertyEditor::GetParentWidget() const
{
    return context->GetParent();
}

void ModernPropertyEditor::PropertyChanged(AbstractProperty* property_)
{
    if (property.Get() == property_)
    {
        OnPropertyChanged();
    }
}

void ModernPropertyEditor::OnStylePropertiesChanged(DAVA::UIControl* control, const DAVA::UIStyleSheetPropertySet& properties)
{
    DAVA::int32 propertyIndex = property->GetStylePropertyIndex();
    if (propertyIndex != -1 &&
        properties.test(propertyIndex) &&
        control == context->GetRoot()->GetControlNode()->GetControl())
    {
        OnPropertyChanged();
    }
}

void ModernPropertyEditor::OnControlLayouted(DAVA::UIControl* control)
{
    DAVA::UIControl* propertyControl = GetRootProperty()->GetControlNode()->GetControl();
    if (isLayoutSensitive)
    {
        for (DAVA::UIControl* p = propertyControl; p != nullptr; p = p->GetParent())
        {
            if (p == control)
            {
                OnPropertyChanged();
                break;
            }
            if (p->GetComponent<DAVA::UILayoutIsolationComponent>() != nullptr)
            {
                break;
            }
        }
    }
}

void ModernPropertyEditor::ResetProperty()
{
    if (!property->IsReadOnly())
    {
        DAVA::DataContext* activeContext = context->GetAccessor()->GetActiveContext();
        DVASSERT(activeContext != nullptr);
        DocumentData* documentData = activeContext->GetData<DocumentData>();
        DVASSERT(documentData != nullptr);

        RootProperty* root = DAVA::DynamicTypeCheck<RootProperty*>(property->GetRootProperty());
        documentData->ExecCommand<ChangePropertyValueCommand>(root->GetControlNode(), property.Get(), DAVA::Any());
    }
}

void ModernPropertyEditor::BindProperty()
{
    if (!property->IsReadOnly())
    {
        DAVA::DataContext* activeContext = context->GetAccessor()->GetActiveContext();
        DVASSERT(activeContext != nullptr);
        DocumentData* documentData = activeContext->GetData<DocumentData>();
        DVASSERT(documentData != nullptr);

        RootProperty* root = DAVA::DynamicTypeCheck<RootProperty*>(property->GetRootProperty());
        documentData->ExecCommand<ChangeBindingCommand>(root->GetControlNode(), property.Get(), "", 0);
    }
}

void ModernPropertyEditor::ShowActionsMenu(const QPoint& pos)
{
    QMenu menu;
    menu.addAction(resetAction);
    menu.addAction(forceOverrideAction);
    if (property->IsBindable() && !property->IsBound())
    {
        menu.addAction(bindAction);
    }
    menu.exec(pos);
}

void ModernPropertyEditor::ForceOverride()
{
    if (!property->IsReadOnly())
    {
        DAVA::DataContext* activeContext = context->GetAccessor()->GetActiveContext();
        DVASSERT(activeContext != nullptr);
        DocumentData* documentData = activeContext->GetData<DocumentData>();
        DVASSERT(documentData != nullptr);

        RootProperty* root = DAVA::DynamicTypeCheck<RootProperty*>(property->GetRootProperty());
        documentData->ExecCommand<ChangePropertyForceOverrideCommand>(root->GetControlNode(), property.Get());
    }
}

bool ModernPropertyEditor::eventFilter(QObject* o, QEvent* e)
{
    if (e->type() == QEvent::Wheel)
    {
        e->ignore();
        return true;
    }
    return QObject::eventFilter(o, e);
}
