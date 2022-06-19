#pragma once

#include <PluginManager/Plugin.h>

#include <Base/Type.h>
#include <Base/BaseTypes.h>

namespace DAVA
{
class TArcPlugin;
class TArcPluginManager
{
public:
    TArcPluginManager(const String& applicationName, const String& pluginsFolder);
    ~TArcPluginManager();

    void LoadPlugins(Vector<String>& errors);
    void UnloadPlugins();

    TArcPlugin* GetPlugin(const String& pluginName) const;
    Vector<TArcPlugin*> GetPluginsWithBaseType(const Type* t) const;

private:
    String applicationName;
    String pluginsFolder;

    struct PluginNode
    {
        Vector<TArcPlugin*> pluginInstances;
        PluginHandle handle = nullptr;
        String libraryPath;
    };

    Vector<PluginNode> pluginCollection;
};
} // namespace DAVA
