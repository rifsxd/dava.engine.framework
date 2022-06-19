#include "TArc/Controls/ContentFilter/Private/FilterWidget.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Controls/ReflectedButton.h"
#include "TArc/Controls/Label.h"
#include "TArc/Utils/Utils.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedObject.h>

#include <QPushButton>
#include <QPaintEvent>
#include <QPainter>
#include <QPalette>
#include <QColor>
#include <QMenu>

namespace DAVA
{
FilterWidget::FilterWidget(const FilterWidget::Params& params, DataWrappersProcessor& processor, Reflection model, QWidget* parent)
    : TBase(params, ControlDescriptor(params.fields), &processor, model, parent)
{
    SetupControl();
}

void FilterWidget::ResetModel(Reflection model_)
{
    model = model_;
}

void FilterWidget::paintEvent(QPaintEvent* e)
{
    QPainter p(this);
    QPalette pal = palette();
    QBrush wndColor = pal.window();

    p.setPen(QColor(wndColor.color()));
    p.setBrush(wndColor);
    p.setRenderHint(QPainter::Antialiasing, true);
    QRectF drawRect(0.0f, 0.0f, width(), height());
    p.drawRoundedRect(drawRect, 5.0f, 5.0f);
}

void FilterWidget::contextMenuEvent(QContextMenuEvent* e)
{
    QMenu ctxMenu;
    QString filterTitle = GetTitle();
    QAction* deleteFilter = new QAction(QString("Delete %1").arg(filterTitle), &ctxMenu);
    ctxMenu.addAction(deleteFilter);

    QString text = GetFieldValue<bool>(Fields::Enabled, true) == true ? "Disable" : "Enable";
    QAction* toggleEnabling = new QAction(QString("%1 %2").arg(text).arg(filterTitle), &ctxMenu);
    ctxMenu.addAction(toggleEnabling);

    foreach (QAction* action, actions())
    {
        ctxMenu.addAction(action);
    }

    QAction* triggeredAction = ctxMenu.exec(e->globalPos());
    if (triggeredAction == deleteFilter)
    {
        RemoveFilter();
    }
    else if (triggeredAction == toggleEnabling)
    {
        ToggleEnabling();
    }
}

void FilterWidget::SetupControl()
{
    Reflection thisModel = Reflection::Create(ReflectedObject(this));
    QtHBoxLayout* layout = new QtHBoxLayout();
    layout->setContentsMargins(3, 2, 3, 2);
    layout->setSpacing(0);
    {
        ReflectedPushButton::Params p(controlParams.accessor, controlParams.ui, controlParams.wndKey);
        p.fields[ReflectedPushButton::Fields::Enabled].BindConstValue(true);
        p.fields[ReflectedPushButton::Fields::Icon] = "enabledButtonIcon";
        p.fields[ReflectedPushButton::Fields::IconSize].BindConstValue(QSize(8, 18));
        p.fields[ReflectedPushButton::Fields::Tooltip].BindConstValue("Enable/Disable this filter");
        p.fields[ReflectedPushButton::Fields::Clicked] = "toggleEnabling";
        ReflectedPushButton* button = new ReflectedPushButton(p, p.accessor, thisModel, this);
        QPushButton* pushButton = qobject_cast<QPushButton*>(button->ToWidgetCast());
        pushButton->setFlat(true);
        pushButton->setFixedSize(8, 18);
        layout->AddControl(button);
    }

    {
        ReflectedPushButton::Params p(controlParams.accessor, controlParams.ui, controlParams.wndKey);
        p.fields[ReflectedPushButton::Fields::Enabled].BindConstValue(true);
        p.fields[ReflectedPushButton::Fields::Icon] = "inverseButtonIcon";
        p.fields[ReflectedPushButton::Fields::IconSize].BindConstValue(QSize(18, 18));
        p.fields[ReflectedPushButton::Fields::Tooltip].BindConstValue("Inverse logic of this filter");
        p.fields[ReflectedPushButton::Fields::Clicked] = "toggleInversing";
        ReflectedPushButton* button = new ReflectedPushButton(p, p.accessor, thisModel, this);
        QPushButton* pushButton = qobject_cast<QPushButton*>(button->ToWidgetCast());
        pushButton->setFlat(true);
        pushButton->setFixedSize(18, 18);
        layout->AddControl(button);
    }

    {
        ReflectedPushButton::Params p(controlParams.accessor, controlParams.ui, controlParams.wndKey);
        p.fields[ReflectedPushButton::Fields::Enabled].BindConstValue(true);
        p.fields[ReflectedPushButton::Fields::Text] = "filterTitle";
        p.fields[ReflectedPushButton::Fields::Clicked] = "toggleEnabling";

        ReflectedPushButton* labelButton = new ReflectedPushButton(p, p.accessor, thisModel, this);
        QPushButton* pushButton = qobject_cast<QPushButton*>(labelButton->ToWidgetCast());
        pushButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
        pushButton->setFlat(true);
        pushButton->setMinimumSize(4, 18);
        layout->AddControl(labelButton);
        titleButton = pushButton;
    }
    {
        ReflectedPushButton::Params p(controlParams.accessor, controlParams.ui, controlParams.wndKey);
        p.fields[ReflectedPushButton::Fields::Enabled].BindConstValue(true);
        p.fields[ReflectedPushButton::Fields::Icon].BindConstValue(SharedIcon(":/TArc/ContentFilter/Private/Resources/remove.png"));
        p.fields[ReflectedPushButton::Fields::IconSize].BindConstValue(QSize(18, 18));
        p.fields[ReflectedPushButton::Fields::Tooltip].BindConstValue("Remove filter");
        p.fields[ReflectedPushButton::Fields::Clicked] = "removeFilter";
        ReflectedPushButton* button = new ReflectedPushButton(p, p.accessor, thisModel, this);
        QPushButton* pushButton = qobject_cast<QPushButton*>(button->ToWidgetCast());
        pushButton->setFlat(true);
        pushButton->setFixedSize(18, 18);
        layout->AddControl(button);
    }

    setLayout(layout);

    QPalette labelPalette = titleButton->palette();
    disabledBrush = labelPalette.brush(QPalette::Disabled, QPalette::ButtonText);
    enabledBrush = labelPalette.brush(QPalette::Active, QPalette::ButtonText);
}

void FilterWidget::UpdateControl(const ControlDescriptor& descriptor)
{
    if (descriptor.IsChanged(Fields::Enabled) == true)
    {
        UpdateTitlePalette(GetFieldValue(Fields::Enabled, true));
    }
}

QIcon FilterWidget::GetEnableButtonIcon() const
{
    RETURN_IF_MODEL_LOST(QIcon());

    bool isEnabled = GetFieldValue(Fields::Enabled, true);
    if (isEnabled == true)
    {
        return DAVA::SharedIcon(":/TArc/ContentFilter/Private/Resources/enabled.png");
    }
    else
    {
        return DAVA::SharedIcon(":/TArc/ContentFilter/Private/Resources/disable.png");
    }
}

QIcon FilterWidget::GetInverseButtonIcon() const
{
    RETURN_IF_MODEL_LOST(QIcon());

    bool isEnabled = GetFieldValue(Fields::Inversed, true);
    if (isEnabled == true)
    {
        return DAVA::SharedIcon(":/TArc/ContentFilter/Private/Resources/inverse-enabled.png");
    }
    else
    {
        return DAVA::SharedIcon(":/TArc/ContentFilter/Private/Resources/inverse-disabled.png");
    }
}

QString FilterWidget::GetTitle() const
{
    RETURN_IF_MODEL_LOST(QString());
    return GetFieldValue(Fields::Title, QString("unknown"));
}

void FilterWidget::ToggleEnabling()
{
    bool isEnabled = !GetFieldValue(Fields::Enabled, true);
    wrapper.SetFieldValue(GetFieldName(Fields::Enabled), isEnabled);
    UpdateTitlePalette(isEnabled);
    updateRequire.Emit();
}

void FilterWidget::ToggleInversing()
{
    wrapper.SetFieldValue(GetFieldName(Fields::Inversed), !GetFieldValue(Fields::Inversed, true));
    updateRequire.Emit();
}

void FilterWidget::RemoveFilter()
{
    requestRemoving.Emit();
}

void FilterWidget::UpdateTitlePalette(bool isEnabled)
{
    if (titleButton.isNull() == false)
    {
        QPalette labelPalette = titleButton->palette();
        if (isEnabled == true)
        {
            labelPalette.setBrush(QPalette::Active, QPalette::ButtonText, enabledBrush);
            labelPalette.setBrush(QPalette::Disabled, QPalette::ButtonText, disabledBrush);
        }
        else
        {
            labelPalette.setBrush(QPalette::Active, QPalette::ButtonText, disabledBrush);
            labelPalette.setBrush(QPalette::Disabled, QPalette::ButtonText, enabledBrush);
        }
        titleButton->setPalette(labelPalette);
        titleButton->update();
    }
}

DAVA_REFLECTION_IMPL(FilterWidget)
{
    ReflectionRegistrator<FilterWidget>::Begin()
    .Field("enabledButtonIcon", &FilterWidget::GetEnableButtonIcon, nullptr)
    .Field("inverseButtonIcon", &FilterWidget::GetInverseButtonIcon, nullptr)
    .Field("filterTitle", &FilterWidget::GetTitle, nullptr)
    .Method("toggleEnabling", &FilterWidget::ToggleEnabling)
    .Method("toggleInversing", &FilterWidget::ToggleInversing)
    .Method("removeFilter", &FilterWidget::RemoveFilter)
    .End();
}
} // namespace DAVA
