#include "TArc/WindowSubSystem/QtAction.h"

#include <QMenu>

namespace DAVA
{
QtAction::QtAction(ContextAccessor* accessor, QObject* parent)
    : QAction(parent)
    , fieldBinder(accessor)
{
}

QtAction::QtAction(ContextAccessor* accessor, const QString& text, QObject* parent)
    : QAction(text, parent)
    , fieldBinder(accessor)
{
    setToolTip(text);
}

QtAction::QtAction(ContextAccessor* accessor, const QIcon& icon, const QString& text, QObject* parent)
    : QAction(icon, text, parent)
    , fieldBinder(accessor)
{
    setToolTip(text);
}

void QtAction::OnActionTriggered(bool checked, eActionState state, const FieldDescriptor& fieldDescr)
{
    Any value = fieldBinder.GetValue(fieldDescr);
    DVASSERT(!value.IsEmpty());
    OnFieldValueChanged(value, state);
}

void QtAction::SetStateUpdationFunction(eActionState state, const FieldDescriptor& fieldDescr, const Function<Any(const Any&)>& fn)
{
    DVASSERT(functorsMap.count(state) == 0);
    functorsMap.emplace(state, fn);
    fieldBinder.BindField(fieldDescr, Bind(&QtAction::OnFieldValueChanged, this, _1, state));
    connect(this, &QAction::triggered, this, Bind(&QtAction::OnActionTriggered, this, _1, state, fieldDescr));
}

void QtAction::SetStateUpdationFunction(eActionState state, const Reflection& model, const FastName& name, const Function<Any(const Any&)>& fn)
{
    DVASSERT(functorsMap.count(state) == 0);
    functorsMap.emplace(state, fn);
    fieldBinder.BindField(model, name, Bind(&QtAction::OnFieldValueChanged, this, _1, state));
}

void QtAction::OnFieldValueChanged(const Any& value, eActionState state)
{
    const auto iter = functorsMap.find(state);
    DVASSERT(iter != functorsMap.end());
    Any stateResult = iter->second(value);
    switch (state)
    {
    case Enabled:
    {
        DVASSERT(stateResult.CanCast<bool>());
        bool stateEnabled = stateResult.Cast<bool>();
        if (stateEnabled != isEnabled())
        {
            setEnabled(stateEnabled);
            QMenu* m = menu();
            if (m != nullptr)
            {
                m->setEnabled(stateEnabled);
            }
        }
    }
    break;
    case Visible:
    {
        DVASSERT(stateResult.CanCast<bool>());
        bool stateVisible = stateResult.Cast<bool>();
        if (stateVisible != isVisible())
        {
            setVisible(stateVisible);
        }
    }
    break;
    case Checked:
    {
        DVASSERT(stateResult.CanCast<bool>());
        if (isCheckable() == false)
        {
            setCheckable(true);
        }

        bool stateChecked = stateResult.Cast<bool>();
        if (stateChecked != isChecked())
        {
            setChecked(stateChecked);
        }
    }
    break;
    case Text:
        DVASSERT(stateResult.CanCast<String>());
        setText(QString::fromStdString(stateResult.Cast<String>()));
        break;
    case Tooltip:
        DVASSERT(stateResult.CanCast<String>());
        setToolTip(QString::fromStdString(stateResult.Cast<String>()));
        break;
    case Icon:
        if (stateResult.CanCast<QIcon>())
        {
            setIcon(stateResult.Cast<QIcon>());
        }
        else
        {
            DVASSERT(stateResult.CanCast<String>());
            setIcon(QIcon(QString::fromStdString(stateResult.Cast<String>())));
        }
        break;
    default:
        DVASSERT(false);
        break;
    }
}

QtActionSeparator::QtActionSeparator(const QString& name, QObject* parent)
    : QAction(parent)
{
    setObjectName(name);
    setSeparator(true);
}
} // namespace DAVA
