#pragma once
#include "TArc/Controls/ControlProxy.h"

#include <Base/BaseTypes.h>

#include <QtEvents>

namespace DAVA
{
class MultiDoubleSpinBox : public ControlProxyImpl<QWidget>
{
    using TBase = ControlProxyImpl<QWidget>;

public:
    struct FieldDescriptor
    {
        String valueRole;
        String readOnlyRole;
        String rangeRole;
        String showSpinArrowsRole;
        String accuracyRole;

        bool operator==(const FieldDescriptor& other) const;
    };

    enum class Fields : uint32
    {
        FieldsList,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    MultiDoubleSpinBox(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    MultiDoubleSpinBox(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    template <typename T>
    void SetupControl(T* accessor);

    void ForceUpdate() override;
    void TearDown() override;

    void UpdateControl(const ControlDescriptor& descriptor) override;

    Vector<ControlProxy*> subControls;
};
} // namespace DAVA
