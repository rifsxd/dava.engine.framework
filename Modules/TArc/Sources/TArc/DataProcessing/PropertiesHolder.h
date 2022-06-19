#pragma once

#include <TArc/Qt/QtRect.h>
#include <TArc/Qt/QtByteArray.h>
#include <TArc/Qt/QtString.h>

#include <Base/BaseTypes.h>
#include <Base/Any.h>

#include <memory>

namespace DAVA
{
class FilePath;
class Type;
class FileSystem;
class PropertiesItem;

class PropertiesHolder
{
public:
    PropertiesHolder(const String& rootPath, const FilePath& dirPath);
    ~PropertiesHolder();

    PropertiesItem CreateSubHolder(const String& name) const;
    static std::unique_ptr<PropertiesHolder> CopyWithNewPath(PropertiesHolder& holder,
                                                             FileSystem* fs,
                                                             const String& projectName,
                                                             const FilePath& directory);
    void SaveToFile();

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

class PropertiesItem
{
public:
    virtual ~PropertiesItem();

    PropertiesItem CreateSubHolder(const String& name) const;

    void Set(const String& key, const Any& value);

    template <typename T>
    T Get(const String& key, const T& defaultValue = T()) const;
    Any Get(const String& key, const Any& defaultValue, const Type* type) const;

    PropertiesItem(const PropertiesItem& holder) = delete;
    PropertiesItem(PropertiesItem&& holder);
    PropertiesItem& operator=(const PropertiesItem& holder) = delete;
    PropertiesItem& operator=(PropertiesItem&& holder);

private:
    friend class PropertiesHolder;
    //RootPropertiesHolder use this empty c-tor
    PropertiesItem();
    PropertiesItem(const PropertiesItem& parent, const String& name);

    struct Impl;
    std::unique_ptr<Impl> impl;
};

template <typename T>
T PropertiesItem::Get(const String& key, const T& defaultValue) const
{
    Any loadedValue = Get(key, defaultValue, Type::Instance<T>());
    return loadedValue.Cast<T>(defaultValue);
}
} // namespace DAVA
