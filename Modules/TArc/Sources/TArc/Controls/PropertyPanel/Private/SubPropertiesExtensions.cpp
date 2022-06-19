#include "TArc/Controls/PropertyPanel/Private/SubPropertiesExtensions.h"
#include "TArc/Controls/PropertyPanel/Private/kDComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/MultiIntSpinBox.h"
#include "TArc/Controls/PropertyPanel/Private/MultiDoubleSpinBox.h"
#include "TArc/Controls/PropertyPanel/Private/TextComponentValue.h"
#include "TArc/Controls/ColorPicker/ColorPickerButton.h"
#include "TArc/Utils/StringFormatingUtils.h"
#include "TArc/Utils/Utils.h"

#include <Math/Vector.h>
#include <Math/Rect.h>
#include <Math/AABBox3.h>
#include <Math/Color.h>
#include <Base/BaseTypes.h>
#include <Utils/StringFormat.h>

#include <QHBoxLayout>

namespace DAVA
{
namespace SubPropertiesExtensionsDetail
{
FastName colorR = FastName("R");
FastName colorG = FastName("G");
FastName colorB = FastName("B");
FastName colorA = FastName("A");
FastName vectorX = FastName("X");
FastName vectorY = FastName("Y");
FastName vectorZ = FastName("Z");
FastName vectorW = FastName("W");
FastName rectX = FastName("X");
FastName rectY = FastName("Y");
FastName rectW = FastName("Width");
FastName rectH = FastName("Height");
FastName boxMinX = FastName("Min X");
FastName boxMinY = FastName("Min Y");
FastName boxMinZ = FastName("Min Z");
FastName boxMaxX = FastName("Max X");
FastName boxMaxY = FastName("Max Y");
FastName boxMaxZ = FastName("Max Z");

struct NotReadOnlyTraits
{
    static bool IsReadOnly()
    {
        return false;
    }
};

struct ColorTraits : public NotReadOnlyTraits
{
    using Type = Color;
    static int32 GetFieldIndex(const FastName& /*name*/)
    {
        return -1;
    }

    static String ToString(const Color& c, int32 /*fieldIndex*/, const Reflection& /*ref*/)
    {
        String r, g, b, a;
        FloatToString(c.r, 3, r);
        FloatToString(c.g, 3, g);
        FloatToString(c.b, 3, b);
        FloatToString(c.a, 3, a);
        return Format("[ %s, %s, %s, %s]", r.c_str(), g.c_str(), b.c_str(), a.c_str());
    }

    static uint32 GetMaxComponentCount()
    {
        return 4;
    }

    static Color CombineValue(const Any& v, const Color& /*prevValue*/, int32 /*fieldIndex*/)
    {
        const Vector<float32>& values = v.Get<Vector<float32>>();
        uint32 componentCount = static_cast<uint32>(values.size());
        Color c;
        if (componentCount > 0)
            c.r = values[0];
        if (componentCount > 1)
            c.g = values[1];
        if (componentCount > 2)
            c.b = values[2];
        if (componentCount == 4)
            c.a = values[3];

        return c;
    }

    static Any GetParseResult(const Vector<float32>& value)
    {
        if (value.empty())
        {
            Color c;
            Vector<float32> result = { c.r, c.g, c.b, c.a };
            return result;
        }
        return value;
    }

    static String GetParseErrorMessage()
    {
        return "Incorrect color format. Color format [ r; g; b; a]";
    }
};
struct Vector2Traits : public NotReadOnlyTraits
{
    using Type = Vector2;
    static int32 GetFieldIndex(const FastName& /*name*/)
    {
        return -1;
    }

    static String ToString(const Vector2& v, int32 /*fieldIndex*/, const Reflection& ref)
    {
        int32 accuracy = 6;
        const M::FloatNumberAccuracy* accuracyMeta = ref.GetMeta<M::FloatNumberAccuracy>();
        if (accuracyMeta != nullptr)
        {
            accuracy = accuracyMeta->accuracy;
        }

        String x, y;
        FloatToString(v.x, accuracy, x);
        FloatToString(v.y, accuracy, y);

        return Format("[ %s, %s]", x.c_str(), y.c_str());
    }

    static uint32 GetMaxComponentCount()
    {
        return 2;
    }

