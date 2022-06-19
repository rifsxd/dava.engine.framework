#pragma once

#include "ModernSectionWidget.h"

#include <Base/BaseTypes.h>
#include <Functional/Function.h>

#include <TArc/Core/ContextAccessor.h>
#include <TArc/WindowSubSystem/UI.h>

#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QFormLayout>
#include <QGridLayout>

namespace DAVA
{
class Type;
}

class AbstractProperty;
class ValueProperty;
class ComponentPropertiesSection;
class RootProperty;
class ControlPropertiesSection;
class ControlNode;
class ModernPropertyContext;
class ModernPropertyEditor;
class CompletionsProvider;

class ModernComponentSectionWidget : public ModernSectionWidget
{
    Q_OBJECT
public:
    ModernComponentSectionWidget(DAVA::ContextAccessor* accessor, DAVA::OperationInvoker* invoker, DAVA::UI* ui, ControlNode* controlNode, const DAVA::Type* componentType);
    ~ModernComponentSectionWidget() override;

    void RecreateProperties() override;

    void AttachComponentPropertiesSection(ComponentPropertiesSection* section, RootProperty* root);
    void DetachComponentPropertiesSection();

    const DAVA::Type* GetComponentType() const;
    ComponentPropertiesSection* GetSection() const;
    void RefreshTitle();

private:
    void OnAddRemoveComponent();

    const DAVA::Type* componentType = nullptr;
    ComponentPropertiesSection* section = nullptr;

    QToolButton* addRemoveButton = nullptr;
};
