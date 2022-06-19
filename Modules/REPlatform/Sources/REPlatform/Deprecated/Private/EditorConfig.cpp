#include "REPlatform/Deprecated/EditorConfig.h"

namespace DAVA
{
EditorConfig::EditorConfig()
{
    empty.push_back("none");
}

EditorConfig::~EditorConfig()
{
    ClearConfig();
}

void EditorConfig::ClearConfig()
{
    propertyNames.clear();
    Map<String, PropertyDescription*>::iterator it = properties.begin();
    Map<String, PropertyDescription*>::iterator propEnd = properties.end();
    for (; it != propEnd; ++it)
    {
        SafeRelease(it->second);
    }
    properties.clear();
}

int32 EditorConfig::ParseType(const String& typeStr)
{
    if (typeStr == "Bool")
    {
        return PT_BOOL;
    }
    if (typeStr == "Int")
    {
        return PT_INT;
    }
    if (typeStr == "Float")
    {
        return PT_FLOAT;
    }

    if (typeStr == "String")
    {
        return PT_STRING;
    }
    if (typeStr == "Combobox")
    {
        return PT_COMBOBOX;
    }

    if (typeStr == "ColorList")
    {
        return PT_COLOR_LIST;
    }

    if (typeStr == "CollisionTypeMap")
    {
        return PT_COLLISION_TYPE_MAP;
    }
    return PT_NONE;
}

void EditorConfig::ParseConfig(const FilePath& filePath)
{
    ClearConfig();

    RefPtr<YamlParser> parser(YamlParser::Create(filePath));
    if (parser.Get() == nullptr)
        return;

    YamlNode* rootNode = parser->GetRootNode();
    if (rootNode == nullptr)
        return;

    const auto& yamlNodes = rootNode->AsVector();
    size_t propertiesCount = yamlNodes.size();
    for (size_t i = 0; i < propertiesCount; ++i)
    {
        YamlNode* propertyNode = yamlNodes[i].Get();
        if (propertyNode == nullptr)
        {
            Logger::Error("EditorConfig::ParseConfig %s ERROR property %d is missing", filePath.GetAbsolutePathname().c_str(), i);
            continue;
        }

        const YamlNode* nameNode = propertyNode->Get("name");
        const YamlNode* typeNode = propertyNode->Get("type");
        const YamlNode* defaultNode = propertyNode->Get("default");
        if (nameNode == nullptr || typeNode == nullptr)
        {
            Logger::Error("EditorConfig::ParseConfig %s ERROR property %d type or name is missing", filePath.GetAbsolutePathname().c_str(), i);
            continue;
        }

        const String& nameStr = nameNode->AsString();
        const String& typeStr = typeNode->AsString();
        int32 type = ParseType(typeStr);
        if (type == 0)
        {
            Logger::Error("EditorConfig::ParseConfig %s ERROR property %d unknown type %s", filePath.GetAbsolutePathname().c_str(), i, typeStr.c_str());
            continue;
        }

        bool isOk = true;
        for (const String& propertyName : propertyNames)
        {
            if (propertyName == nameStr)
            {
                isOk = false;
                Logger::Error("EditorConfig::ParseConfig %s ERROR property %d property %s already exists", filePath.GetAbsolutePathname().c_str(), i, nameStr.c_str());
                break;
            }
        }
        if (isOk == false)
            continue;

        const DAVA::YamlNode* isHiddenNode = propertyNode->Get("hidden");
        bool propertyHidden = false;
        if (isHiddenNode != nullptr)
        {
            propertyHidden = isHiddenNode->AsBool();
        }

        properties[nameStr] = new PropertyDescription();
        properties[nameStr]->name = nameStr;
        properties[nameStr]->type = type;
        switch (type)
        {
        case PT_BOOL:
        {
            bool defaultValue = false;
            if (defaultNode)
            {
                defaultValue = defaultNode->AsBool();
            }
            properties[nameStr]->defaultValue.SetBool(defaultValue);
        }
        break;
        case PT_INT:
        {
            DAVA::int32 defaultValue = 0;
            if (defaultNode)
            {
                defaultValue = defaultNode->AsInt();
            }
            properties[nameStr]->defaultValue.SetInt32(defaultValue);
        }
        break;
        case PT_STRING:
        {
            DAVA::String defaultValue;
            if (defaultNode)
            {
                defaultValue = defaultNode->AsString();
            }
            properties[nameStr]->defaultValue.SetString(defaultValue);
        }
        break;
        case PT_FLOAT:
        {
            DAVA::float32 defaultValue = 0.0f;
            if (defaultNode)
            {
                defaultValue = defaultNode->AsFloat();
            }
            properties[nameStr]->defaultValue.SetFloat(defaultValue);
        }
        break;
        case PT_COMBOBOX:
        {
            DAVA::int32 defaultValue = 0;
            if (defaultNode)
            {
                defaultValue = defaultNode->AsInt();
            }
            properties[nameStr]->defaultValue.SetInt32(defaultValue);

            const DAVA::YamlNode* comboNode = propertyNode->Get("list");
            if (comboNode == nullptr)
                break;

            const auto& comboValueNodes = comboNode->AsVector();
            for (const auto& comboValueNode : comboValueNodes)
            {
                properties[nameStr]->comboValues.push_back(comboValueNode->AsString());
            }
        }
        break;
        case PT_COLOR_LIST:
        {
            DAVA::int32 defaultValue = 0;
            if (defaultNode)
            {
                defaultValue = defaultNode->AsInt();
            }
            properties[nameStr]->defaultValue.SetInt32(defaultValue);

            const DAVA::YamlNode* colorListNode = propertyNode->Get("list");
            if (colorListNode == nullptr)
                break;

            const auto& colorListNodes = colorListNode->AsVector();
            for (const auto& colorNode : colorListNodes)
            {
                if (!colorNode || colorNode->GetCount() != 4)
                    continue;

                DAVA::Color color(colorNode->Get(0)->AsFloat() / 255.f,
                                  colorNode->Get(1)->AsFloat() / 255.f,
                                  colorNode->Get(2)->AsFloat() / 255.f,
                                  colorNode->Get(3)->AsFloat() / 255.f);

                properties[nameStr]->colorListValues.push_back(color);
            }
        }
        break;
        case PT_COLLISION_TYPE_MAP:
        {
            DAVA::int32 defaultValue = 0;
            if (defaultNode != nullptr)
            {
                defaultValue = defaultNode->AsInt();
            }
            properties[nameStr]->defaultValue.SetInt32(defaultValue);

            const DAVA::YamlNode* mapNode = propertyNode->Get("map");
            if (mapNode == nullptr)
                break;

            const auto& mapNodes = mapNode->AsVector();
            for (const auto& pairNode : mapNodes)
            {
                std::pair<DAVA::int32, DAVA::String> pair = { pairNode->Get(1)->AsInt(), pairNode->Get(0)->AsString() };
                properties[nameStr]->collisionTypeMapValues.push_back(pair);
            }
        }
        break;
        default:
            break;
        }
        if (propertyHidden == false)
        {
            propertyNames.push_back(nameStr);
        }
    }
}

const Vector<String>& EditorConfig::GetProjectPropertyNames() const
{
    return propertyNames;
}

const Vector<String>& EditorConfig::GetComboPropertyValues(const String& nameStr) const
{
    auto itemNode = properties.find(nameStr);
    if (itemNode != properties.end())
    {
        return itemNode->second->comboValues;
    }
    else
    {
        return empty;
    }
}

const Vector<Color>& EditorConfig::GetColorPropertyValues(const String& nameStr) const
{
    auto iter = properties.find(nameStr);
    if (iter != properties.end())
    {
        return iter->second->colorListValues;
    }
    else
    {
        return emptyColors;
    }
}

const DAVA::Vector<std::pair<DAVA::int32, DAVA::String>>& EditorConfig::GetCollisionTypeMap(const DAVA::String& nameStr) const
{
    auto iter = properties.find(nameStr);
    if (iter != properties.end())
    {
        return iter->second->collisionTypeMapValues;
    }
    else
    {
        return emptyCollisionTypeMap;
    }
}

const PropertyDescription* EditorConfig::GetPropertyDescription(const String& propertyName) const
{
    Map<String, PropertyDescription*>::const_iterator findIt = properties.find(propertyName);
    if (findIt != properties.end())
    {
        return findIt->second;
    }
    else
    {
        return nullptr;
    }
}

bool EditorConfig::HasProperty(const String& propertyName) const
{
    return (GetPropertyDescription(propertyName) != nullptr);
}

int32 EditorConfig::GetValueTypeFromPropertyType(int32 propertyType) const
{
    int32 type = VariantType::TYPE_NONE;
    switch (propertyType)
    {
    case PT_BOOL:
        type = VariantType::TYPE_BOOLEAN;
        break;
    case PT_INT:
    case PT_COMBOBOX:
    case PT_COLOR_LIST:
    case PT_COLLISION_TYPE_MAP:
        // collision type map is a list of pairs, where first is
        // collision type name and second is collision type value
        type = VariantType::TYPE_INT32;
        break;
    case PT_STRING:
        type = VariantType::TYPE_STRING;
        break;
    case PT_FLOAT:
        type = VariantType::TYPE_FLOAT;
        break;
    }
    return type;
}

int32 EditorConfig::GetPropertyValueType(const String& propertyName) const
{
    int32 type = VariantType::TYPE_NONE;
    const PropertyDescription* propertyDescription = GetPropertyDescription(propertyName);
    if (propertyDescription)
    {
        type = GetValueTypeFromPropertyType(propertyDescription->type);
    }
    return type;
}

const VariantType* EditorConfig::GetPropertyDefaultValue(const String& propertyName) const
{
    const VariantType* defaultValue = nullptr;
    const PropertyDescription* propertyDescription = GetPropertyDescription(propertyName);
    if (propertyDescription)
    {
        defaultValue = &propertyDescription->defaultValue;
    }
    return defaultValue;
}
} // namespace DAVA
