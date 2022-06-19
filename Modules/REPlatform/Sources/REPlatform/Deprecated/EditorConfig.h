#pragma once

#include "DAVAEngine.h"

namespace DAVA
{
class PropertyDescription : public BaseObject
{
protected:
    ~PropertyDescription()
    {
    }

public:
    PropertyDescription()
        : BaseObject()
        , type(0){};

    String name;
    int32 type;
    VariantType defaultValue;
    Vector<String> comboValues;
    Vector<Color> colorListValues;
    Vector<std::pair<int32, String>> collisionTypeMapValues;
};

class EditorConfig
{
public:
    EditorConfig();
    virtual ~EditorConfig();

    enum ePropertyType
    {
        PT_NONE = 0,
        PT_BOOL,
        PT_INT,
        PT_FLOAT,
        PT_STRING,
        PT_COMBOBOX,
        PT_COLOR_LIST,
        PT_COLLISION_TYPE_MAP,

        PROPERTY_TYPES_COUNT
    };

    void ParseConfig(const FilePath& filePath);

    const Vector<String>& GetProjectPropertyNames() const;
    const Vector<String>& GetComboPropertyValues(const String& nameStr) const;
    const Vector<Color>& GetColorPropertyValues(const String& nameStr) const;
    const Vector<std::pair<int32, String>>& GetCollisionTypeMap(const String& nameStr) const;

    bool HasProperty(const String& propertyName) const;
    int32 GetPropertyValueType(const String& propertyName) const;
    const VariantType* GetPropertyDefaultValue(const String& propertyName) const;

protected:
    void ClearConfig();

    const PropertyDescription* GetPropertyDescription(const String& propertyName) const;

    int32 GetValueTypeFromPropertyType(int32 propertyType) const;
    int32 ParseType(const String& typeStr);

    Vector<String> propertyNames;
    Map<String, PropertyDescription*> properties;
    Vector<String> empty;
    Vector<Color> emptyColors;
    Vector<std::pair<int32, String>> emptyCollisionTypeMap;
};
} // namespace DAVA
