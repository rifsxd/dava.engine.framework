#include "TArc/Controls/PropertyPanel/Private/MultiIntSpinBox.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Controls/QtWrapLayout.h"
#include "TArc/Controls/IntSpinBox.h"
#include "TArc/Controls/ControlDescriptor.h"

#include <Debug/DVAssert.h>

#include <QLabel>
#include <QWidget>

namespace DAVA
{
namespace MultiIntSpinBoxDetails
{
template <typename TEnum>
void ApplyRole(ControlDescriptorBuilder<TEnum>& descriptor, TEnum role, const String& name)
{
    if (name.empty() == false)
    {
        descriptor[role] = name;
    }
}

template <typename T>
QWidget* CreatedEditor(const Reflection& r, const MultiIntSpinBox::FieldDescriptor& fieldDescr, QWidget* parent, T* accessor,
                       const MultiIntSpinBox::BaseParams& controlParams, Reflection& model, Vector<ControlProxy*>& subControls)
{
    QWidget* result = new QWidget(parent);
    QtHBoxLayout* layout = new QtHBoxLayout(result);
    layout->setMargin(2);
    layout->setSpacing(2);

    QLabel* label = new QLabel(QString::fromStdString(fieldDescr.valueRole + ":"), result);
    layout->addWidget(label);

    IntSpinBox::Params params(controlParams.accessor, controlParams.ui, controlParams.wndKey);
    params.fields[IntSpinBox::Fields::Value] = fieldDescr.valueRole;
    ApplyRole(params.fields, IntSpinBox::Fields::Range, fieldDescr.rangeRole);
    ApplyRole(params.fields, IntSpinBox::Fields::ShowSpinArrows, fieldDescr.showSpinArrowsRole);
    ApplyRole(params.fields, IntSpinBox::Fields::IsReadOnly, fieldDescr.readOnlyRole);
    IntSpinBox* spinBox = new IntSpinBox(params, accessor, model, result);
    subControls.push_back(spinBox);
    layout->AddControl(spinBox);

    QWidget* editor = spinBox->ToWidgetCast();
    QSizePolicy policy = editor->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    editor->setSizePolicy(policy);

    return result;
}
}

MultiIntSpinBox::MultiIntSpinBox(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent /*= nullptr*/)
    : TBase(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl(wrappersProcessor);
}

MultiIntSpinBox::MultiIntSpinBox(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent /*= nullptr*/)
    : TBase(params, ControlDescriptor(params.fields), accessor, model, parent)
{
    SetupControl(accessor);
}

template <typename T>
void MultiIntSpinBox::SetupControl(T* accessor)
{
    DVASSERT(layout() == nullptr);
    Reflection r = model.GetField(GetFieldName(Fields::FieldsList));
    DVASSERT(r.IsValid());

    Vector<Reflection::Field> fields = r.GetFields();
    if (fields.empty())
    {
        return;
    }

    QtWrapLayout* layout = new QtWrapLayout(this);
    layout->setMargin(2);
    layout->SetHorizontalSpacing(2);
    layout->SetVerticalSpacing(2);
    for (Reflection::Field& f : fields)
    {
        Any fieldValue = f.ref.GetValue();
        DVASSERT(fieldValue.CanGet<FieldDescriptor>());
        FieldDescriptor fieldDescr = fieldValue.Get<FieldDescriptor>();

        Reflection editableField = model.GetField(fieldDescr.valueRole);
        DVASSERT(editableField.IsValid());
        layout->addWidget(MultiIntSpinBoxDetails::CreatedEditor(editableField, fieldDescr, this, accessor, controlParams, model, subControls));
    }
}

void MultiIntSpinBox::ForceUpdate()
{
    for_each(subControls.begin(), subControls.end(), [](ControlProxy* proxy)
             {
                 proxy->ForceUpdate();
             });

    TBase::ForceUpdate();
}

void MultiIntSpinBox::TearDown()
{
    for_each(subControls.begin(), subControls.end(), [](ControlProxy* proxy)
             {
                 proxy->TearDown();
             });

    TBase::TearDown();
}

void MultiIntSpinBox::UpdateControl(const ControlDescriptor& descriptor)
{
}

bool MultiIntSpinBox::FieldDescriptor::operator==(const FieldDescriptor& other) const
{
    return valueRole == other.valueRole &&
    readOnlyRole == other.readOnlyRole &&
    rangeRole == other.rangeRole &&
    showSpinArrowsRole == other.showSpinArrowsRole;
}
} // namespace DAVA