    static Vector2 CombineValue(const Any& v, const Vector2& /*prevValue*/, int32 /*fieldIndex*/)
    {
        const Vector<float32>& values = v.Get<Vector<float32>>();
        uint32 componentCount = static_cast<uint32>(values.size());
        Vector2 vec;
        if (componentCount > 0)
            vec.x = values[0];
        if (componentCount > 1)
            vec.y = values[1];

        return vec;
    }

    static Any GetParseResult(const Vector<float32>& value)
    {
        if (value.empty())
        {
            Vector2 vec;
            Vector<float32> result = { vec.x, vec.y };
            return result;
        }
        return value;
    }

    static String GetParseErrorMessage()
    {
        return "Incorrect vector2 format. Vector2 format [ x; y]";
    }
};
struct Vector3Traits : public NotReadOnlyTraits
{
    using Type = Vector3;
    static int32 GetFieldIndex(const FastName& /*name*/)
    {
        return -1;
    }

    static Vector3 GetDefaultValue()
    {
        return Vector3();
    }

    static String ToString(const Vector3& v, int32 /*fieldIndex*/, const Reflection& ref)
    {
        int32 accuracy = 6;
        const M::FloatNumberAccuracy* accuracyMeta = ref.GetMeta<M::FloatNumberAccuracy>();
        if (accuracyMeta != nullptr)
        {
            accuracy = accuracyMeta->accuracy;
        }

        String x, y, z;
        FloatToString(v.x, accuracy, x);
        FloatToString(v.y, accuracy, y);
        FloatToString(v.z, accuracy, z);
        return Format("[ %s, %s, %s]", x.c_str(), y.c_str(), z.c_str());
    }

    static uint32 GetMaxComponentCount()
    {
        return 3;
    }

    static Vector3 CombineValue(const Any& v, const Vector3& /*prevValue*/, int32 /*fieldIndex*/)
    {
        const Vector<float32>& values = v.Get<Vector<float32>>();
        uint32 componentCount = static_cast<uint32>(values.size());
        Vector3 vec;
        if (componentCount > 0)
            vec.x = values[0];
        if (componentCount > 1)
            vec.y = values[1];
        if (componentCount > 2)
            vec.z = values[2];

        return vec;
    }

    static Any GetParseResult(const Vector<float32>& value)
    {
        if (value.empty())
        {
            Vector3 vec;
            Vector<float32> result = { vec.x, vec.y, vec.z };
            return result;
        }

        return value;
    }

    static String GetParseErrorMessage()
    {
        return "Incorrect vector3 format. Vector3 format [ x; y; z]";
    }
};
struct Vector4Traits : public NotReadOnlyTraits
{
    using Type = Vector4;
    static int32 GetFieldIndex(const FastName& /*name*/)
    {
        return -1;
    }

    static Vector4 GetDefaultValue()
    {
        return Vector4();
    }

    static String ToString(const Vector4& v, int32 /*fieldIndex*/, const Reflection& ref)
    {
        int32 accuracy = 6;
        const M::FloatNumberAccuracy* accuracyMeta = ref.GetMeta<M::FloatNumberAccuracy>();
        if (accuracyMeta != nullptr)
        {
            accuracy = accuracyMeta->accuracy;
        }

        String x, y, z, w;
        FloatToString(v.x, accuracy, x);
        FloatToString(v.y, accuracy, y);
        FloatToString(v.z, accuracy, z);
        FloatToString(v.w, accuracy, w);

        return Format("[ %s, %s, %s, %s]", x.c_str(), y.c_str(), z.c_str(), w.c_str());
    }

    static uint32 GetMaxComponentCount()
    {
        return 4;
    }

    static Vector4 CombineValue(const Any& v, const Vector4& /*prevValue*/, int32 /*fieldIndex*/)
    {
        const Vector<float32>& values = v.Get<Vector<float32>>();
        uint32 componentCount = static_cast<uint32>(values.size());
        Vector4 vec;
        if (componentCount > 0)
            vec.x = values[0];
        if (componentCount > 1)
            vec.y = values[1];
        if (componentCount > 2)
            vec.z = values[2];
        if (componentCount > 3)
            vec.w = values[3];

        return vec;
    }

