#include "ModernControlSectionWidget.h"

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

#include <QHBoxLayout>
#include <QMoveEvent>
#include <QStyle>

ModernControlSectionWidget::ModernControlSectionWidget(DAVA::ContextAccessor* accessor_, DAVA::OperationInvoker* invoker_, DAVA::UI* ui_, ControlNode* controlNode_, ControlPropertiesSection* section_)
    : ModernSectionWidget(accessor_, invoker_, ui_, controlNode_)
    , section(section_)
{
    using namespace DAVA;

    header->setDisabled(controlNode->GetRootProperty()->IsReadOnly());
    header->setProperty("created", true);

    headerCaption->setText(QString::fromStdString(section->GetDisplayName()));
    headerCaption->setProperty("inherited", false);
    headerCaption->setProperty("created", true);
    headerCaption->style()->unpolish(headerCaption);
    headerCaption->style()->polish(headerCaption);

    RecreateProperties();
}

ModernControlSectionWidget::~ModernControlSectionWidget()
{
}

ControlPropertiesSection* ModernControlSectionWidget::GetSection() const
{
    return section;
}

void ModernControlSectionWidget::RecreateProperties()
{
    RemoveAllProperties();

    int row = 0;
    if (section->GetName() == "UIControl")
    {
        header->hide();

        RootProperty* root = controlNode->GetRootProperty();
        AddPropertyEditor(root->GetNameProperty(), row++, 0);
        if (root->GetControlNode()->GetPrototype())
        {
            AddPropertyEditor(root->GetPrototypeProperty(), row++, 0);
        }
        AddPropertyEditor(root->GetClassProperty(), row++, 0);

        AddPropertyEditor(root->GetCustomClassProperty(), row++, 0);
        AddPropertyEditor(section, "position", row++, 0, -1);
        AddPropertyEditor(section, "size", row++, 0, -1);
        AddPropertyEditor(section, "scale", row++, 0, -1);
        AddPropertyEditor(section, "pivot", row++, 0, -1);
        AddPropertyEditor(section, "angle", row++, 0, -1);
        AddPropertyEditor(section, "visible", row++, 0, -1);
        AddPropertyEditor(section, "enabled", row++, 0, -1);
        AddPropertyEditor(section, "selected", row++, 0, -1);
        AddPropertyEditor(section, "noInput", row++, 0, -1);
        AddPropertyEditor(section, "exclusiveInput", row++, 0, -1);
        AddPropertyEditor(section, "wheelSensitivity", row++, 0, -1);
        AddPropertyEditor(section, "tag", row++, 0, -1);
        AddPropertyEditor(section, "classes", row++, 0, -1);
    }
    else if (section->GetName() == "UIParticles")
    {
        AddPathPropertyEditor(section, "effectPath", { ".sc2" }, "/3d/", false, row++, 0, -1);
    }
    else if (section->GetName() == "UITextField")
    {
        AddPropertyEditor(section, "text", row++, 0, -1);
        AddPathPropertyEditor(section, "fontPath", { ".ttf", ".otf", ".fnt", ".fntconf" }, "/Fonts/", true, row++, 0, -1);
        AddPropertyEditor(section, "fontSize", row++, 0, -1);
        AddPropertyEditor(section, "font", row++, 0, -1);
    }

    AddRestEditorsForSection(section, row);

    container->show();
}
