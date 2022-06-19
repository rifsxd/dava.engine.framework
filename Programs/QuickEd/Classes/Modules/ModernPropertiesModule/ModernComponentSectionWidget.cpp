#include "ModernComponentSectionWidget.h"

#include "UI/Properties/PredefinedCompletionsProvider.h"
#include "UI/Properties/CompletionsProviderForUIReflection.h"
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
#include <UI/DataBinding/UIDataSourceComponent.h>
#include <UI/DataBinding/UIDataListComponent.h>
#include <UI/DataBinding/UIDataChildFactoryComponent.h>
#include <UI/Events/UIInputEventComponent.h>
#include <UI/Layouts/UIAnchorComponent.h>
#include <UI/Layouts/UISizePolicyComponent.h>
#include <UI/Layouts/UILinearLayoutComponent.h>
#include <UI/Layouts/UIFlowLayoutComponent.h>
#include <UI/Scroll/UIScrollBarDelegateComponent.h>
#include <UI/Components/UIComponentUtils.h>
#include <UI/UIControlBackground.h>
#include <UI/Text/UITextComponent.h>
#include <UI/RichContent/UIRichContentAliasesComponent.h>
#include <UI/Spine/UISpineComponent.h>
#include <UI/Spine/UISpineAttachControlsToBonesComponent.h>
#include <UI/Sound/UISoundComponent.h>

#include <QAction>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QMoveEvent>
#include <QStyle>

ModernComponentSectionWidget::ModernComponentSectionWidget(DAVA::ContextAccessor* accessor_, DAVA::OperationInvoker* invoker_, DAVA::UI* ui_, ControlNode* controlNode_, const DAVA::Type* componentType_)
    : ModernSectionWidget(accessor_, invoker_, ui_, controlNode_)
    , componentType(componentType_)
{
    addRemoveButton = new QToolButton(header);
    QSizePolicy addRemoveButtonSizePolicy = addRemoveButton->sizePolicy();
    addRemoveButtonSizePolicy.setRetainSizeWhenHidden(true);
    addRemoveButtonSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    addRemoveButton->setSizePolicy(addRemoveButtonSizePolicy);
    addRemoveButton->setObjectName("addRemoveButton");
    addRemoveButton->hide();

    header->layout()->addWidget(addRemoveButton);

    connect(addRemoveButton, &QToolButton::clicked, this, &ModernComponentSectionWidget::OnAddRemoveComponent);
    RefreshTitle();

    container->hide();
}

ModernComponentSectionWidget::~ModernComponentSectionWidget()
{
}

void ModernComponentSectionWidget::RecreateProperties()
{
    if (section != nullptr)
    {
        AttachComponentPropertiesSection(section, DAVA::DynamicTypeCheck<RootProperty*>(section->GetRootProperty()));
    }
}