    static Any GetParseResult(const Vector<float32>& value)
    {
        if (value.empty())
        {
            Vector4 vec;
            Vector<float32> result = { vec.x, vec.y, vec.z, vec.w };
            return result;
        }

        return value;
    }

    static String GetParseErrorMessage()
    {
        return "Incorrect vector4 format. Vector4 format [ x; y; z; w]";
    }
};
struct RectTraits
{
    using Type = Rect;
    static int32 GetFieldIndex(const FastName& /*name*/)
    {
        return -1;
    }

    static String ToString(const Rect& v, int32 /*fieldIndex*/, const Reflection& ref)
    {
        int32 accuracy = 6;
        const M::FloatNumberAccuracy* accuracyMeta = ref.GetMeta<M::FloatNumberAccuracy>();
        if (accuracyMeta != nullptr)
        {
            accuracy = accuracyMeta->accuracy;
        }

        String x, y, w, h;
        FloatToString(v.x, accuracy, x);
        FloatToString(v.y, accuracy, y);
        FloatToString(v.dx, accuracy, w);
        FloatToString(v.dy, accuracy, h);
        return Format("[ %s, %s, %s, %s]", x.c_str(), y.c_str(), w.c_str(), h.c_str());
    }

    static uint32 GetMaxComponentCount()
    {
        DVASSERT(false);
        return 0;
    }

    static Rect CombineValue(const Any& v, const Rect& /*prevValue*/, int32 /*fieldIndex*/)
    {
        DVASSERT(false);
        return Rect();
    }

    static Any GetParseResult(const Vector<float32>& value)
    {
        DVASSERT(false);
        return value;
    }

    static bool IsReadOnly()
    {
        return true;
    }

    static String GetParseErrorMessage()
    {
        DVASSERT(false);
        return "";
    }
};
struct AABBox3Traits
{
    using Type = AABBox3;
    static int32 GetFieldIndex(const FastName& /*name*/)
    {
        return -1;
    }

    static String ToString(const AABBox3& v, int32 /*fieldIndex*/, const Reflection& ref)
    {
        int32 accuracy = 6;
        const M::FloatNumberAccuracy* accuracyMeta = ref.GetMeta<M::FloatNumberAccuracy>();
        if (accuracyMeta != nullptr)
        {
            accuracy = accuracyMeta->accuracy;
        }

        String minX, minY, minZ, maxX, maxY, maxZ;
        FloatToString(v.min.x, accuracy, minX);
        FloatToString(v.min.y, accuracy, minY);
        FloatToString(v.min.z, accuracy, minZ);
        FloatToString(v.max.x, accuracy, maxX);
        FloatToString(v.max.y, accuracy, maxY);
        FloatToString(v.max.z, accuracy, maxZ);

        return Format("[ %s, %s, %s,\n %s, %s, %s]", minX.c_str(), minY.c_str(), minZ.c_str(), maxX.c_str(), maxY.c_str(), maxZ.c_str());
    }

    static uint32 GetMaxComponentCount()
    {
        DVASSERT(false);
        return 0;
    }

    static AABBox3 CombineValue(const Any& v, const AABBox3& /*prevValue*/, int32 /*fieldIndex*/)
    {
        DVASSERT(false);
        return AABBox3();
    }

    static Any GetParseResult(const Vector<float32>& value)
    {
        DVASSERT(false);
        return value;
    }

    static bool IsReadOnly()
    {
        return true;
    }

    static String GetParseErrorMessage()
    {
        DVASSERT(false);
        return "";
    }
};

struct ColorChannelTraits : public NotReadOnlyTraits
{
public:
    using Type = Color;
    static int32 GetFieldIndex(const FastName& name)
    {
        if (name == colorR)
            return 0;
        else if (name == colorG)
            return 1;
        else if (name == colorB)
            return 2;
        else if (name == colorA)
            return 3;

        return -1;
    }

    static String ToString(const Color& value, int32 fieldIndex, const Reflection& ref)
    {
        String result;
        FloatToString(value.color[fieldIndex], 3, result);
        return result;
    }
    static uint32 GetMaxComponentCount()
    {
        return 1;
    }

    static Color CombineValue(const Any& v, const Color& prevValue, int32 fieldIndex)
    {
        Color result = prevValue;
        result.color[fieldIndex] = v.Get<float32>();

        return result;
    }

