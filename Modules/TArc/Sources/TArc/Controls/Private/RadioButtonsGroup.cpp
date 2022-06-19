#include "TArc/Controls/RadioButtonsGroup.h"
#include "TArc/DataProcessing/AnyQMetaType.h"

#include <Base/FastName.h>

#include <QGridLayout>
#include <QVariant>
#include <QRadioButton>
#include <QLabel>

namespace RadioButtonsGroupDetails
{
const char* dataPropertyName = "data";
}

namespace DAVA
{
RadioButtonsGroup::RadioButtonsGroup(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QWidget>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

RadioButtonsGroup::RadioButtonsGroup(const Params& params, Reflection model, QWidget* parent)
    : ControlProxyImpl<QWidget>(params, ControlDescriptor(params.fields), params.accessor, model, parent)
{
    SetupControl();
}

void RadioButtonsGroup::SetupControl()
{
    connections.AddConnection(&buttonGroup, static_cast<void (QButtonGroup::*)(QAbstractButton*)>(&QButtonGroup::buttonClicked), MakeFunction(this, &RadioButtonsGroup::OnButtonToggled));
}

void RadioButtonsGroup::UpdateControl(const ControlDescriptor& changedFields)
{
    DVASSERT(updateControlProceed == false);
    ScopedValueGuard<bool> guard(updateControlProceed, true);

    bool enabledChanged = changedFields.IsChanged(Fields::Enabled);
    bool valueChanged = changedFields.IsChanged(Fields::Value);
    if (enabledChanged || valueChanged)
    {
        bool enabled = IsValueEnabled(changedFields, Fields::Value, Fields::Enabled);
        setEnabled(enabled);
    }

    Reflection fieldEnumerator;
    const FastName& enumeratorName = changedFields.GetName(Fields::Enumerator);
    if (enumeratorName.IsValid())
    {
        fieldEnumerator = model.GetField(enumeratorName);
    }

    Qt::Orientation orientation = Qt::Horizontal;
    const FastName& orientationName = changedFields.GetName(Fields::Orientation);
    if (orientationName.IsValid())
    {
        Any orientationValue = model.GetField(orientationName).GetValue();
        if (orientationValue.CanCast<Qt::Orientation>())
        {
            orientation = orientationValue.Cast<Qt::Orientation>();
        }
    }

    Reflection fieldValue = model.GetField(changedFields.GetName(Fields::Value));
    DVASSERT(fieldValue.IsValid());
    if (buttonGroup.buttons().isEmpty() || changedFields.IsChanged(Fields::Enumerator) || changedFields.IsChanged(Fields::Orientation))
    {
        CreateItems(fieldValue, fieldEnumerator, orientation);
    }

    int currentIndex = SelectCurrentItem(fieldValue, fieldEnumerator);
    DVASSERT(currentIndex != -1);
}

int RadioButtonsGroup::SelectCurrentItem(const Reflection& fieldValue, const Reflection& fieldEnumerator)
{
    Any value = fieldValue.GetValue();
    if (value.IsEmpty() == false)
    {
        QList<QAbstractButton*> radioButtons = buttonGroup.buttons();
        for (int i = 0, count = radioButtons.size(); i < count; ++i)
        {
            QAbstractButton* button = radioButtons.at(i);
            Any iAny = button->property(RadioButtonsGroupDetails::dataPropertyName).value<Any>();
            if (value == iAny)
            {
                button->setChecked(true);
                return i;
            }
            else if (iAny.CanCast<int>() && value.CanCast<int>() &&
                     iAny.Cast<int>() == value.Cast<int>())
            {
                button->setChecked(true);
                return i;
            }
        }
    }

    return -1;
}

void RadioButtonsGroup::CreateItems(const Reflection& fieldValue, const Reflection& fieldEnumerator, Qt::Orientation orientation)
{
    delete layout();
    QGridLayout* newLayout = new QGridLayout();
    newLayout->setVerticalSpacing(10);
    setLayout(newLayout);

    const M::Enum* enumMeta = fieldValue.GetMeta<M::Enum>();
    if (enumMeta != nullptr)
    {
        const EnumMap* enumMap = enumMeta->GetEnumMap();
        for (size_t i = 0, count = enumMap->GetCount(); i < count; ++i)
        {
            int iValue = 0;
            bool ok = enumMap->GetValue(i, iValue);
            if (ok)
            {
                QVariant dataValue;
                dataValue.setValue(Any(iValue));

                CreateItem(enumMap->ToString(iValue), dataValue, orientation, static_cast<int32>(i));
            }
            else
            {
                DVASSERT(false);
            }
        }
    }
    else
    {
        DVASSERT(fieldEnumerator.IsValid() == true);

        Vector<Reflection::Field> fields = fieldEnumerator.GetFields();
        for (size_t i = 0, count = fields.size(); i < count; ++i)
        {
            const Reflection::Field& field = fields.at(i);
            Any fieldDescr = field.ref.GetValue();

            QVariant dataValue;
            dataValue.setValue(field.key);

            CreateItem(fieldDescr.Cast<String>().c_str(), dataValue, orientation, static_cast<int32>(i));
        }
    }
}

void RadioButtonsGroup::CreateItem(const String& text, const QVariant& data, Qt::Orientation orientation, int32 index)
{
    QRadioButton* radioButton = new QRadioButton();
    radioButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    radioButton->setProperty(RadioButtonsGroupDetails::dataPropertyName, data);
    buttonGroup.addButton(radioButton);

    QLabel* label = new QLabel(QString::fromStdString(text));
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QGridLayout* currentLayout = qobject_cast<QGridLayout*>(layout());
    DVASSERT(currentLayout != nullptr);

    if (orientation == Qt::Horizontal)
    {
        currentLayout->addWidget(radioButton, 0, index, 1, 1, Qt::AlignCenter);
        currentLayout->addWidget(label, 1, index, 1, 1, Qt::AlignCenter);
    }
    else
    {
        currentLayout->addWidget(radioButton, index, 0);
        currentLayout->addWidget(label, index, 1);
    }
}

void RadioButtonsGroup::OnButtonToggled(QAbstractButton* button)
{
    if (updateControlProceed)
    {
        // ignore reaction on control initialization
        return;
    }

    wrapper.SetFieldValue(GetFieldName(Fields::Value), button->property(RadioButtonsGroupDetails::dataPropertyName).value<Any>());
}
} // namespace DAVA
