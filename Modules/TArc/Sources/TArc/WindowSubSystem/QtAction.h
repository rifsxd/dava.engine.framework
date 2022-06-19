#pragma once

#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Functional/Function.h"

#include "TArc/Core/FieldBinder.h"
#include "TArc/DataProcessing/Common.h"

#include <Reflection/Reflection.h>

#include <QAction>

namespace DAVA
{
class QtAction : public QAction
{
public:
    QtAction(ContextAccessor* accessor, QObject* parent = nullptr);
    QtAction(ContextAccessor* accessor, const QString& text, QObject* parent = nullptr);
    QtAction(ContextAccessor* accessor, const QIcon& icon, const QString& text, QObject* parent = nullptr);

    enum eActionState
    {
        Enabled, // call back should return Any that can be casted to bool
        Visible,
        Checked,
        Text, // String
        Tooltip, // String
        Icon // String
    };

    void SetStateUpdationFunction(eActionState state, const FieldDescriptor& fieldDescr, const Function<Any(const Any&)>& fn);
    void SetStateUpdationFunction(eActionState state, const Reflection& model, const FastName& name, const Function<Any(const Any&)>& fn);

private:
    void OnFieldValueChanged(const Any& value, eActionState state);
    void OnActionTriggered(bool checked, eActionState state, const FieldDescriptor& fieldDescr);

private:
    FieldBinder fieldBinder;
    UnorderedMap<eActionState, Function<Any(const Any&)>> functorsMap;
};

class QtActionSeparator : public QAction
{
public:
    QtActionSeparator(const QString& name, QObject* parent = nullptr);
};
} // namespace DAVA