    static Any GetParseResult(const Vector<float32>& value)
    {
        if (value.empty())
        {
            return 0.0f;
        }
        return value[0];
    }

    static String GetParseErrorMessage()
    {
        return "Incorrect color channel format. Value should be float";
    }
};
struct Vector2ChannelTraits : public NotReadOnlyTraits
{
public:
    using Type = Vector2;
    static int32 GetFieldIndex(const FastName& name)
    {
        if (name == vectorX)
            return 0;
        else if (name == vectorY)
            return 1;

        return -1;
    }

    static String ToString(const Vector2& value, int32 fieldIndex, const Reflection& ref)
    {
        int32 accuracy = 6;
        const M::FloatNumberAccuracy* accuracyMeta = ref.GetMeta<M::FloatNumberAccuracy>();
        if (accuracyMeta != nullptr)
        {
            accuracy = accuracyMeta->accuracy;
        }

        String result;
        FloatToString(value.data[fieldIndex], accuracy, result);

        return result;
    }
    static uint32 GetMaxComponentCount()
    {
        return 1;
    }

    static Vector2 CombineValue(const Any& v, const Vector2& prevValue, int32 fieldIndex)
    {
        Vector2 result = prevValue;
        result.data[fieldIndex] = v.Get<float32>();

        return result;
    }

    static Any GetParseResult(const Vector<float32>& value)
    {
        if (value.empty())
        {
            return 0.0f;
        }
        return value[0];
    }

    static String GetParseErrorMessage()
    {
        return "Incorrect vector component format. Value should be float";
    }
};
struct Vector3ChannelTraits : public NotReadOnlyTraits
{
public:
    using Type = Vector3;
    static int32 GetFieldIndex(const FastName& name)
    {
        if (name == vectorX)
            return 0;
        else if (name == vectorY)
            return 1;
        else if (name == vectorZ)
            return 2;

        return -1;
    }

    static String ToString(const Vector3& value, int32 fieldIndex, const Reflection& ref)
    {
        int32 accuracy = 6;
        const M::FloatNumberAccuracy* accuracyMeta = ref.GetMeta<M::FloatNumberAccuracy>();
        if (accuracyMeta != nullptr)
        {
            accuracy = accuracyMeta->accuracy;
        }

        String result;
        FloatToString(value.data[fieldIndex], accuracy, result);

        return result;
    }
    static uint32 GetMaxComponentCount()
    {
        return 1;
    }

    static Vector3 CombineValue(const Any& v, const Vector3& prevValue, int32 fieldIndex)
    {
        Vector3 result = prevValue;
        result.data[fieldIndex] = v.Get<float32>();

        return result;
    }

    static Any GetParseResult(const Vector<float32>& value)
    {
        if (value.empty())
        {
            return 0.0f;
        }
        return value[0];
    }

    static String GetParseErrorMessage()
    {
        return "Incorrect vector component format. Value should be float";
    }
};
struct Vector4ChannelTraits : public NotReadOnlyTraits
{
public:
    using Type = Vector4;
    static int32 GetFieldIndex(const FastName& name)
    {
        if (name == vectorX)
            return 0;
        else if (name == vectorY)
            return 1;
        else if (name == vectorZ)
            return 2;
        else if (name == vectorW)
            return 3;

        return -1;
    }

    static String ToString(const Vector4& value, int32 fieldIndex, const Reflection& ref)
    {
        int32 accuracy = 6;
        const M::FloatNumberAccuracy* accuracyMeta = ref.GetMeta<M::FloatNumberAccuracy>();
        if (accuracyMeta != nullptr)
        {
            accuracy = accuracyMeta->accuracy;
        }

        String result;
        FloatToString(value.data[fieldIndex], accuracy, result);

        return result;
    }
    static uint32 GetMaxComponentCount()
    {
        return 1;
    }

    static Vector4 CombineValue(const Any& v, const Vector4& prevValue, int32 fieldIndex)
    {
        Vector4 result = prevValue;
        result.data[fieldIndex] = v.Get<float32>();

        return result;
    }

    static Any GetParseResult(const Vector<float32>& value)
    {
        if (value.empty())
        {
            return 0.0f;
        }
        return value[0];
    }

