#include "TArc/Controls/ReflectedButton.h"

#include <Base/FastName.h>
#include <Engine/PlatformApiQt.h>
#include <Functional/Function.h>
#include <Reflection/ReflectedMeta.h>
#include <Time/SystemTimer.h>

#include <QPainter>
#include <QPixmap>
#include <QApplication>

namespace DAVA
{
BaseToolButton::BaseToolButton(const BaseToolButton::BaseParams& params, const ControlDescriptor& descriptor, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QToolButton>(params, descriptor, wrappersProcessor, model, parent)
{
    SetupControl();
}

BaseToolButton::BaseToolButton(const BaseToolButton::BaseParams& params, const ControlDescriptor& descriptor, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QToolButton>(params, descriptor, accessor, model, parent)
{
    SetupControl();
}

void BaseToolButton::SetupControl()
{
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setAutoRaise(autoRaise);
    setEnabled(true);

    connections.AddConnection(this, &QToolButton::pressed, MakeFunction(this, &BaseToolButton::ButtonPressed));
    connections.AddConnection(this, &QToolButton::released, MakeFunction(this, &BaseToolButton::ButtonReleased));
}

void BaseToolButton::UpdateControl(const ControlDescriptor& changedFields)
{
    if (changedFields.IsChanged(BaseFields::Visible) == true)
    {
        setVisible(GetFieldValue<bool>(BaseFields::Visible, false));
    }
    if (changedFields.IsChanged(BaseFields::Icon) == true)
    {
        icon = GetFieldValue<QIcon>(BaseFields::Icon, QIcon());
    }
    if (changedFields.IsChanged(BaseFields::Text) == true)
    {
        text = GetFieldValue<QString>(BaseFields::Text, QString());
    }

    setIcon(icon);
    setText(text);

    if (changedFields.IsChanged(BaseFields::Tooltip) == true)
    {
        QString tooltip = GetFieldValue<QString>(BaseFields::Tooltip, QString());
        setToolTip(tooltip);
    }

    if (changedFields.IsChanged(BaseFields::IconSize) == true)
    {
        QSize iconSize = GetFieldValue<QSize>(BaseFields::IconSize, QSize(16, 16));
        setIconSize(iconSize);
    }

    if (icon.isNull() == false && text.isEmpty() == false)
    {
        setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
    else if (icon.isNull() == false)
    {
        setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
    else if (text.isEmpty() == false)
    {
        setToolButtonStyle(Qt::ToolButtonTextOnly);
    }
    else
    {
        DVASSERT(false);
    }

    if (changedFields.IsChanged(BaseFields::AutoRaise) == true)
    {
        autoRaise = GetFieldValue<bool>(BaseFields::AutoRaise, true);
    }
    setAutoRaise(autoRaise);

    if (changedFields.IsChanged(BaseFields::Tooltip) == true)
    {
        setToolTip(GetFieldValue<QString>(BaseFields::Tooltip, QString("")));
    }

    if (changedFields.IsChanged(BaseFields::Enabled) == true)
    {
        bool enabled = GetFieldValue<bool>(BaseFields::Enabled, true);
        setEnabled(enabled);
    }
}

void BaseToolButton::ButtonPressed()
{
}

void BaseToolButton::ButtonReleased()
{
}

void BaseToolButton::EmitClicked()
{
    AnyFn method = model.GetMethod(GetFieldName(BaseFields::Clicked).c_str());
    DVASSERT(method.IsValid());

    Any bindedArgument = GetFieldValue<Any>(BaseFields::BindedArgument, Any());

    const AnyFn::Params& params = method.GetInvokeParams();

    Vector<const Type*> argsType = params.argsType;
    if (argsType.empty() == false)
    {
        DVASSERT(bindedArgument.IsEmpty() == false);
        DVASSERT(argsType.size() == 1);
        DVASSERT(argsType[0]->Decay() == Type::Instance<Any>() || argsType[0] == bindedArgument.GetType());
    }

    if (argsType.empty() == true)
    {
        method.Invoke();
    }
    else
    {
        method.Invoke(bindedArgument);
    }
}

////////////////////////////////////////////////////////////////////////////////

ReflectedButton::ReflectedButton(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : BaseToolButton(params, params.fields, wrappersProcessor, model, parent)
{
}

ReflectedButton::ReflectedButton(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : BaseToolButton(params, params.fields, accessor, model, parent)
{
}

void ReflectedButton::ButtonReleased()
{
    EmitClicked();
}

////////////////////////////////////////////////////////////////////////////////

DelayedToolButton::DelayedToolButton(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent /*= nullptr*/)
    : BaseToolButton(params, params.fields, wrappersProcessor, model, parent)
{
}

DelayedToolButton::DelayedToolButton(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent /*= nullptr*/)
    : BaseToolButton(params, params.fields, accessor, model, parent)
{
    delayTimer.setInterval(16);
    delayTimer.setSingleShot(0);
    connections.AddConnection(&delayTimer, &QTimer::timeout, MakeFunction(this, &DelayedToolButton::RedrawCursor));
}

void DelayedToolButton::ButtonPressed()
{
    pressTime = DAVA::SystemTimer::GetMs();
    delay = GetFieldValue<int32>(Fields::DelayMs, 0);
    delayTimer.start();
    progressBarColor = GetFieldValue(Fields::ProgressBarColor, QColor(Qt::gray));
    RedrawCursor();
}

void DelayedToolButton::ButtonReleased()
{
    delayTimer.stop();
    pressTime = -1;
    delay = 0;
    cursorOverriden = false;
    PlatformApi::Qt::GetApplication()->restoreOverrideCursor();
}

void DelayedToolButton::Timeout()
{
    EmitClicked();
    ButtonReleased();
}

void DelayedToolButton::RedrawCursor()
{
    const QSize targetImageRect(32, 32);
    const float32 penWidth = 4.0f;
    const float32 outlineWidth = 2.0f;
    const float32 fullWith = outlineWidth + penWidth;
    QPixmap cursorImage(targetImageRect);
    cursorImage.fill(Qt::transparent);
    QPainter painter(&cursorImage);
    int64 currentMs = DAVA::SystemTimer::GetMs();
    int32 delta = currentMs - pressTime;

    if (delta >= delay)
    {
        Timeout();
        return;
    }

    int32 startAngle = -16 * 90;
    int32 endAngle = -16 * static_cast<int32>(360.0 * (delta / static_cast<float32>(delay)));

    QRectF rect(fullWith, fullWith, targetImageRect.width() - 2.0f * fullWith, targetImageRect.height() - 2.0f * fullWith);

    painter.setRenderHints(painter.renderHints() | QPainter::Antialiasing);

    QColor outlineColor = progressBarColor;
    outlineColor.setAlpha(0.5f);
    QPen outlinePen(QBrush(outlineColor), fullWith);
    painter.setPen(outlinePen);
    painter.drawArc(rect, startAngle, endAngle);

    QPen pen(QBrush(progressBarColor), penWidth);
    painter.setPen(pen);
    painter.drawArc(rect, startAngle, endAngle);

    QPoint cursorCenter = cursorImage.rect().center();
    if (cursorOverriden == false)
    {
        PlatformApi::Qt::GetApplication()->setOverrideCursor(QCursor(cursorImage, cursorCenter.x(), cursorCenter.y()));
        cursorOverriden = true;
    }
    else
    {
        PlatformApi::Qt::GetApplication()->changeOverrideCursor(QCursor(cursorImage, cursorCenter.x(), cursorCenter.y()));
    }
}

////////////////////////////////////////////////////////////////////////////////

ReflectedPushButton::ReflectedPushButton(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QPushButton>(params, params.fields, wrappersProcessor, model, parent)
{
    SetupControl();
}

ReflectedPushButton::ReflectedPushButton(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QPushButton>(params, params.fields, accessor, model, parent)
{
    SetupControl();
}

void ReflectedPushButton::SetupControl()
{
    setEnabled(true);

    connections.AddConnection(this, &QPushButton::released, MakeFunction(this, &ReflectedPushButton::ButtonReleased));
}

void ReflectedPushButton::UpdateControl(const ControlDescriptor& changedFields)
{
    if (changedFields.IsChanged(Fields::Visible) == true)
    {
        setVisible(GetFieldValue<bool>(Fields::Visible, false));
    }
    if (changedFields.IsChanged(Fields::Icon) == true)
    {
        icon = GetFieldValue<QIcon>(Fields::Icon, QIcon());
    }
    if (changedFields.IsChanged(Fields::Text) == true)
    {
        text = GetFieldValue<QString>(Fields::Text, QString());
    }

    setIcon(icon);
    setText(text);

    if (changedFields.IsChanged(Fields::Tooltip) == true)
    {
        QString tooltip = GetFieldValue<QString>(Fields::Tooltip, QString());
        setToolTip(tooltip);
    }

    if (changedFields.IsChanged(Fields::IconSize) == true)
    {
        QSize iconSize = GetFieldValue<QSize>(Fields::IconSize, QSize(16, 16));
        setIconSize(iconSize);
    }

    if (changedFields.IsChanged(Fields::Tooltip) == true)
    {
        setToolTip(GetFieldValue<QString>(Fields::Tooltip, QString("")));
    }

    if (changedFields.IsChanged(Fields::Enabled) == true)
    {
        bool enabled = GetFieldValue<bool>(Fields::Enabled, true);
        setEnabled(enabled);
    }
}

void ReflectedPushButton::ButtonReleased()
{
    AnyFn method = model.GetMethod(GetFieldName(Fields::Clicked).c_str());
    DVASSERT(method.IsValid());
    DVASSERT(method.GetInvokeParams().argsType.empty() == true, "We could invoke only methods without arguments");
    method.Invoke();
}
} // namespace DAVA
