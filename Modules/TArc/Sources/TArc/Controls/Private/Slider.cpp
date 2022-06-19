#include "TArc/Controls/Slider.h"

#include <QMouseEvent>

namespace DAVA
{
class Slider::ValueMapper
{
public:
    virtual ~ValueMapper() = default;

    virtual bool IsDisabledMapper() const
    {
        return false;
    }
    virtual bool ReinitRange(const M::Range* range) = 0;

    virtual bool IsValidValue(const Any& v) const = 0;
    virtual int MapValue(const Any& v) const = 0;
    virtual Any MapValue(int sliderValue) const = 0;
    virtual int GetSliderMin() const = 0;
    virtual int GetSliderMax() const = 0;
    virtual int GetSliderStep() const = 0;
};

class Slider::DisabledMapper : public Slider::ValueMapper
{
public:
    bool ReinitRange(const M::Range* range) override
    {
        return true;
    };

    bool IsDisabledMapper() const override
    {
        return true;
    }

    bool IsValidValue(const Any& v) const override
    {
        return false;
    }

    int MapValue(const Any& v) const override
    {
        return 0;
    }

    Any MapValue(int sliderValue) const override
    {
        return Any();
    }
    int GetSliderMin() const override
    {
        return 0;
    }

    int GetSliderMax() const override
    {
        return 0;
    }

    int GetSliderStep() const override
    {
        return 0;
    }
};

template <typename T>
class Slider::TValueMapper : public Slider::ValueMapper
{
public:
    TValueMapper()
    {
        InitRange(static_cast<T>(0), static_cast<T>(100), static_cast<T>(1));
    }

    bool InitRange(T min, T max, T step_)
    {
        if (min == srcMin && max == srcMax && step == step_)
        {
            return false;
        }

        srcMin = min;
        srcMax = max;
        step = step_;
        DAVA::int64 valuesCount = static_cast<DAVA::int64>((max - min) / step);
        DAVA::int64 halfValuesCount = valuesCount >> 1;
        if (halfValuesCount > static_cast<DAVA::int64>(std::numeric_limits<int>::max()))
        {
            halfValuesCount = std::numeric_limits<int>::max();
        }

        minValue = -halfValuesCount;
        maxValue = halfValuesCount;
        if ((valuesCount & 0x1) != 0)
        {
            maxValue += 1;
        }
        return true;
    }

    bool ReinitRange(const M::Range* range) override
    {
        T min = std::numeric_limits<T>::lowest();
        T max = std::numeric_limits<T>::max();
        T step = static_cast<T>(1.0f);
        if (range != nullptr)
        {
            min = range->minValue.Cast<T>(min);
            max = range->maxValue.Cast<T>(max);
            step = range->step.Cast<T>(step);
        }

        return InitRange(min, max, step);
    }

    bool IsValidValue(const Any& v) const override
    {
        return v.CanCast<T>();
    }

    int MapValue(const Any& v) const override
    {
        T value = v.Cast<T>();
        T sliderRange = static_cast<T>(maxValue - minValue);
        T srcRange = srcMax - srcMin;

        return (((value - srcMin) * sliderRange) / srcRange) + minValue;
    }

    DAVA::Any MapValue(int sliderValue) const override
    {
        T stepsCount = static_cast<T>(sliderValue - minValue);
        return (stepsCount * step + srcMin);
    }

    int GetSliderMin() const override
    {
        return minValue;
    }