    static String GetParseErrorMessage()
    {
        return "Incorrect vector component format. Value should be float";
    }
};

struct RectChannelTraits : public NotReadOnlyTraits
{
public:
    using Type = Rect;
    static int32 GetFieldIndex(const FastName& name)
    {
        if (name == rectX)
            return 0;
        else if (name == rectY)
            return 1;
        else if (name == rectW)
            return 2;
        else if (name == rectH)
            return 3;

        return -1;
    }

    static String ToString(const Rect& value, int32 fieldIndex, const Reflection& ref)
    {
        int32 accuracy = 6;
        const M::FloatNumberAccuracy* accuracyMeta = ref.GetMeta<M::FloatNumberAccuracy>();
        if (accuracyMeta != nullptr)
        {
            accuracy = accuracyMeta->accuracy;
        }
        String result;
        FloatToString(*GetComponentPointer<const Rect, const float32>(value, fieldIndex), accuracy, result);

        return result;
    }
    static uint32 GetMaxComponentCount()
    {
        return 1;
    }

    static Rect CombineValue(const Any& v, const Rect& prevValue, int32 fieldIndex)
    {
        Rect result = prevValue;
        *GetComponentPointer<Rect, float32>(result, fieldIndex) = v.Get<float32>();

        return result;
    }

    static Any GetParseResult(const Vector<float32>& value)
    {
        if (value.empty())
        {
            return 0.0f;
        }
        return value[0];
    }

    static String GetParseErrorMessage()
    {
        return "Incorrect rect component format. Value should be float";
    }

private:
    template <typename T, typename TRet>
    static TRet* GetComponentPointer(T& r, int32 index)
    {
        switch (index)
        {
        case 0:
            return &r.x;
        case 1:
            return &r.y;
        case 2:
            return &r.dx;
        case 3:
            return &r.dy;
        default:
            DVASSERT(false);
            break;
        }

        return nullptr;
    }
};

struct AABBox3ChannelTraits : public NotReadOnlyTraits
{
public:
    using Type = AABBox3;
    static int32 GetFieldIndex(const FastName& name)
    {
        if (name == boxMinX)
            return 0;
        else if (name == boxMinY)
            return 1;
        else if (name == boxMinZ)
            return 2;
        else if (name == boxMaxX)
            return 3;
        else if (name == boxMaxY)
            return 4;
        else if (name == boxMaxZ)
            return 5;

        return -1;
    }

    static String ToString(const AABBox3& value, int32 fieldIndex, const Reflection& ref)
    {
        int32 accuracy = 6;
        const M::FloatNumberAccuracy* accuracyMeta = ref.GetMeta<M::FloatNumberAccuracy>();
        if (accuracyMeta != nullptr)
        {
            accuracy = accuracyMeta->accuracy;
        }

        String result;
        FloatToString(*GetComponentPointer<const AABBox3, const float32>(value, fieldIndex), accuracy, result);
        return result;
    }
    static uint32 GetMaxComponentCount()
    {
        return 1;
    }

    static AABBox3 CombineValue(const Any& v, const AABBox3& prevValue, int32 fieldIndex)
    {
        AABBox3 result = prevValue;
        *GetComponentPointer<AABBox3, float32>(result, fieldIndex) = v.Get<float32>();

        return result;
    }

    static Any GetParseResult(const Vector<float32>& value)
    {
        if (value.empty())
        {
            return 0.0f;
        }
        return value[0];
    }

    static String GetParseErrorMessage()
    {
        return "Incorrect aabbox3 component format. Value should be float";
    }

private:
    template <typename T, typename TRet>
    static TRet* GetComponentPointer(T& r, int32 index)
    {
        if (index < 3)
        {
            return &r.min.data[index];
        }
        else
        {
            DVASSERT(index < 6);
            index = index - 3;
            return &r.max.data[index];
        }
    }
};

template <typename Traits>
class TraitsFieldAccessor : public IFieldAccessor
{
public:
    TraitsFieldAccessor(const FastName& propertyName_)
        : fieldIndex(Traits::GetFieldIndex(propertyName_))
        , propertyName(propertyName_.c_str())
    {
    }

    String GetFieldValue(const Any& v, const Reflection& r) const override
    {
        if (v.CanCast<typename Traits::Type>() == false)
        {
            return v.Cast<String>();
        }

        return Traits::ToString(v.Cast<typename Traits::Type>(), fieldIndex, r);
    }

