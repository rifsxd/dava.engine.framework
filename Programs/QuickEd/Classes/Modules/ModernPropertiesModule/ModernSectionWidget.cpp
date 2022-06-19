#include "ModernSectionWidget.h"

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyDefaultEditor.h"
#include "Modules/ModernPropertiesModule/Editors/ModernPropertyStringEditor.h"
#include "Modules/ModernPropertiesModule/Editors/ModernPropertyMultilineEditor.h"
#include "Modules/ModernPropertiesModule/Editors/ModernPropertyFloatEditor.h"
#include "Modules/ModernPropertiesModule/Editors/ModernPropertyIntEditor.h"
#include "Modules/ModernPropertiesModule/Editors/ModernPropertyVectorEditor.h"
#include "Modules/ModernPropertiesModule/Editors/ModernPropertyBoolEditor.h"
#include "Modules/ModernPropertiesModule/Editors/ModernPropertyFlagsEditor.h"
#include "Modules/ModernPropertiesModule/Editors/ModernPropertyEnumEditor.h"
#include "Modules/ModernPropertiesModule/Editors/ModernPropertyColorEditor.h"
#include "Modules/ModernPropertiesModule/Editors/ModernPropertyPathEditor.h"
#include "Modules/ModernPropertiesModule/Editors/ModernPropertyCompletionsEditor.h"
#include "Modules/ModernPropertiesModule/Editors/ModernPropertyTableEditor.h"
#include "Modules/ModernPropertiesModule/Editors/ModernPropertyFMODEventEditor.h"
#include "Modules/ModernPropertiesModule/Editors/ModernPropertyBindingEditor.h"
#include "Modules/ModernPropertiesModule/Editors/ModernPropertyExpressionEditor.h"

#include "UI/Properties/PredefinedCompletionsProvider.h"
#include "UI/Properties/CompletionsProviderForScrollBar.h"

#include "Model/ControlProperties/AbstractProperty.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "Model/ControlProperties/ControlPropertiesSection.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/NameProperty.h"
#include "Model/ControlProperties/CustomClassProperty.h"
#include "Model/ControlProperties/ClassProperty.h"
#include "Model/ControlProperties/PrototypeNameProperty.h"
#include "Utils/QtDavaConvertion.h"
#include "UI/CommandExecutor.h"

#include <Reflection/ReflectedTypeDB.h>

#include <QAction>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QMoveEvent>

ModernSectionWidget::ModernSectionWidget(DAVA::ContextAccessor* accessor_, DAVA::OperationInvoker* invoker_, DAVA::UI* ui_, ControlNode* controlNode_)
    : accessor(accessor_)
    , invoker(invoker_)
    , ui(ui_)
    , controlNode(controlNode_)
{
    using namespace DAVA;

    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setMargin(0);
    headerLayout->setSpacing(4);
    headerLayout->setContentsMargins(4, 0, 4, 0);
    header = new QWidget(this);
    header->installEventFilter(this);
    header->setObjectName("sectionHeader");
    header->setLayout(headerLayout);

    headerCaption = new QLabel(header);
    headerCaption->setObjectName("headerCaption");
    header->layout()->addWidget(headerCaption);

    container = new QWidget(this);
    containerLayout = new QGridLayout();
    containerLayout->setMargin(4);
    container->setLayout(containerLayout);

    QVBoxLayout* sectionLayout = new QVBoxLayout();
    sectionLayout->setMargin(0);
    sectionLayout->setSpacing(0);
    sectionLayout->addWidget(header);
    sectionLayout->addWidget(container);
    setLayout(sectionLayout);

    context = std::make_shared<ModernPropertyContext>(controlNode->GetRootProperty(), accessor, invoker, this);
}

ModernSectionWidget::~ModernSectionWidget()
{
    RemoveAllProperties();
}

ModernPropertyEditor* ModernSectionWidget::FindEditorForProperty(AbstractProperty* property)
{
    for (ModernPropertyEditor* e : editors)
    {
        if (e->GetProperty() == property)
        {
            return e;
        }
    }
    return nullptr;
}

bool ModernSectionWidget::ShouldRecreateForChangedProperty(const DAVA::String& propertyNames)
{
    return refreshInitiatorProperties.find(propertyNames) != refreshInitiatorProperties.end();
}

void ModernSectionWidget::RemoveAllProperties()
{
    container->hide();
    for (ModernPropertyEditor* e : editors)
    {
        delete e;
    }
    editors.clear();
    createdProperties.clear();
    refreshInitiatorProperties.clear();
    RemoveAllPropertiesImpl(containerLayout);
}