void ModernComponentSectionWidget::AttachComponentPropertiesSection(ComponentPropertiesSection* section_, RootProperty* root)
{
    RemoveAllProperties();
    section = section_;

    int row = 0;
    if (componentType == DAVA::Type::Instance<DAVA::UIAnchorComponent>())
    {
        AddPropertyEditor(section, "enabled", 0, 0, 1);
        AddPropertyEditor(section, "useRtl", 0, 1, 1);
        AddPropertyEditor(section, "leftAnchorEnabled", 1, 0, 1);
        AddPropertyEditor(section, "leftAnchor", 1, 1, -1);
        AddPropertyEditor(section, "hCenterAnchorEnabled", 2, 0, 1);
        AddPropertyEditor(section, "hCenterAnchor", 2, 1, -1);
        AddPropertyEditor(section, "rightAnchorEnabled", 3, 0, 1);
        AddPropertyEditor(section, "rightAnchor", 3, 1, -1);
        AddPropertyEditor(section, "topAnchorEnabled", 4, 0, 1);
        AddPropertyEditor(section, "topAnchor", 4, 1, -1);
        AddPropertyEditor(section, "vCenterAnchorEnabled", 5, 0, 1);
        AddPropertyEditor(section, "vCenterAnchor", 5, 1, -1);
        AddPropertyEditor(section, "bottomAnchorEnabled", 6, 0, 1);
        AddPropertyEditor(section, "bottomAnchor", 6, 1, -1);
    }
    else if (componentType == DAVA::Type::Instance<DAVA::UISizePolicyComponent>())
    {
        QStringList formulaCompletions;
        formulaCompletions << "childrenSum"
                           << "maxChild"
                           << "firstChild"
                           << "lastChild"
                           << "content"
                           << "parent"
                           << "parentRest"
                           << "parentLine"
                           << "min(parentRest, content)"
                           << "max(parent, childrenSum)"
                           << "visibilityMargins.left"
                           << "visibilityMargins.right"
                           << "visibilityMargins.top"
                           << "visibilityMargins.bottom"
                           << "safeAreaInsets.left"
                           << "safeAreaInsets.right"
                           << "safeAreaInsets.top"
                           << "safeAreaInsets.bottom";

        AddSubsection(QObject::tr("Horizontal"), row++);
        AddPropertyEditor(section, "horizontalPolicy", row++, 0, -1);
        AddPropertyEditor(section, "horizontalValue", row, 0, 1);
        AddPropertyEditor(section, "horizontalMin", row, 2, 1);
        AddPropertyEditor(section, "horizontalMax", row++, 4, 1);
        AddCompletionsEditor(section, "horizontalFormula", std::make_unique<PredefinedCompletionsProvider>(formulaCompletions), true, row++, 0, -1);

        AddSubsection(QObject::tr("Vertical"), row++);
        AddPropertyEditor(section, "verticalPolicy", row++, 0, -1);
        AddPropertyEditor(section, "verticalValue", row, 0, 1);
        AddPropertyEditor(section, "verticalMin", row, 2, 1);
        AddPropertyEditor(section, "verticalMax", row++, 4, 1);
        AddCompletionsEditor(section, "verticalFormula", std::make_unique<PredefinedCompletionsProvider>(formulaCompletions), true, row++, 0, -1);
    }
    else if (componentType == DAVA::Type::Instance<DAVA::UILinearLayoutComponent>())
    {
        AddPropertyEditor(section, "enabled", row, 0, 1);
        AddPropertyEditor(section, "useRtl", row++, 1, 1);
        AddPropertyEditor(section, "skipInvisible", row++, 0, -1);
        AddPropertyEditor(section, "orientation", row++, 0, -1);
        AddPropertyEditor(section, "padding", row++, 0, -1);
        AddPropertyEditor(section, "dynamicPadding", row++, 1, -1);
        AddPropertyEditor(section, "spacing", row++, 0, -1);
        AddPropertyEditor(section, "dynamicSpacing", row++, 1, -1);
        AddPropertyEditor(section, "safeAreaPaddingInset", row++, 1, -1);
    }
    else if (componentType == DAVA::Type::Instance<DAVA::UIFlowLayoutComponent>())
    {
        AddPropertyEditor(section, "enabled", row, 0, 1);
        AddPropertyEditor(section, "useRtl", row++, 1, 1);
        AddPropertyEditor(section, "skipInvisible", row++, 0, -1);
        AddPropertyEditor(section, "orientation", row++, 0, -1);

        AddSubsection(QObject::tr("Horizontal"), row++);

        AddPropertyEditor(section, "hPadding", row++, 0, -1);
        AddPropertyEditor(section, "hDynamicPadding", row++, 1, -1);
        AddPropertyEditor(section, "hDynamicInLinePadding", row++, 1, -1);
        AddPropertyEditor(section, "hSpacing", row++, 0, -1);
        AddPropertyEditor(section, "hDynamicSpacing", row++, 1, -1);
        AddPropertyEditor(section, "hSafeAreaPaddingInset", row++, 1, -1);

        AddSubsection(QObject::tr("Vertical"), row++);

        AddPropertyEditor(section, "vPadding", row++, 0, -1);
        AddPropertyEditor(section, "vDynamicPadding", row++, 1, -1);
        AddPropertyEditor(section, "vSpacing", row++, 0, -1);
        AddPropertyEditor(section, "vDynamicSpacing", row++, 1, -1);
        AddPropertyEditor(section, "vSafeAreaPaddingInset", row++, 1, -1);
    }
    else if (componentType == DAVA::Type::Instance<DAVA::UIControlBackground>())
    {
        AddPropertyEditor(section, "drawType", row++, 0, -1);
        AddPropertyEditor(section, "color", row++, 0, -1);
        AddPropertyEditor(section, "colorInherit", row++, 0, -1);
        AddPathPropertyEditor(section, "sprite", { ".psd" }, "/Gfx/", true, row++, 0, -1);
        AddPropertyEditor(section, "frame", row++, 0, -1);
        AddPropertyEditor(section, "align", row++, 0, -1);
        AddPropertyEditor(section, "leftRightStretchCap", row++, 0, -1);
        AddPropertyEditor(section, "topBottomStretchCap", row++, 0, -1);
        AddPropertyEditor(section, "spriteModification", row++, 0, -1);
        AddPathPropertyEditor(section, "mask", { ".psd" }, "/Gfx/", true, row++, 0, -1);
        AddPathPropertyEditor(section, "detail", { ".psd" }, "/Gfx/", true, row++, 0, -1);
        AddPathPropertyEditor(section, "gradient", { ".psd" }, "/Gfx/", true, row++, 0, -1);
        AddPathPropertyEditor(section, "contour", { ".psd" }, "/Gfx/", true, row++, 0, -1);
        AddPropertyEditor(section, "gradientMode", row++, 0, -1);
        AddPropertyEditor(section, "perPixelAccuracy", row++, 0, -1);
    }
    else if (componentType == DAVA::Type::Instance<DAVA::UITextComponent>())
    {
        AddMultilineEditor(section, "text", row++, 0, -1);
        AddPathPropertyEditor(section, "fontPath", { ".ttf", ".otf", ".fnt", ".fntconf" }, "/Fonts/", true, row++, 0, -1);
        AddPropertyEditor(section, "fontSize", row++, 0, -1);
        AddPropertyEditor(section, "fontName", row++, 0, -1);
        AddPropertyEditor(section, "align", row++, 0, -1);
        AddPropertyEditor(section, "multiline", row++, 0, -1);
        AddPropertyEditor(section, "fitting", row++, 0, -1);
        AddPropertyEditor(section, "color", row++, 0, -1);
        AddPropertyEditor(section, "colorInheritType", row++, 0, -1);
        AddPropertyEditor(section, "shadowColor", row++, 0, -1);
        AddPropertyEditor(section, "shadowOffset", row++, 0, -1);
        AddPropertyEditor(section, "perPixelAccuracyType", row++, 0, -1);
        AddPropertyEditor(section, "useRtlAlign", row++, 0, 1);
        AddPropertyEditor(section, "forceBiDiSupport", row++, 0, -1);
    }
    else if (componentType == DAVA::Type::Instance<DAVA::UIScrollBarDelegateComponent>())
    {
        AddCompletionsEditor(section, "delegate", std::make_unique<CompletionsProviderForScrollBar>(), true, row++, 0, 1);
    }
    else if (componentType == DAVA::Type::Instance<DAVA::UIRichContentAliasesComponent>())
    {
        AddTableEditor(section, "aliases", QStringList({ "Alias", "Xml" }), row++, 0, -1);
    }
    else if (componentType == DAVA::Type::Instance<DAVA::UISpineComponent>())
    {
        AddPathPropertyEditor(section, "skeletonPath", { ".json", ".skel" }, "", false, row++, 0, -1);
        AddPathPropertyEditor(section, "atlasPath", { ".atlas" }, "/Gfx/", false, row++, 0, -1);
        AddCompletionsEditor(section, "animationName", std::make_unique<CompletionsProviderForUIReflection>("animationsNames", "UISpineComponent"), false, row++, 0, -1);
        AddCompletionsEditor(section, "skinName", std::make_unique<CompletionsProviderForUIReflection>("skinsNames", "UISpineComponent"), false, row++, 0, -1);
    }
    else if (componentType == DAVA::Type::Instance<DAVA::UISpineAttachControlsToBonesComponent>())
    {
        AddTableEditor(section, "bonesBinds", QStringList({ "Bone", "Control" }), row++, 0, -1);
    }
    else if (componentType == DAVA::Type::Instance<DAVA::UISoundComponent>())
    {
        AddFMODEventEditor(section, "touchDown", row++, 0, -1);
        AddFMODEventEditor(section, "touchUpInside", row++, 0, -1);
        AddFMODEventEditor(section, "touchUpOutside", row++, 0, -1);
        AddFMODEventEditor(section, "valueChanged", row++, 0, -1);
    }
    else if (componentType == DAVA::Type::Instance<DAVA::UIDataSourceComponent>())
    {
        refreshInitiatorProperties.insert("sourceType");
        AddPropertyEditor(section, "sourceType", row++, 0, 1);

        if (section->FindPropertyByName("sourceType")->GetValue().Cast<DAVA::int32>() == DAVA::UIDataSourceComponent::FROM_EXPRESSION)
        {
            AddExpressionEditor(section, "source", row++, 0, 1);
        }
        else
        {
            AddPathPropertyEditor(section, "source", { ".model" }, "/UI/", false, row++, 0, -1);
        }
    }
    else if (componentType == DAVA::Type::Instance<DAVA::UIDataListComponent>())
    {
        AddPathPropertyEditor(section, "cellPackage", { ".yaml" }, "/UI/", false, row++, 0, -1);
        AddPropertyEditor(section, "cellControl", row++, 0, -1);
        AddExpressionEditor(section, "dataContainer", row++, 0, -1);
    }
    else if (componentType == DAVA::Type::Instance<DAVA::UIDataChildFactoryComponent>())
    {
        AddExpressionEditor(section, "package", row++, 0, -1);
        AddExpressionEditor(section, "control", row++, 0, -1);
    }
    else if (componentType == DAVA::Type::Instance<DAVA::UIInputEventComponent>())
    {
        AddPropertyEditor(section, "onTouchDown", row, 0, 1);
        AddExpressionEditor(section, "onTouchDownData", row++, 2, 1);

        AddPropertyEditor(section, "onTouchUpInside", row, 0, 1);
        AddExpressionEditor(section, "onTouchUpInsideData", row++, 2, 1);

        AddPropertyEditor(section, "onTouchUpOutside", row, 0, 1);
        AddExpressionEditor(section, "onTouchUpOutsideData", row++, 2, 1);

        AddPropertyEditor(section, "onValueChanged", row, 0, 1);
        AddExpressionEditor(section, "onValueChangedData", row++, 2, 1);

        AddPropertyEditor(section, "onHoverSet", row, 0, 1);
        AddExpressionEditor(section, "onHoverSetData", row++, 2, 1);

        AddPropertyEditor(section, "onHoverRemoved", row, 0, 1);
        AddExpressionEditor(section, "onHoverRemovedData", row++, 2, 1);
    }

    if (section->GetCount() == 0)
    {
        containerLayout->addWidget(new QLabel(QObject::tr("No Properties"), container), 0, 0);
    }
    else
    {
        AddRestEditorsForSection(section, row);
    }

    container->show();
    RefreshTitle();
}