    Any CreateNewValue(const String& newFieldValue, const Any& propertyValue, M::ValidationResult& result) const override
    {
        if (IsReadOnly())
        {
            result.state = M::ValidationResult::eState::Valid;
            return Any();
        }

        DVASSERT(propertyValue.CanCast<typename Traits::Type>());
        Any parseResult = Parse(newFieldValue, result);
        if (result.state == M::ValidationResult::eState::Invalid)
        {
            return result;
        }

        result.state = M::ValidationResult::eState::Valid;
        return Traits::CombineValue(parseResult, propertyValue.Cast<typename Traits::Type>(), fieldIndex);
    }

    Any Parse(const String& strValue, M::ValidationResult& result) const override
    {
        if (IsReadOnly())
        {
            result.state = M::ValidationResult::eState::Valid;
            return Any();
        }

        if (strValue.empty())
        {
            result.state = M::ValidationResult::eState::Valid;
            return Traits::GetParseResult(Vector<float32>());
        }

        Vector<float32> parseResult = ParseFloatList(strValue);
        uint32 componentCount = static_cast<uint32>(parseResult.size());
        if (componentCount > Traits::GetMaxComponentCount())
        {
            result.state = M::ValidationResult::eState::Invalid;
            result.message = Traits::GetParseErrorMessage();
            return Any();
        }

        result.state = M::ValidationResult::eState::Valid;
        return Traits::GetParseResult(parseResult);
    }

    bool IsReadOnly() const override
    {
        return Traits::IsReadOnly();
    }

    bool OverridePropertyName(QString& name) const override
    {
        if (fieldIndex == -1)
        {
            return false;
        }

        name = propertyName;
        return true;
    }

private:
    int32 fieldIndex = -1;
    const QString propertyName;
};

class ColorComponentValue : public TextComponentValue
{
public:
    ColorComponentValue()
        : TextComponentValue(std::make_unique<TraitsFieldAccessor<ColorTraits>>(FastName("Color")))
    {
    }

protected:
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) override
    {
        Widget* w = new Widget(parent);
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setMargin(0);
        layout->setSpacing(1);
        w->SetLayout(layout);

        ColorPickerButton::Params params(GetAccessor(), GetUI(), GetWindowKey());
        params.fields[ColorPickerButton::Fields::Color] = "color";
        params.fields[ColorPickerButton::Fields::IntermediateColor] = "intermediateColor";
        params.fields[ColorPickerButton::Fields::IsReadOnly] = readOnlyFieldName;
        w->AddControl(new ColorPickerButton(params, wrappersProcessor, model, w->ToWidgetCast()));
        w->AddControl(TextComponentValue::CreateEditorWidget(w->ToWidgetCast(), model, wrappersProcessor));

        return w;
    }

    Any GetColor() const
    {
        return GetValue();
    }

    void SetColor(const Any& color)
    {
        SetValue(color);
    }

    Any GetIntermediateColor() const
    {
        return GetValue();
    }

    void SetIntermediateColor(const Any& color)
    {
        Any currentValue = nodes.front()->field.ref.GetValue();
        for (const std::shared_ptr<const PropertyNode>& node : nodes)
        {
            if (currentValue != node->field.ref.GetValue())
            {
                currentValue = GetMultipleValue();
                break;
            }
        }

        if (IsValidValueToSet(color, currentValue))
        {
            for (const std::shared_ptr<PropertyNode>& node : nodes)
            {
                node->cachedValue = color;
                node->field.ref.SetValueWithCast(color);
            }
        }
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ColorComponentValue, TextComponentValue)
    {
        ReflectionRegistrator<ColorComponentValue>::Begin()
        .Field("color", &ColorComponentValue::GetColor, &ColorComponentValue::SetColor)
        .Field("intermediateColor", &ColorComponentValue::GetIntermediateColor, &ColorComponentValue::SetIntermediateColor)
        .End();
    }
};

using TCreatorFn = Function<std::unique_ptr<BaseComponentValue>(const FastName&)>;
struct Key
{
    Key() = default;
    Key(const Type* t, int32 propType)
        : type(t)
        , propertyType(propType)
    {
    }

    bool operator<(const Key& other) const
    {
        if (type != other.type)
        {
            return type < other.type;
        }
        return propertyType < other.propertyType;
    }

