#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Controls/ControlDescriptor.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/Qt/QtString.h"
#include "TArc/Qt/QtIcon.h"

#include <QToolButton>
#include <QPushButton>
#include <QTimer>

namespace DAVA
{
class BaseToolButton : public ControlProxyImpl<QToolButton>
{
public:
    enum BaseFields : uint32
    {
        Clicked,
        BindedArgument,
        Text, // QString
        Icon, // QIcon
        IconSize, // QSize
        AutoRaise, // bool
        Enabled, // bool
        Visible, // bool
        Tooltip, // QString
    };

    using BaseParams = typename ControlProxyImpl<QToolButton>::BaseParams;
    BaseToolButton(const BaseParams& params, const ControlDescriptor& descriptor, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    BaseToolButton(const BaseParams& params, const ControlDescriptor& descriptor, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

protected:
    virtual void ButtonPressed();
    virtual void ButtonReleased();

    void EmitClicked();

    QtConnections connections;

private:
    void UpdateControl(const ControlDescriptor& changedfields) override;
    void SetupControl();

    QIcon icon;
    QString text;
    bool autoRaise = true;
};

class ReflectedButton : public BaseToolButton
{
public:
    enum class Fields : uint32
    {
        Clicked = BaseToolButton::BaseFields::Clicked,
        BindedArgument = BaseToolButton::BaseFields::BindedArgument,
        Text = BaseToolButton::BaseFields::Text, // QString
        Icon = BaseToolButton::BaseFields::Icon, // QIcon
        IconSize = BaseToolButton::BaseFields::IconSize, // QSize
        AutoRaise = BaseToolButton::BaseFields::AutoRaise, // bool
        Enabled = BaseToolButton::BaseFields::Enabled, // bool
        Visible = BaseToolButton::BaseFields::Visible, // bool
        Tooltip = BaseToolButton::BaseFields::Tooltip, // QString
        FieldCount,
    };

    DECLARE_CONTROL_PARAMS(Fields);
    ReflectedButton(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    ReflectedButton(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

protected:
    void ButtonReleased() override;
};

class DelayedToolButton : public BaseToolButton
{
public:
    enum class Fields : uint32
    {
        Clicked = BaseToolButton::BaseFields::Clicked,
        BindedArgument = BaseToolButton::BaseFields::BindedArgument,
        Text = BaseToolButton::BaseFields::Text, // QString
        Icon = BaseToolButton::BaseFields::Icon, // QIcon
        IconSize = BaseToolButton::BaseFields::IconSize, // QSize
        AutoRaise = BaseToolButton::BaseFields::AutoRaise, // bool
        Enabled = BaseToolButton::BaseFields::Enabled, // bool
        Visible = BaseToolButton::BaseFields::Visible, // bool
        Tooltip = BaseToolButton::BaseFields::Tooltip, // QString
        DelayMs, // int32
        ProgressBarColor, // QColor
        FieldCount,
    };

    DECLARE_CONTROL_PARAMS(Fields);
    DelayedToolButton(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    DelayedToolButton(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

protected:
    void ButtonPressed() override;
    void ButtonReleased() override;

    void Timeout();
    void RedrawCursor();

private:
    QTimer delayTimer;
    QColor progressBarColor;
    int64 pressTime = -1;
    int32 delay = 0;
    bool cursorOverriden = false;
};

class ReflectedPushButton : public ControlProxyImpl<QPushButton>
{
public:
    enum class Fields : uint32
    {
        Clicked,
        Text, // QString
        Icon, // QIcon
        IconSize, // QSize
        Enabled, // bool
        Visible, // bool
        Tooltip, // QString
        FieldCount,
    };

    DECLARE_CONTROL_PARAMS(Fields);
    ReflectedPushButton(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    ReflectedPushButton(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void UpdateControl(const ControlDescriptor& changedfields) override;
    void SetupControl();

    void ButtonReleased();

    QIcon icon;
    QString text;
    QtConnections connections;
};
} // namespace DAVA
