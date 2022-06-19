#pragma once

#include <TArc/DataProcessing/DataWrapper.h>
#include <TArc/DataProcessing/DataListener.h>

#include <Base/Type.h>

#include <QWidget>
#include <QVBoxLayout>

class QScrollArea;

class RootProperty;
class AbstractProperty;
class ComponentPropertiesSection;
class ModernComponentSectionWidget;
class ModernControlSectionWidget;
class ModernPropertyEditorFactory;

namespace DAVA
{
class ContextAccessor;
class OperationInvoker;
class UI;
}

class ModernPropertiesTab : public QWidget, private DAVA::DataListener
{
    Q_OBJECT

public:
    ModernPropertiesTab(DAVA::ContextAccessor* accessor, DAVA::OperationInvoker* invoker, DAVA::UI* ui, const DAVA::Vector<const DAVA::Type*>& componentTypes);
    ~ModernPropertiesTab() override;

private:
    void OnDataChanged(const DAVA::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;
    void SetRootProperty(RootProperty* root);

    void PropertyChanged(AbstractProperty* property);
    void ComponentPropertiesWasAdded(RootProperty* root, ComponentPropertiesSection* section, int index);
    void ComponentPropertiesWasRemoved(RootProperty* root, ComponentPropertiesSection* section, int index);

    DAVA::ContextAccessor* accessor = nullptr;
    DAVA::OperationInvoker* invoker = nullptr;
    DAVA::UI* ui = nullptr;
    DAVA::DataWrapper documentDataWrapper;

    QVector<ModernComponentSectionWidget*> componentSections;
    QVector<ModernControlSectionWidget*> controlSections;

    QVBoxLayout* vLayout = nullptr;
    QScrollArea* scroll = nullptr;

    RootProperty* root = nullptr;

    DAVA::Vector<const DAVA::Type*> componentTypes;
};
