#pragma once

#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/MultiDoubleSpinBox.h"
#include "TArc/Controls/PropertyPanel/Private/MultiIntSpinBox.h"

#include <Reflection/ReflectedMeta.h>
#include <Math/Vector.h>
#include <Math/Color.h>
#include <Math/Rect.h>
#include <Math/AABBox3.h>
#include <Base/Vector.h>

namespace DAVA
{
namespace KDComponentValueTraits
{
template <typename T, typename TComponent, int32 Index>
TComponent GetAxisValue(const T& v)
{
    DVASSERT(false);
    return 0.0;
}

template <typename T, typename TComponent, int32 Index>
void SetAxisValue(T& v, TComponent component)
{
    DVASSERT(false);
}

template <typename T, typename TEditor>
void InitFieldsList(Vector<typename TEditor::FieldDescriptor>& fields)
{
    DVASSERT(false);
}

template <typename T, typename TComponent>
void InitRanges(const Vector<std::shared_ptr<PropertyNode>>& nodes, Array<std::unique_ptr<M::Range>, 6>& ranges)
{
    DVASSERT(false);
}

/////////////////////////////////////////////////////////////////////////////////////
//                        Vector2 traits                                           //
/////////////////////////////////////////////////////////////////////////////////////
template <>
float32 GetAxisValue<Vector2, float32, 0>(const Vector2& v)
{
    return v.x;
}

template <>
float32 GetAxisValue<Vector2, float32, 1>(const Vector2& v)
{
    return v.y;
}

template <>
void SetAxisValue<Vector2, float32, 0>(Vector2& v, float32 component)
{
    v.x = component;
}

template <>
void SetAxisValue<Vector2, float32, 1>(Vector2& v, float32 component)
{
    v.y = component;
}

template <>
void InitFieldsList<Vector2, MultiDoubleSpinBox>(Vector<MultiDoubleSpinBox::FieldDescriptor>& fields)
{
    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "X";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "xRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "Y";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "yRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }
}

template <>
void InitRanges<Vector2, float32>(const Vector<std::shared_ptr<PropertyNode>>& nodes, Array<std::unique_ptr<M::Range>, 6>& ranges)
{
}

/////////////////////////////////////////////////////////////////////////////////////
//                        Vector3 traits                                           //
/////////////////////////////////////////////////////////////////////////////////////
template <>
float32 GetAxisValue<Vector3, float32, 0>(const Vector3& v)
{
    return v.x;
}

template <>
float32 GetAxisValue<Vector3, float32, 1>(const Vector3& v)
{
    return v.y;
}

template <>
float32 GetAxisValue<Vector3, float32, 2>(const Vector3& v)
{
    return v.z;
}

template <>
void SetAxisValue<Vector3, float32, 0>(Vector3& v, float32 component)
{
    v.x = component;
}

template <>
void SetAxisValue<Vector3, float32, 1>(Vector3& v, float32 component)
{
    v.y = component;
}

template <>
void SetAxisValue<Vector3, float32, 2>(Vector3& v, float32 component)
{
    v.z = component;
}

template <>
void InitFieldsList<Vector3, MultiDoubleSpinBox>(Vector<MultiDoubleSpinBox::FieldDescriptor>& fields)
{
    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "X";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "xRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "Y";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "yRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "Z";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "zRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }
}

template <>
void InitRanges<Vector3, float32>(const Vector<std::shared_ptr<PropertyNode>>& nodes, Array<std::unique_ptr<M::Range>, 6>& ranges)
{
    std::shared_ptr<PropertyNode> node = nodes.front();
    const M::Range* rangeMeta = node->field.ref.GetMeta<M::Range>();
    if (rangeMeta == nullptr)
    {
        return;
    }

    Array<Any, 3> minRange;
    Array<Any, 3> maxRange;
    Array<Any, 3> step;

    if (rangeMeta->minValue.CanCast<Vector3>())
    {
        Vector3 minVector = rangeMeta->minValue.Cast<Vector3>();
        minRange[0] = minVector.x;
        minRange[1] = minVector.y;
        minRange[2] = minVector.z;
    }

    if (rangeMeta->maxValue.CanCast<Vector3>())
    {
        Vector3 maxVector = rangeMeta->maxValue.Cast<Vector3>();
        maxRange[0] = maxVector.x;
        maxRange[1] = maxVector.y;
        maxRange[2] = maxVector.z;
    }

    if (rangeMeta->step.CanCast<Vector3>())
    {
        Vector3 stepVector = rangeMeta->step.Cast<Vector3>();
        step[0] = stepVector.x;
        step[1] = stepVector.y;
        step[2] = stepVector.z;
    }

    ranges[0].reset(new M::Range(minRange[0], maxRange[0], step[0]));
    ranges[1].reset(new M::Range(minRange[1], maxRange[1], step[1]));
    ranges[2].reset(new M::Range(minRange[2], maxRange[2], step[2]));
}

/////////////////////////////////////////////////////////////////////////////////////
//                        Vector4 traits                                           //
/////////////////////////////////////////////////////////////////////////////////////
template <>
float32 GetAxisValue<Vector4, float32, 0>(const Vector4& v)
{
    return v.x;
}

template <>
float32 GetAxisValue<Vector4, float32, 1>(const Vector4& v)
{
    return v.y;
}

template <>
float32 GetAxisValue<Vector4, float32, 2>(const Vector4& v)
{
    return v.z;
}

template <>
float32 GetAxisValue<Vector4, float32, 3>(const Vector4& v)
{
    return v.w;
}

template <>
void SetAxisValue<Vector4, float32, 0>(Vector4& v, float32 component)
{
    v.x = component;
}

template <>
void SetAxisValue<Vector4, float32, 1>(Vector4& v, float32 component)
{
    v.y = component;
}

template <>
void SetAxisValue<Vector4, float32, 2>(Vector4& v, float32 component)
{
    v.z = component;
}

template <>
void SetAxisValue<Vector4, float32, 3>(Vector4& v, float32 component)
{
    v.w = component;
}

template <>
void InitFieldsList<Vector4, MultiDoubleSpinBox>(Vector<MultiDoubleSpinBox::FieldDescriptor>& fields)
{
    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "X";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "xRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "Y";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "yRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "Z";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "zRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "W";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "wRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }
}