    const Type* type = nullptr;
    int32 propertyType;
};

Map<Key, TCreatorFn> creatorMap;

void InitSubPropertyTypes()
{
    if (creatorMap.empty())
    {
        {
            Key k(Type::Instance<Color>(), PropertyNode::RealProperty);
            creatorMap[k] = [](const FastName&) { return std::make_unique<ColorComponentValue>(); };
        }

        {
            Key k(Type::Instance<Color>(), PropertyNode::VirtualProperty);
            creatorMap[k] = [](const FastName& fieldName) { return std::make_unique<TextComponentValue>(std::make_unique<TraitsFieldAccessor<ColorChannelTraits>>(fieldName)); };
        }

        {
            Key k(Type::Instance<Vector2>(), PropertyNode::RealProperty);
            creatorMap[k] = [](const FastName& fieldName) { return std::make_unique<TextComponentValue>(std::make_unique<TraitsFieldAccessor<Vector2Traits>>(fieldName)); };
        }

        {
            Key k(Type::Instance<Vector2>(), PropertyNode::VirtualProperty);
            creatorMap[k] = [](const FastName& fieldName) { return std::make_unique<TextComponentValue>(std::make_unique<TraitsFieldAccessor<Vector2ChannelTraits>>(fieldName)); };
        }

        {
            Key k(Type::Instance<Vector3>(), PropertyNode::RealProperty);
            creatorMap[k] = [](const FastName& fieldName) { return std::make_unique<TextComponentValue>(std::make_unique<TraitsFieldAccessor<Vector3Traits>>(fieldName)); };
        }

        {
            Key k(Type::Instance<Vector3>(), PropertyNode::VirtualProperty);
            creatorMap[k] = [](const FastName& fieldName) { return std::make_unique<TextComponentValue>(std::make_unique<TraitsFieldAccessor<Vector3ChannelTraits>>(fieldName)); };
        }

        {
            Key k(Type::Instance<Vector4>(), PropertyNode::RealProperty);
            creatorMap[k] = [](const FastName& fieldName) { return std::make_unique<TextComponentValue>(std::make_unique<TraitsFieldAccessor<Vector4Traits>>(fieldName)); };
        }

        {
            Key k(Type::Instance<Vector4>(), PropertyNode::VirtualProperty);
            creatorMap[k] = [](const FastName& fieldName) { return std::make_unique<TextComponentValue>(std::make_unique<TraitsFieldAccessor<Vector4ChannelTraits>>(fieldName)); };
        }

        {
            Key k(Type::Instance<Rect>(), PropertyNode::RealProperty);
            creatorMap[k] = [](const FastName& fieldName) { return std::make_unique<TextComponentValue>(std::make_unique<TraitsFieldAccessor<RectTraits>>(fieldName)); };
        }

        {
            Key k(Type::Instance<Rect>(), PropertyNode::VirtualProperty);
            creatorMap[k] = [](const FastName& fieldName) { return std::make_unique<TextComponentValue>(std::make_unique<TraitsFieldAccessor<RectChannelTraits>>(fieldName)); };
        }

        {
            Key k(Type::Instance<AABBox3>(), PropertyNode::RealProperty);
            creatorMap[k] = [](const FastName& fieldName) { return std::make_unique<TextComponentValue>(std::make_unique<TraitsFieldAccessor<AABBox3Traits>>(fieldName)); };
        }

        {
            Key k(Type::Instance<AABBox3>(), PropertyNode::VirtualProperty);
            creatorMap[k] = [](const FastName& fieldName) { return std::make_unique<TextComponentValue>(std::make_unique<TraitsFieldAccessor<AABBox3ChannelTraits>>(fieldName)); };
        }
    }
}
}

