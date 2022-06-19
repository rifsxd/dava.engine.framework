#include "Modules/ModernPropertiesModule/ModernPropertiesTab.h"

#include "Modules/ModernPropertiesModule/ModernControlSectionWidget.h"
#include "Modules/ModernPropertiesModule/ModernComponentSectionWidget.h"
#include "Modules/ModernPropertiesModule/Editors/ModernPropertyEditor.h"
#include "Modules/DocumentsModule/DocumentData.h"

#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/NameProperty.h"
#include "Model/ControlProperties/ControlPropertiesSection.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"

#include <UI/Components/UIComponentUtils.h>

#include <QScrollArea>
#include <QTimer>

ModernPropertiesTab::ModernPropertiesTab(DAVA::ContextAccessor* accessor_, DAVA::OperationInvoker* invoker_, DAVA::UI* ui_, const DAVA::Vector<const DAVA::Type*>& componentTypes_)
    : accessor(accessor_)
    , invoker(invoker_)
    , ui(ui_)
    , componentTypes(componentTypes_)
{
    documentDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<DocumentData>());
    documentDataWrapper.SetListener(this);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);

    layout()->addWidget(scroll);

    QWidget* content = new QWidget(this);
    scroll->setWidget(content);

    vLayout = new QVBoxLayout();
    vLayout->setMargin(0);
    vLayout->setSpacing(0);

    content->setLayout(vLayout);
}

ModernPropertiesTab::~ModernPropertiesTab()
{
    SetRootProperty(nullptr);
}

void ModernPropertiesTab::OnDataChanged(const DAVA::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    using namespace DAVA;

    bool hasData = wrapper.HasData();
    if (!hasData)
    {
        SetRootProperty(nullptr);
        return;
    }

    bool currentNodeWasChanged = fields.empty() || std::find(fields.begin(), fields.end(), DocumentData::currentNodePropertyName) != fields.end();
    bool packageWasChanged = fields.empty() || std::find(fields.begin(), fields.end(), DocumentData::packagePropertyName) != fields.end();

    if (packageWasChanged)
    {
        SetRootProperty(nullptr);
    }

    if (currentNodeWasChanged)
    {
        Any currentNodeValue = wrapper.GetFieldValue(DocumentData::currentNodePropertyName);
        if (currentNodeValue.CanGet<PackageBaseNode*>())
        {
            PackageBaseNode* currentNode = currentNodeValue.Get<PackageBaseNode*>();
            if (currentNode != nullptr)
            {
                PackageBaseNode* parent = currentNode->GetParent();
                if (parent != nullptr && parent->GetParent() != nullptr && currentNode->GetControl() != nullptr)
                {
                    SetRootProperty(DynamicTypeCheck<ControlNode*>(currentNode)->GetRootProperty());
                    return;
                }
            }
        }

        if (!packageWasChanged)
        {
            SetRootProperty(nullptr);
        }
    }
}