template <>
void InitRanges<Vector4, float32>(const Vector<std::shared_ptr<PropertyNode>>& nodes, Array<std::unique_ptr<M::Range>, 6>& ranges)
{
}

/////////////////////////////////////////////////////////////////////////////////////
//                           Rect traits                                           //
/////////////////////////////////////////////////////////////////////////////////////
template <>
float32 GetAxisValue<Rect, float32, 0>(const Rect& v)
{
    return v.x;
}

template <>
float32 GetAxisValue<Rect, float32, 1>(const Rect& v)
{
    return v.y;
}

template <>
float32 GetAxisValue<Rect, float32, 2>(const Rect& v)
{
    return v.dx;
}

template <>
float32 GetAxisValue<Rect, float32, 3>(const Rect& v)
{
    return v.dy;
}

template <>
void SetAxisValue<Rect, float32, 0>(Rect& v, float32 component)
{
    v.x = component;
}

template <>
void SetAxisValue<Rect, float32, 1>(Rect& v, float32 component)
{
    v.y = component;
}

template <>
void SetAxisValue<Rect, float32, 2>(Rect& v, float32 component)
{
    v.dx = component;
}

template <>
void SetAxisValue<Rect, float32, 3>(Rect& v, float32 component)
{
    v.dy = component;
}

template <>
void InitFieldsList<Rect, MultiDoubleSpinBox>(Vector<MultiDoubleSpinBox::FieldDescriptor>& fields)
{
    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "X";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "xRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "Y";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "yRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "Width";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "widthRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "Height";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "heightRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }
}

