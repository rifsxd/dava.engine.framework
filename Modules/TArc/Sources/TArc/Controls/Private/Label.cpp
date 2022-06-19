#include "TArc/Controls/Label.h"
#include "TArc/Controls/CommonStrings.h"
#include "TArc/Qt/QtString.h"

#include <Base/FastName.h>
#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
Label::Label(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QLabel>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
}

Label::Label(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QLabel>(params, ControlDescriptor(params.fields), accessor, model, parent)
{
}

void Label::UpdateControl(const ControlDescriptor& descriptor)
{
    if (descriptor.IsChanged(Fields::Text))
    {
        DAVA::String stringValue = GetFieldValue<DAVA::String>(Fields::Text, DAVA::String());
        QString v = QString::fromStdString(stringValue);
        setText(v);
        setToolTip(v);
    }

    if (descriptor.IsChanged(Fields::IsVisible))
    {
        bool value = GetFieldValue<bool>(Fields::IsVisible, isVisible());
        setVisible(value);
    }
}
} // namespace DAVA