void ModernPropertiesTab::SetRootProperty(RootProperty* root_)
{
    using namespace DAVA;
    QLayoutItem* item = nullptr;
    while ((item = vLayout->takeAt(0)))
    {
        delete item;
    }

    for (ModernSectionWidget* w : componentSections)
    {
        delete w;
    }

    for (ModernSectionWidget* w : controlSections)
    {
        delete w;
    }

    componentSections.clear();
    controlSections.clear();

    if (root)
    {
        root->propertyChanged.Disconnect(this);
        root->componentPropertiesWasAdded.Disconnect(this);
        root->componentPropertiesWasRemoved.Disconnect(this);
    }

    root = root_;
    if (root)
    {
        root->propertyChanged.Connect(this, &ModernPropertiesTab::PropertyChanged);
        root->componentPropertiesWasAdded.Connect(this, &ModernPropertiesTab::ComponentPropertiesWasAdded);
        root->componentPropertiesWasRemoved.Connect(this, &ModernPropertiesTab::ComponentPropertiesWasRemoved);

        if (componentTypes.empty())
        {
            for (int32 i = 0; i < root->GetControlPropertiesSectionsCount(); i++)
            {
                ControlPropertiesSection* section = root->GetControlPropertiesSection(i);
                ModernControlSectionWidget* sectionWidget = new ModernControlSectionWidget(accessor, invoker, ui, root->GetControlNode(), section);

                vLayout->addWidget(sectionWidget);
                controlSections.push_back(sectionWidget);
            }
        }
        else
        {
            for (const Type* componentType : componentTypes)
            {
                int32 index = 0;
                ComponentPropertiesSection* section = nullptr;
                bool sectionWasAdded = false;
                while ((section = root->FindComponentPropertiesSection(componentType, index)) != nullptr)
                {
                    sectionWasAdded = true;
                    ModernComponentSectionWidget* sectionWidget = new ModernComponentSectionWidget(accessor, invoker, ui, root->GetControlNode(), componentType);
                    sectionWidget->AttachComponentPropertiesSection(section, root);
                    vLayout->addWidget(sectionWidget);
                    componentSections.push_back(sectionWidget);
                    index++;
                }

                if (!sectionWasAdded || UIComponentUtils::IsMultiple(componentType))
                {
                    ModernComponentSectionWidget* sectionWidget = new ModernComponentSectionWidget(accessor, invoker, ui, root->GetControlNode(), componentType);
                    vLayout->addWidget(sectionWidget);
                    componentSections.push_back(sectionWidget);
                    index++;
                }
            }
        }
        vLayout->addStretch();
    }
}

void ModernPropertiesTab::PropertyChanged(AbstractProperty* property)
{
    for (ModernComponentSectionWidget* sectionWidget : componentSections)
    {
        if (sectionWidget->GetSection() == property)
        {
            sectionWidget->RefreshTitle();
        }
        else if (sectionWidget->GetSection() == property->GetParent())
        {
            ModernPropertyEditor* editor = sectionWidget->FindEditorForProperty(property);

            if ((editor != nullptr && editor->IsBindingEditor() != property->IsBound()) ||
                sectionWidget->ShouldRecreateForChangedProperty(property->GetName()))
            {
                // recreate section because editor widget must be changed
                sectionWidget->RecreateProperties();
            }
        }
    }

    for (ModernControlSectionWidget* sectionWidget : controlSections)
    {
        if (property->GetParent() == sectionWidget->GetSection())
        {
            ModernPropertyEditor* editor = sectionWidget->FindEditorForProperty(property);
            if (editor != nullptr && editor->IsBindingEditor() != property->IsBound())
            {
                // recreate section because editor widget must be changed
                sectionWidget->RecreateProperties();
            }
        }
    }
}

void ModernPropertiesTab::ComponentPropertiesWasAdded(RootProperty* root, ComponentPropertiesSection* section, int index)
{
    using namespace DAVA;

    for (int32 index = componentSections.size() - 1; index >= 0; index--)
    {
        if (componentSections[index]->GetComponentType() == section->GetComponentType())
        {
            if (componentSections[index]->GetSection() == nullptr)
            {
                componentSections[index]->AttachComponentPropertiesSection(section, root);
            }

            if (UIComponentUtils::IsMultiple(section->GetComponentType()))
            {
                ModernComponentSectionWidget* sectionWidget = new ModernComponentSectionWidget(accessor, invoker, ui, root->GetControlNode(), section->GetComponentType());
                vLayout->insertWidget(index + 1, sectionWidget);
                componentSections.insert(index + 1, sectionWidget);
            }
            break;
        }
    }
}

void ModernPropertiesTab::ComponentPropertiesWasRemoved(RootProperty* root, ComponentPropertiesSection* section, int index)
{
    using namespace DAVA;

    for (int32 index = componentSections.size() - 1; index >= 0; index--)
    {
        if (componentSections[index]->GetSection() == section)
        {
            componentSections[index]->DetachComponentPropertiesSection();
            if (UIComponentUtils::IsMultiple(section->GetComponentType()))
            {
                delete componentSections[index];
                componentSections.removeAt(index);
                break;
            }
        }
    }

    for (int32 index = componentSections.size() - 1; index >= 0; index--)
    {
        componentSections[index]->RefreshTitle();
    }
}
