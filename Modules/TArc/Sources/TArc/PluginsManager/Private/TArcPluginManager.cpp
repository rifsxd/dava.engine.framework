#include "TArc/PluginsManager/TArcPluginManager.h"
#include "TArc/PluginsManager/TArcPlugin.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <PluginManager/PluginManager.h>

#include <FileSystem/FileList.h>
#include <Debug/DVAssert.h>

#define PLUGIN_MANAGER_STR_VALUE_IMPL(name) #name
#define PLUGIN_MANAGER_STR_VALUE(name) PLUGIN_MANAGER_STR_VALUE_IMPL(name)

namespace DAVA
{
TArcPluginManager::TArcPluginManager(const String& applicationName_, const String& pluginsFolder_)
    : applicationName(applicationName_)
    , pluginsFolder(pluginsFolder_)
{
}

TArcPluginManager::~TArcPluginManager()
{
    DVASSERT(pluginCollection.empty() == true);
}

void TArcPluginManager::LoadPlugins(Vector<String>& errors)
{
    UnorderedMap<String, size_t> pluginsMap;

    const EngineContext* ctx = GetEngineContext();
    Vector<FilePath> plugins = PluginManager::LookupPlugins(pluginsFolder, PluginManager::Auto);
    for (const FilePath& filePath : plugins)
    {
        String absPath = filePath.GetAbsolutePathname();
        PluginHandle handle = OpenPlugin(absPath.c_str());
        if (handle == nullptr)
        {
            errors.push_back(Format("Couldn't load dynamic library %s", absPath.c_str()));
            continue;
        }

        TCreatePluginArrayFn createFn = LoadFunction<TCreatePluginArrayFn>(handle, PLUGIN_MANAGER_STR_VALUE(CREATE_PLUGIN_ARRAY_FUNCTION_NAME));
        if (createFn == nullptr)
        {
            errors.push_back(Format("Dynamic library %s doesn't contains TArcPlugin", absPath.c_str()));
            continue;
        }

        TDestroyPluginArrayFn destroyArray = LoadFunction<TDestroyPluginArrayFn>(handle, PLUGIN_MANAGER_STR_VALUE(DELETE_PLUGIN_ARRAY_FUNCTION_NAME));

        TArcPlugin** plugins = createFn(ctx);
        int32 index = 0;

        pluginCollection.push_back(PluginNode());
        PluginNode& node = pluginCollection.back();
        node.handle = handle;
        node.libraryPath = absPath;

        while (plugins[index] != nullptr)
        {
            TArcPlugin* pluginInstance = plugins[index];
            ++index;
            if (pluginInstance == nullptr)
            {
                errors.push_back(Format("Dynamic librray %s can't create plugin", absPath.c_str()));
                continue;
            }

            const TArcPlugin::PluginDescriptor& descriptor = pluginInstance->GetDescription();
            if (applicationName != descriptor.applicationName)
            {
                errors.push_back(Format("Plugin %s loaded from %s isn't matched by applicationName. Plugin's application name is %s, plugins manager is configured for %s",
                                        descriptor.pluginName.c_str(), absPath.c_str(), descriptor.applicationName.c_str(), applicationName.c_str()));
                continue;
            }

            auto iter = pluginsMap.find(descriptor.pluginName);
            if (iter != pluginsMap.end())
            {
                String path = pluginCollection[iter->second].libraryPath;
                errors.push_back(Format("Plugin's name conflict. Libraries %s and %s contains plugin with same name %s. The last one will be ignored",
                                        path.c_str(), absPath.c_str(), descriptor.pluginName.c_str()));
                continue;
            }

            node.pluginInstances.push_back(pluginInstance);
            pluginsMap.emplace(descriptor.pluginName, pluginCollection.size());
        }

        destroyArray(plugins);
    }

#undef REPORT_LOADING_ERROR
}

void TArcPluginManager::UnloadPlugins()
{
    for (PluginNode& node : pluginCollection)
    {
        TDestroyPluginFn destroyFn = LoadFunction<TDestroyPluginFn>(node.handle, PLUGIN_MANAGER_STR_VALUE(DESTROY_PLUGIN_FUNCTION_NAME));
        DVASSERT(destroyFn != nullptr);
        if (destroyFn == nullptr)
        {
            continue;
        }

        for (TArcPlugin* instance : node.pluginInstances)
        {
            destroyFn(instance);
        }
        ClosePlugin(node.handle);
    }

    pluginCollection.clear();
}

TArcPlugin* TArcPluginManager::GetPlugin(const String& pluginName) const
{
    for (const PluginNode& node : pluginCollection)
    {
        for (TArcPlugin* instance : node.pluginInstances)
        {
            if (instance->GetDescription().pluginName == pluginName)
            {
                return instance;
            }
        }
    }

    return nullptr;
}

Vector<TArcPlugin*> TArcPluginManager::GetPluginsWithBaseType(const Type* t) const
{
    Vector<TArcPlugin*> result;
    result.reserve(pluginCollection.size());
    for (const PluginNode& node : pluginCollection)
    {
        for (TArcPlugin* instance : node.pluginInstances)
        {
            const ReflectedType* moduleType = instance->GetModuleType();
            const Type* type = moduleType->GetType();
            if (TypeInheritance::CanDownCast(type, t))
            {
                result.push_back(instance);
            }
        }
    }

    return result;
}
} // namespace DAVA