void SubPropertyValueChildCreator::ExposeChildren(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const
{
    using namespace SubPropertiesExtensionsDetail;
    InitSubPropertyTypes();
    bool isOurType = false;
    if (parent->propertyType == PropertyNode::RealProperty)
    {
        const Type* valueType = parent->field.ref.GetValueType()->Decay();
        if (valueType == Type::Instance<Color>())
        {
            ExposeColorChildren(parent, children);
            isOurType = true;
        }
        else if (valueType == Type::Instance<Vector2>())
        {
            ExposeVectorChildren<Vector2>(parent, children);
            isOurType = true;
        }
        else if (valueType == Type::Instance<Vector3>())
        {
            ExposeVectorChildren<Vector3>(parent, children);
            isOurType = true;
        }
        else if (valueType == Type::Instance<Vector4>())
        {
            ExposeVectorChildren<Vector4>(parent, children);
            isOurType = true;
        }
        else if (valueType == Type::Instance<Rect>())
        {
            ExposeRectChildren(parent, children);
            isOurType = true;
        }
        else if (valueType == Type::Instance<AABBox3>())
        {
            ExposeAABBox3Children(parent, children);
            isOurType = true;
        }
    }

    if (isOurType == false)
    {
        ChildCreatorExtension::ExposeChildren(parent, children);
    }
}

void SubPropertyValueChildCreator::ExposeColorChildren(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const
{
    using namespace SubPropertiesExtensionsDetail;
    {
        Reflection::Field field = parent->field;
        field.key = colorR;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }
    {
        Reflection::Field field = parent->field;
        field.key = colorG;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }
    {
        Reflection::Field field = parent->field;
        field.key = colorB;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }
    {
        Reflection::Field field = parent->field;
        field.key = colorA;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }
}

void SubPropertyValueChildCreator::ExposeRectChildren(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const
{
    using namespace SubPropertiesExtensionsDetail;
    {
        Reflection::Field field = parent->field;
        field.key = rectX;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }
    {
        Reflection::Field field = parent->field;
        field.key = rectY;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }
    {
        Reflection::Field field = parent->field;
        field.key = rectW;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }
    {
        Reflection::Field field = parent->field;
        field.key = rectH;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }
}

void SubPropertyValueChildCreator::ExposeAABBox3Children(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const
{
    using namespace SubPropertiesExtensionsDetail;
    {
        Reflection::Field field = parent->field;
        field.key = boxMinX;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }
    {
        Reflection::Field field = parent->field;
        field.key = boxMinY;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }
    {
        Reflection::Field field = parent->field;
        field.key = boxMinZ;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }
    {
        Reflection::Field field = parent->field;
        field.key = boxMaxX;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }
    {
        Reflection::Field field = parent->field;
        field.key = boxMaxY;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }
    {
        Reflection::Field field = parent->field;
        field.key = boxMaxZ;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }
}

template <typename TVector>
void SubPropertyValueChildCreator::ExposeVectorChildren(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const
{
    using namespace SubPropertiesExtensionsDetail;
    {
        Reflection::Field field = parent->field;
        field.key = vectorX;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }
    {
        Reflection::Field field = parent->field;
        field.key = vectorY;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }

    const Type* vecType = Type::Instance<TVector>();
    const Type* vec3Type = Type::Instance<Vector3>();
    const Type* vec4Type = Type::Instance<Vector4>();
    if (vecType == vec3Type || vecType == vec4Type)
    {
        Reflection::Field field = parent->field;
        field.key = vectorZ;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }

    if (vecType == vec4Type)
    {
        Reflection::Field field = parent->field;
        field.key = vectorW;
        children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::VirtualProperty));
    }
}

#if __clang__
_Pragma("clang diagnostic push")
_Pragma("clang diagnostic ignored \"-Wweak-template-vtables\"")
#endif

template void SubPropertyValueChildCreator::ExposeVectorChildren<Vector2>(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const;
template void SubPropertyValueChildCreator::ExposeVectorChildren<Vector3>(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const;
template void SubPropertyValueChildCreator::ExposeVectorChildren<Vector4>(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const;

#if __clang__
_Pragma("clang diagnostic pop")
#endif

std::unique_ptr<BaseComponentValue> SubPropertyEditorCreator::GetEditor(const std::shared_ptr<const PropertyNode>& node) const
{
    using namespace SubPropertiesExtensionsDetail;
    InitSubPropertyTypes();

    const Type* valueType = node->field.ref.GetValueType()->Decay();
    Key k(valueType, node->propertyType);
    auto iter = creatorMap.find(k);
    if (iter != creatorMap.end())
    {
        return (*iter).second(node->field.key.Cast<FastName>());
    }

    return EditorComponentExtension::GetEditor(node);
}
} // namespace DAVA