void ModernComponentSectionWidget::DetachComponentPropertiesSection()
{
    section = nullptr;
    RemoveAllProperties();
    RefreshTitle();
}

const DAVA::Type* ModernComponentSectionWidget::GetComponentType() const
{
    return componentType;
}

ComponentPropertiesSection* ModernComponentSectionWidget::GetSection() const
{
    return section;
}

void ModernComponentSectionWidget::RefreshTitle()
{
    using namespace DAVA;

    bool inherited = false;
    bool created = true;
    if (section)
    {
        inherited = section->GetFlags() & AbstractProperty::EF_INHERITED;
        headerCaption->setText(QObject::tr(section->GetDisplayName().c_str()));

        addRemoveButton->setText(QObject::tr("Remove"));
        addRemoveButton->setVisible(controlNode->GetRootProperty()->CanRemoveComponent(componentType, section->GetComponentIndex()));
    }
    else
    {
        created = false;
        String name = UIComponentUtils::GetDisplayName(componentType);
        headerCaption->setText(QObject::tr(name.c_str()));

        addRemoveButton->setText(QObject::tr("Add"));
        addRemoveButton->setVisible(controlNode->GetRootProperty()->CanAddComponent(componentType));
    }

    header->setDisabled(controlNode->GetRootProperty()->IsReadOnly());
    headerCaption->setProperty("inherited", inherited);
    headerCaption->setProperty("created", created);
    header->setProperty("created", created);
    headerCaption->style()->unpolish(headerCaption);
    headerCaption->style()->polish(headerCaption);
}

void ModernComponentSectionWidget::OnAddRemoveComponent()
{
    using namespace DAVA;
    DVASSERT(accessor->GetActiveContext() != nullptr);

    if (componentType != nullptr)
    {
        CommandExecutor executor(accessor, ui);
        if (section == nullptr)
        {
            executor.AddComponent(controlNode, componentType);
        }
        else
        {
            executor.RemoveComponent(controlNode, componentType, section->GetComponentIndex());
        }
    }
}
