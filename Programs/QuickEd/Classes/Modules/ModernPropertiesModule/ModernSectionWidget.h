#pragma once

#include <Base/BaseTypes.h>
#include <Functional/Function.h>

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Core/OperationInvoker.h>
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

class ModernSectionWidget : public QWidget
{
    Q_OBJECT
public:
    ModernSectionWidget(DAVA::ContextAccessor* accessor, DAVA::OperationInvoker* invoker, DAVA::UI* ui, ControlNode* controlNode);
    ~ModernSectionWidget() override;

    ModernPropertyEditor* FindEditorForProperty(AbstractProperty* property);
    virtual void RecreateProperties() = 0;
    bool ShouldRecreateForChangedProperty(const DAVA::String& propertyNames);

protected:
    void RemoveAllProperties();
    void RemoveAllPropertiesImpl(QLayout* layout);

    void AddRestEditorsForSection(AbstractProperty* section, int row);

    ModernPropertyEditor* CreateDefaultPropertyEditor(ValueProperty* property) const;

    void AddPropertyEditor(ValueProperty* property, int row, int col, int colSpan = -1);
    ModernPropertyEditor* CreateDefaultPropertyEditor(AbstractProperty* section, const DAVA::String& name) const;
    void AddPropertyEditor(AbstractProperty* section, const DAVA::String& name, int row, int col, int colSpan);
    void AddExpressionEditor(AbstractProperty* section, const DAVA::String& name, int row, int col, int colSpan);
    void AddMultilineEditor(AbstractProperty* section, const DAVA::String& name, int row, int col, int colSpan);
    void AddPathPropertyEditor(AbstractProperty* section, const DAVA::String& name,
                               const QList<QString>& extensions, const QString& resourceSubDir, bool allowAnyExtension, int row, int col, int colSpan);
    void AddCompletionsEditor(AbstractProperty* section, const DAVA::String& name, std::unique_ptr<CompletionsProvider> completionsProvider, bool isEditable, int row, int col, int colSpan);
    void AddTableEditor(AbstractProperty* section, const DAVA::String& name, const QStringList& header, int row, int col, int colSpan);
    void AddFMODEventEditor(AbstractProperty* section, const DAVA::String& name, int row, int col, int colSpan);

    void AddCustomPropertyEditor(AbstractProperty* section, const DAVA::String& name, int row, int col, int colSpan, const DAVA::Function<ModernPropertyEditor*(ValueProperty*)>& createFn);

    void AddSeparator(int row);
    void AddSubsection(const QString& title, int row);

    DAVA::ContextAccessor* accessor = nullptr;
    DAVA::OperationInvoker* invoker = nullptr;
    DAVA::UI* ui = nullptr;
    ControlNode* controlNode = nullptr;

    QGridLayout* containerLayout = nullptr;
    QWidget* header = nullptr;
    QLabel* headerCaption = nullptr;
    QWidget* container = nullptr;

    std::shared_ptr<ModernPropertyContext> context;
    DAVA::UnorderedSet<DAVA::String> createdProperties;
    DAVA::UnorderedSet<DAVA::String> refreshInitiatorProperties;
    DAVA::Vector<ModernPropertyEditor*> editors;
};