    int GetSliderMax() const override
    {
        return maxValue;
    }
    int GetSliderStep() const override
    {
        return 1;
    }

private:
    int minValue;
    int maxValue;
    T step;
    T srcMin;
    T srcMax;
};

Slider::Slider(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : TBase(params, params.fields, wrappersProcessor, model, parent)
{
    SetupControl();
}

Slider::Slider(const Params& params, Reflection model, QWidget* parent)
    : TBase(params, params.fields, model, parent)
{
    SetupControl();
}

Slider::~Slider()
{
    delete mapper;
}

void Slider::SetupControl()
{
    setOrientation(Qt::Horizontal);
    setTracking(true);
    setMouseTracking(true);
    connections.AddConnection(this, &QSlider::valueChanged, MakeFunction(this, &Slider::OnValuedChanged));
    connections.AddConnection(this, &QSlider::sliderPressed, MakeFunction(this, &Slider::OnSliderPressed));
    connections.AddConnection(this, &QSlider::sliderReleased, MakeFunction(this, &Slider::OnSliderReleased));
}

void Slider::UpdateControl(const ControlDescriptor& descriptor)
{
    RETURN_IF_MODEL_LOST(void());
    UnsureMapperCreated();

    if (descriptor.IsChanged(Fields::Orientation))
    {
        setOrientation(GetFieldValue(Fields::Orientation, Qt::Horizontal));
    }

    bool rangeUpdated = false;
    if (descriptor.IsChanged(Fields::Range) || descriptor.IsChanged(Fields::Value))
    {
        rangeUpdated = UpdateRange();
    }

    if (descriptor.IsChanged(Fields::Enabled) || descriptor.IsChanged(Fields::Value) || rangeUpdated == true)
    {
        Any v = GetFieldValue<Any>(Fields::Value, Any());
        if (mapper->IsValidValue(v))
        {
            setValue(mapper->MapValue(v));
            setEnabled(GetFieldValue<bool>(Fields::Enabled, true) && !IsReadOnlyConstOfMeta(Fields::Value));
        }
        else
        {
            setEnabled(false);
            setValue(0);
        }
    }
}

bool Slider::UpdateRange()
{
    int minV = std::numeric_limits<int>::lowest();
    int maxV = std::numeric_limits<int>::max();
    int valueStep = 1;

    Reflection valueField = model.GetField(this->GetFieldName(Fields::Value));
    DVASSERT(valueField.IsValid());

    const M::Range* rangeMeta = nullptr;
    FastName rangeFieldName = GetFieldName(Fields::Range);
    if (rangeFieldName.IsValid())
    {
        rangeMeta = GetFieldValue<const M::Range*>(Fields::Range, nullptr);
    }

    if (rangeMeta == nullptr)
    {
        rangeMeta = valueField.GetMeta<M::Range>();
    }

    DVASSERT(mapper != nullptr);
    bool reinited = mapper->ReinitRange(rangeMeta);

    minV = mapper->GetSliderMin();
    maxV = mapper->GetSliderMax();
    valueStep = mapper->GetSliderStep();

    if (minV != minimum() || maxV != maximum())
    {
        setRange(minV, maxV);
    }

    if (valueStep != singleStep())
    {
        setSingleStep(valueStep);
    }

    if (valueStep != pageStep())
    {
        setPageStep(valueStep);
    }

    return reinited;
}

void Slider::UnsureMapperCreated()
{
    if (mapper == nullptr || mapper->IsDisabledMapper())
    {
        FastName valueFieldName = GetFieldName(Fields::Value);
        DVASSERT(valueFieldName.IsValid());

        Reflection valueField = model.GetField(valueFieldName);
        const Type* vType = valueField.GetValue().GetType();
        if (CheckTypeAndCreateMapper<float32>(vType) ||
            CheckTypeAndCreateMapper<float64>(vType) ||
            CheckTypeAndCreateMapper<int8>(vType) ||
            CheckTypeAndCreateMapper<uint8>(vType) ||
            CheckTypeAndCreateMapper<int16>(vType) ||
            CheckTypeAndCreateMapper<uint16>(vType) ||
            CheckTypeAndCreateMapper<int32>(vType) ||
            CheckTypeAndCreateMapper<uint32>(vType))
        {
        }
        else
        {
            const Type* vFieldType = valueField.GetValueType();
            if (CheckTypeAndCreateMapper<float32>(vFieldType) ||
                CheckTypeAndCreateMapper<float64>(vFieldType) ||
                CheckTypeAndCreateMapper<int8>(vFieldType) ||
                CheckTypeAndCreateMapper<uint8>(vFieldType) ||
                CheckTypeAndCreateMapper<int16>(vFieldType) ||
                CheckTypeAndCreateMapper<uint16>(vFieldType) ||
                CheckTypeAndCreateMapper<int32>(vFieldType) ||
                CheckTypeAndCreateMapper<uint32>(vFieldType))
            {
            }
            else
            {
                if (mapper == nullptr)
                {
                    mapper = new DisabledMapper();
                }
            }
        }
    }
}

template <typename T>
bool Slider::CheckTypeAndCreateMapper(const DAVA::Type* t)
{
    if (t == DAVA::Type::Instance<T>())
    {
        if (mapper != nullptr)
        {
            DVASSERT(mapper->IsDisabledMapper() == true);
            delete mapper;
        }
        mapper = new TValueMapper<T>();
        return true;
    }

    return false;
}

bool Slider::event(QEvent* e)
{
    if (e->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* ev = static_cast<QMouseEvent*>(e);
        if (ev->button() == Qt::LeftButton)
        {
            grabMouse();
        }
    }
    else if (e->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* ev = static_cast<QMouseEvent*>(e);
        if (ev->button() == Qt::LeftButton)
        {
            releaseMouse();
        }
    }

    return TBase::event(e);
}

void Slider::OnValuedChanged(int value)
{
    DVASSERT(mapper != nullptr);
    if (isEnabled() == false)
    {
        return;
    }

    if (isSliderDown())
    {
        SetImmediateValue(mapper->MapValue(value));
    }
    else
    {
        OnSliderReleased();
    }
}

void Slider::OnSliderPressed()
{
    cachedValue = GetFieldValue<Any>(Fields::Value, Any());
}

void Slider::OnSliderReleased()
{
    DVASSERT(mapper != nullptr);
    if (isEnabled() == false)
    {
        return;
    }

    Any newValue = mapper->MapValue(value());
    Any currentValue = GetFieldValue<Any>(Fields::Value, Any());
    Any cachedCopy = cachedValue;
    if (cachedCopy.IsEmpty() == true)
    {
        cachedCopy = currentValue;
    }
    if (cachedCopy != newValue || currentValue != newValue)
    {
        SetImmediateValue(cachedValue);
        SetFieldValue(Fields::Value, newValue);
        cachedValue = Any();
    }
}

void Slider::SetImmediateValue(const Any& v)
{
    FastName immediateValueName = GetFieldName(Fields::ImmediateValue);
    if (immediateValueName.IsValid() == true)
    {
        AnyFn fn = model.GetMethod(immediateValueName.c_str());
        DVASSERT(fn.IsValid());
        fn.InvokeWithCast(v);
    }
}
} // namespace DAVA