template <>
void InitRanges<Rect, float32>(const Vector<std::shared_ptr<PropertyNode>>& nodes, Array<std::unique_ptr<M::Range>, 6>& ranges)
{
}

/////////////////////////////////////////////////////////////////////////////////////
//                           Color traits                                           //
/////////////////////////////////////////////////////////////////////////////////////
template <>
float32 GetAxisValue<Color, float32, 0>(const Color& v)
{
    return v.r;
}

template <>
float32 GetAxisValue<Color, float32, 1>(const Color& v)
{
    return v.g;
}

template <>
float32 GetAxisValue<Color, float32, 2>(const Color& v)
{
    return v.b;
}

template <>
float32 GetAxisValue<Color, float32, 3>(const Color& v)
{
    return v.a;
}

template <>
void SetAxisValue<Color, float32, 0>(Color& v, float32 component)
{
    v.r = component;
}

template <>
void SetAxisValue<Color, float32, 1>(Color& v, float32 component)
{
    v.g = component;
}

template <>
void SetAxisValue<Color, float32, 2>(Color& v, float32 component)
{
    v.b = component;
}

template <>
void SetAxisValue<Color, float32, 3>(Color& v, float32 component)
{
    v.a = component;
}

template <>
void InitFieldsList<Color, MultiDoubleSpinBox>(Vector<MultiDoubleSpinBox::FieldDescriptor>& fields)
{
    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "R";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "rRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "G";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "gRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "B";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "bRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "A";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "aRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }
}

template <>
void InitRanges<Color, float32>(const Vector<std::shared_ptr<PropertyNode>>& nodes, Array<std::unique_ptr<M::Range>, 6>& ranges)
{
}

template <>
uint32 GetAxisValue<Color, uint32, 0>(const Color& v)
{
    return static_cast<uint32>(255 * v.r);
}

template <>
uint32 GetAxisValue<Color, uint32, 1>(const Color& v)
{
    return static_cast<uint32>(v.g);
}

template <>
uint32 GetAxisValue<Color, uint32, 2>(const Color& v)
{
    return static_cast<uint32>(v.b);
}

template <>
uint32 GetAxisValue<Color, uint32, 3>(const Color& v)
{
    return static_cast<uint32>(v.a);
}

template <>
void SetAxisValue<Color, uint32, 0>(Color& v, uint32 component)
{
    v.r = static_cast<float32>(component) / 255.0f;
}

template <>
void SetAxisValue<Color, uint32, 1>(Color& v, uint32 component)
{
    v.g = static_cast<float32>(component) / 255.0f;
}

template <>
void SetAxisValue<Color, uint32, 2>(Color& v, uint32 component)
{
    v.b = static_cast<float32>(component) / 255.0f;
}

template <>
void SetAxisValue<Color, uint32, 3>(Color& v, uint32 component)
{
    v.a = static_cast<float32>(component) / 255.0f;
}

