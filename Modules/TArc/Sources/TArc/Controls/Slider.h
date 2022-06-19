#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Utils/QtConnections.h"

#include <Base/BaseTypes.h>

#include <QSlider>

namespace DAVA
{
class Slider : public ControlProxyImpl<QSlider>
{
    using TBase = ControlProxyImpl<QSlider>;

public:
    enum class Fields : uint32
    {
        Enabled,
        Range, // const DAVA::M::Range*
        Value, // int
        Orientation, // Qt::Orientation
        ImmediateValue, // Method<void(int)>
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    Slider(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    Slider(const Params& params, Reflection model, QWidget* parent = nullptr);
    ~Slider();

protected:
    bool event(QEvent* e) override;

private:
    void SetupControl();
    void UpdateControl(const ControlDescriptor& descriptor) override;
    bool UpdateRange();

    void UnsureMapperCreated();
    template <typename T>
    bool CheckTypeAndCreateMapper(const DAVA::Type* t);

    void OnValuedChanged(int value);
    void OnSliderPressed();
    void OnSliderReleased();
    void SetImmediateValue(const Any& v);

    class ValueMapper;
    template <typename T>
    class TValueMapper;
    class DisabledMapper;

    ValueMapper* mapper = nullptr;
    Any cachedValue;

    QtConnections connections;
};
} // namespace DAVA
