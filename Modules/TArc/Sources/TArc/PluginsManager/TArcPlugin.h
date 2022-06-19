#pragma once

#include "TArc/DataProcessing/TArcAnyCasts.h"

#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Base/Type.h>
#include <Debug/DVAssert.h>
#include <Engine/EngineContext.h>
#include <Engine/Private/EngineBackend.h>
#include <PluginManager/Plugin.h>
#include <Reflection/ReflectedType.h>
#include <Reflection/ReflectedTypeDB.h>
#include <ReflectionDeclaration/Private/AnyCasts.h>
#include <Render/Renderer.h>
#include <Render/RHI/rhi_Public.h>

namespace DAVA
{
class EngineContext;
class TArcPlugin
{
public:
    struct PluginDescriptor
    {
        PluginDescriptor(const String& appName, const String& plugName, const String& shortDescr,
                         const String& fullDescr, int32 majorVer, int32 minorVer)
            : applicationName(appName)
            , pluginName(plugName)
            , shortDescription(shortDescr)
            , fullDescription(fullDescr)
            , majorVersion(majorVer)
            , minorVersion(minorVer)
        {
        }

        String applicationName;
        String pluginName;
        String shortDescription;
        String fullDescription;
        int32 majorVersion = 0;
        int32 minorVersion = 0;
    };

    TArcPlugin(const EngineContext* context);
    virtual ~TArcPlugin() = default;

    virtual const ReflectedType* GetModuleType() const = 0;
    virtual const PluginDescriptor& GetDescription() const = 0;
};

template <typename T>
class TypedTArcPlugin : public TArcPlugin
{
public:
    TypedTArcPlugin(const EngineContext* context, const TArcPlugin::PluginDescriptor& descriptor)
        : TArcPlugin(context)
        , descr(descriptor)
    {
    }

    const ReflectedType* GetModuleType() const override
    {
        return ReflectedTypeDB::Get<T>();
    }

    const PluginDescriptor& GetDescription() const override
    {
        return descr;
    }

private:
    PluginDescriptor descr;
};
} // namespace DAVA

#define CREATE_PLUGIN_ARRAY_FUNCTION_NAME CreatePluginArray
#define DELETE_PLUGIN_ARRAY_FUNCTION_NAME DeletePluginArray
#define DESTROY_PLUGIN_FUNCTION_NAME DestroyPlugin

using TCreatePluginArrayFn = DAVA::TArcPlugin** (*)(const DAVA::EngineContext* context);
using TDestroyPluginArrayFn = void (*)(DAVA::TArcPlugin** pluginArray);
using TDestroyPluginFn = void (*)(DAVA::TArcPlugin* plugin);

#define START_PLUGIN_DECLARATION()\
extern "C" { \
    PLUGIN_FUNCTION_EXPORT void DESTROY_PLUGIN_FUNCTION_NAME(DAVA::TArcPlugin* plugin) \
    { \
        delete plugin; \
    } \
    PLUGIN_FUNCTION_EXPORT void DELETE_PLUGIN_ARRAY_FUNCTION_NAME(DAVA::TArcPlugin** pluginArray) \
    { \
        delete[] pluginArray; \
    } \
    PLUGIN_FUNCTION_EXPORT DAVA::TArcPlugin** CREATE_PLUGIN_ARRAY_FUNCTION_NAME(const DAVA::EngineContext* context) \
    { \
        DAVA::Private::SetEngineContext(const_cast<DAVA::EngineContext*>(context)); \
        DAVA::TypeDB::GetLocalDB()->SetMasterDB(context->typeDB); \
        DAVA::FastNameDB::GetLocalDB()->SetMasterDB(context->fastNameDB); \
        DAVA::ReflectedTypeDB::GetLocalDB()->SetMasterDB(context->reflectedTypeDB); \
        DAVA::RegisterAnyCasts(); \
        DAVA::RegisterTArcAnyCasts(); \
        rhi::InitParam params; \
        params.maxIndexBufferCount = 8192; \
        params.maxVertexBufferCount = 8192; \
        params.maxConstBufferCount = 32767; \
        params.maxTextureCount = 2048; \
        params.maxSamplerStateCount = 32 * 1024; \
        params.shaderConstRingBufferSize = 256 * 1024 * 1024; \
        DAVA::Renderer::Initialize(rhi::RHI_NULL_RENDERER, params); \
        DAVA::TArcPlugin** plugins = new DAVA::TArcPlugin*[32]; \
        memset(plugins, 0, 32 * sizeof(DAVA::TArcPlugin*)); \
        DAVA::int32 counter = 0

#define DECLARE_PLUGIN(moduleType, descr)\
    { \
        DVASSERT(counter < 32); \
        plugins[counter++] = new DAVA::TypedTArcPlugin<moduleType>(context, descr); \
    }

#define END_PLUGIN_DECLARATION()\
        return plugins; \
    } \
} \
\
int DAVAMain(DAVA::Vector<DAVA::String> cmdline)\
{\
    return 0;\
}