void ModernSectionWidget::RemoveAllPropertiesImpl(QLayout* layout)
{
    QLayoutItem* child = nullptr;
    while ((child = layout->takeAt(0)) != nullptr)
    {
        if (child->layout())
        {
            RemoveAllPropertiesImpl(child->layout());
        }
        delete child->widget();
        delete child;
    }
}

void ModernSectionWidget::AddRestEditorsForSection(AbstractProperty* section, int row)
{
    using namespace DAVA;

    int lastRow = row;
    for (uint32 i = 0; i < section->GetCount(); i++)
    {
        ValueProperty* property = DynamicTypeCheck<ValueProperty*>(section->GetProperty(i));
        if (createdProperties.find(property->GetName()) == createdProperties.end())
        {
            AddPropertyEditor(property, lastRow++, 0);
        }
    }
}

ModernPropertyEditor* ModernSectionWidget::CreateDefaultPropertyEditor(ValueProperty* property) const
{
    using namespace DAVA;

    if (property->GetType() == AbstractProperty::TYPE_ENUM)
    {
        return new ModernPropertyEnumEditor(context, property);
    }
    else if (property->GetType() == AbstractProperty::TYPE_FLAGS)
    {
        return new ModernPropertyFlagsEditor(context, property);
    }
    else if (property->GetValueType() == Type::Instance<bool>())
    {
        return new ModernPropertyBoolEditor(context, property);
    }
    else if (property->GetValueType() == Type::Instance<float32>())
    {
        return new ModernPropertyFloatEditor(context, property);
    }
    else if (property->GetValueType() == Type::Instance<int8>() ||
             property->GetValueType() == Type::Instance<uint8>() ||
             property->GetValueType() == Type::Instance<int16>() ||
             property->GetValueType() == Type::Instance<uint16>() ||
             property->GetValueType() == Type::Instance<int32>() ||
             property->GetValueType() == Type::Instance<uint32>() ||
             property->GetValueType() == Type::Instance<int64>() ||
             property->GetValueType() == Type::Instance<uint64>())
    {
        return new ModernPropertyIntEditor(context, property);
    }
    else if (property->GetValueType() == Type::Instance<String>() ||
             property->GetValueType() == Type::Instance<FastName>())
    {
        return new ModernPropertyStringEditor(context, property);
    }
    else if (property->GetValueType() == Type::Instance<Vector2>())
    {
        return new ModernPropertyVectorEditor(context, property, ModernPropertyVectorEditor::COMPONENTS_XY);
    }
    else if (property->GetValueType() == Type::Instance<Vector3>())
    {
        return new ModernPropertyVectorEditor(context, property, ModernPropertyVectorEditor::COMPONENTS_XYZ);
    }
    else if (property->GetValueType() == Type::Instance<Vector4>())
    {
        return new ModernPropertyVectorEditor(context, property, ModernPropertyVectorEditor::COMPONENTS_XYZW);
    }
    else if (property->GetValueType() == Type::Instance<Color>())
    {
        return new ModernPropertyColorEditor(context, property);
    }
    else if (property->GetValueType() == Type::Instance<FilePath>())
    {
        return new ModernPropertyPathEditor(context, property, { ".*" }, "/", true);
    }
    else
    {
        DVASSERT(false);
        return nullptr;
    }
}

void ModernSectionWidget::AddPropertyEditor(ValueProperty* property, int row, int col, int colSpan)
{
    createdProperties.insert(property->GetName());
    ModernPropertyEditor* w = nullptr;
    if (property->IsBound())
    {
        w = new ModernPropertyBindingEditor(context, property);
    }
    else
    {
        w = CreateDefaultPropertyEditor(property);
    }
    if (w)
    {
        w->AddToGrid(containerLayout, row, col, colSpan);
        editors.push_back(w);
    }
}

ModernPropertyEditor* ModernSectionWidget::CreateDefaultPropertyEditor(AbstractProperty* section, const DAVA::String& name) const
{
    using namespace DAVA;
    for (uint32 i = 0; i < section->GetCount(); i++)
    {
        ValueProperty* property = DynamicTypeCheck<ValueProperty*>(section->GetProperty(i));
        if (property->GetName() == name)
        {
            return ModernSectionWidget::CreateDefaultPropertyEditor(property);
        }
    }

    DVASSERT(false);
    return nullptr;
}