template <>
void InitFieldsList<Color, MultiIntSpinBox>(Vector<MultiIntSpinBox::FieldDescriptor>& fields)
{
    {
        MultiIntSpinBox::FieldDescriptor descr;
        descr.valueRole = "R";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "rRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiIntSpinBox::FieldDescriptor descr;
        descr.valueRole = "G";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "gRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiIntSpinBox::FieldDescriptor descr;
        descr.valueRole = "B";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "bRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiIntSpinBox::FieldDescriptor descr;
        descr.valueRole = "A";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "aRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }
}

template <>
void InitRanges<Color, uint32>(const Vector<std::shared_ptr<PropertyNode>>& nodes, Array<std::unique_ptr<M::Range>, 6>& ranges)
{
    ranges[0] = std::make_unique<M::Range>(0u, 255u, 1u);
    ranges[1] = std::make_unique<M::Range>(0u, 255u, 1u);
    ranges[2] = std::make_unique<M::Range>(0u, 255u, 1u);
    ranges[3] = std::make_unique<M::Range>(0u, 255u, 1u);
}

/////////////////////////////////////////////////////////////////////////////////////
//                         AABBox3 traits                                          //
/////////////////////////////////////////////////////////////////////////////////////
template <>
float32 GetAxisValue<AABBox3, float32, 0>(const AABBox3& v)
{
    return v.min.x;
}

template <>
float32 GetAxisValue<AABBox3, float32, 1>(const AABBox3& v)
{
    return v.min.y;
}

template <>
float32 GetAxisValue<AABBox3, float32, 2>(const AABBox3& v)
{
    return v.min.z;
}

template <>
float32 GetAxisValue<AABBox3, float32, 3>(const AABBox3& v)
{
    return v.max.x;
}

template <>
float32 GetAxisValue<AABBox3, float32, 4>(const AABBox3& v)
{
    return v.max.y;
}

template <>
float32 GetAxisValue<AABBox3, float32, 5>(const AABBox3& v)
{
    return v.max.z;
}

template <>
void SetAxisValue<AABBox3, float32, 0>(AABBox3& v, float32 component)
{
    v.min.x = component;
}

template <>
void SetAxisValue<AABBox3, float32, 1>(AABBox3& v, float32 component)
{
    v.min.y = component;
}

template <>
void SetAxisValue<AABBox3, float32, 2>(AABBox3& v, float32 component)
{
    v.min.z = component;
}

template <>
void SetAxisValue<AABBox3, float32, 3>(AABBox3& v, float32 component)
{
    v.max.x = component;
}

template <>
void SetAxisValue<AABBox3, float32, 4>(AABBox3& v, float32 component)
{
    v.max.y = component;
}

template <>
void SetAxisValue<AABBox3, float32, 5>(AABBox3& v, float32 component)
{
    v.max.z = component;
}

template <>
void InitFieldsList<AABBox3, MultiDoubleSpinBox>(Vector<MultiDoubleSpinBox::FieldDescriptor>& fields)
{
    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "Min X";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "minXRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "Min Y";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "minYRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "Min Z";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "minZRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "Max X";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "maxXRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "Max Y";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "maxYRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }

    {
        MultiDoubleSpinBox::FieldDescriptor descr;
        descr.valueRole = "Max Z";
        descr.accuracyRole = "accuracy";
        descr.readOnlyRole = BaseComponentValue::readOnlyFieldName;
        descr.rangeRole = "maxZRange";
        descr.showSpinArrowsRole = "showSpinArrows";
        fields.push_back(descr);
    }
}

template <>
void InitRanges<AABBox3, float32>(const Vector<std::shared_ptr<PropertyNode>>& nodes, Array<std::unique_ptr<M::Range>, 6>& ranges)
{
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//                                  Common functions                                            //
//////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, typename TComponent, int32 Index>
static Any GetNodesAxisValue(const Vector<std::shared_ptr<PropertyNode>>& nodes)
{
    TComponent v = GetAxisValue<T, TComponent, Index>(nodes.front()->cachedValue.Cast<T>());
    for (const std::shared_ptr<const PropertyNode>& node : nodes)
    {
        if (v != GetAxisValue<T, TComponent, Index>(node->cachedValue.Cast<T>()))
        {
            return Any();
        }
    }

    return v;
}

template <typename T, typename TComponent, int32 Index>
static void SetNodesAxisValue(const Vector<std::shared_ptr<PropertyNode>>& nodes, TComponent v, ModifyExtension::MultiCommandInterface& cmdInterface)
{
    for (const std::shared_ptr<PropertyNode>& node : nodes)
    {
        T currentValue = node->cachedValue.Cast<T>();
        if (GetAxisValue<T, TComponent, Index>(currentValue) != v)
        {
            SetAxisValue<T, TComponent, Index>(currentValue, v);
            cmdInterface.ModifyPropertyValue(node, currentValue);
        }
    }
}

} // namespace KDComponentValueTraits
} // namespace DAVA
