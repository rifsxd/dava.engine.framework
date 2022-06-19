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

class ModernControlSectionWidget : public ModernSectionWidget
{
    Q_OBJECT
public:
    ModernControlSectionWidget(DAVA::ContextAccessor* accessor, DAVA::OperationInvoker* invoker, DAVA::UI* ui, ControlNode* controlNode, ControlPropertiesSection* section);
    ~ModernControlSectionWidget() override;

    ControlPropertiesSection* GetSection() const;
    void RecreateProperties() override;

private:
    ControlPropertiesSection* section = nullptr;
};