void ModernSectionWidget::AddPropertyEditor(AbstractProperty* section, const DAVA::String& name, int row, int col, int colSpan)
{
    AddCustomPropertyEditor(section, name, row, col, colSpan, [this](ValueProperty* property) {
        return CreateDefaultPropertyEditor(property);
    });
}

void ModernSectionWidget::AddExpressionEditor(AbstractProperty* section, const DAVA::String& name, int row, int col, int colSpan)
{
    AddCustomPropertyEditor(section, name, row, col, colSpan, [this](ValueProperty* property) {
        return new ModernPropertyExpressionEditor(context, property);
    });
}

void ModernSectionWidget::AddMultilineEditor(AbstractProperty* section, const DAVA::String& name, int row, int col, int colSpan)
{
    AddCustomPropertyEditor(section, name, row, col, colSpan, [this](ValueProperty* property) {
        return new ModernPropertyMultilineEditor(context, property);
    });
}

void ModernSectionWidget::AddPathPropertyEditor(AbstractProperty* section, const DAVA::String& name,
                                                const QList<QString>& extensions, const QString& resourceSubDir, bool allowAnyExtension, int row, int col, int colSpan)
{
    AddCustomPropertyEditor(section, name, row, col, colSpan, [this, &extensions, &resourceSubDir, allowAnyExtension](ValueProperty* property) {
        return new ModernPropertyPathEditor(context, property, extensions, resourceSubDir, allowAnyExtension);
    });
}

void ModernSectionWidget::AddCompletionsEditor(AbstractProperty* section, const DAVA::String& name, std::unique_ptr<CompletionsProvider> completionsProvider, bool isEditable, int row, int col, int colSpan)
{
    AddCustomPropertyEditor(section, name, row, col, colSpan, [this, &completionsProvider, isEditable](ValueProperty* property) {
        return new ModernPropertyCompletionsEditor(context, property, std::move(completionsProvider), isEditable);
    });
}

void ModernSectionWidget::AddTableEditor(AbstractProperty* section, const DAVA::String& name, const QStringList& header, int row, int col, int colSpan)
{
    AddCustomPropertyEditor(section, name, row, col, colSpan, [this, header](ValueProperty* property) {
        return new ModernPropertyTableEditor(context, property, header);
    });
}

void ModernSectionWidget::AddFMODEventEditor(AbstractProperty* section, const DAVA::String& name, int row, int col, int colSpan)
{
    AddCustomPropertyEditor(section, name, row, col, colSpan, [this](ValueProperty* property) {
        return new ModernPropertyFMODEventEditor(context, property);
    });
}

void ModernSectionWidget::AddCustomPropertyEditor(AbstractProperty* section, const DAVA::String& name, int row, int col, int colSpan, const DAVA::Function<ModernPropertyEditor*(ValueProperty*)>& createFn)
{
    using namespace DAVA;
    for (uint32 i = 0; i < section->GetCount(); i++)
    {
        ValueProperty* property = DynamicTypeCheck<ValueProperty*>(section->GetProperty(i));
        if (property->GetName() == name)
        {
            ModernPropertyEditor* w = nullptr;
            if (property->IsBound())
            {
                w = new ModernPropertyBindingEditor(context, property);
            }
            else
            {
                w = createFn(property);
            }
            createdProperties.insert(name);
            w->AddToGrid(containerLayout, row, col, colSpan);
            editors.push_back(w);
            break;
        }
    }
}

void ModernSectionWidget::AddSeparator(int row)
{
    QFrame* hFrame = new QFrame(this);
    hFrame->setContentsMargins(4, 0, 4, 0);
    hFrame->setFrameShape(QFrame::HLine);
    containerLayout->addWidget(hFrame, row, 0, 1, -1);
}

void ModernSectionWidget::AddSubsection(const QString& title, int row)
{
    QHBoxLayout* l = new QHBoxLayout();
    l->setMargin(0);
    l->setSpacing(4);
    l->setContentsMargins(4, 0, 4, 0);

    QFrame* fr1 = new QFrame(this);
    fr1->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    fr1->setFrameShape(QFrame::HLine);
    l->addWidget(fr1);

    QLabel* label = new QLabel(title, this);
    l->addWidget(label);

    QFrame* fr2 = new QFrame(this);
    fr2->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    fr2->setFrameShape(QFrame::HLine);
    l->addWidget(fr2);

    containerLayout->addLayout(l, row, 0, 1, -1);
}
