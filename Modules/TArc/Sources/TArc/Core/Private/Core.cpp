#include "TArc/Core/Core.h"
#include "TArc/Core/ClientModule.h"
#include "TArc/Core/ConsoleModule.h"
#include "TArc/Core/ControllerModule.h"
#include "TArc/Core/Deprecated.h"
#include "TArc/Core/Private/MacOSUtils.h"

#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/DataProcessing/PropertiesHolder.h"
#include "TArc/PluginsManager/TArcPlugin.h"
#include "TArc/PluginsManager/TArcPluginManager.h"
#include "TArc/Qt/QtIcon.h"
#include "TArc/Utils/AssertGuard.h"
#include "TArc/Utils/QtDelayedExecutor.h"
#include "TArc/Utils/QtMessageHandler.h"
#include "TArc/Utils/RhiEmptyFrame.h"
#include "TArc/Utils/ModuleCollection.h"
#include "TArc/WindowSubSystem/Private/UIManager.h"
#include "TArc/WindowSubSystem/Private/UIProxy.h"

#include <QtHelpers/CrashDumpHandler.h>

#include <Debug/DVAssert.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Engine/PlatformApiQt.h>
#include <Engine/Window.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/KeyedArchive.h>
#include <Functional/Function.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <Render/Renderer.h>
#include <Render/RenderHelper.h>
#include <UI/UIControlSystem.h>
#include <Utils/Utils.h>

#include <QApplication>
#include <QCloseEvent>
#include <QOffscreenSurface>
#include <QOpenGLContext>

namespace DAVA
{
class Core::Impl : public CoreInterface
{
public:
    Impl(Engine& engine_, Core* core_)
        : engine(engine_)
        , core(core_)
        , globalContext(new DataContext())
    {
        InitCrashDumpHandler();
    }

    ~Impl()
    {
        DVASSERT(contexts.empty());
        DVASSERT(globalContext == nullptr);
    }

    virtual void AddModule(ConsoleModule* module)
    {
        DVASSERT(false);
    }

    virtual void AddModule(ClientModule* module)
    {
        DVASSERT(false);
    }

    virtual void AddModule(ControllerModule* module)
    {
        DVASSERT(false);
    }

    bool IsConsoleMode() const
    {
        return engine.IsConsoleMode();
    }

    virtual void PostInit()
    {
        FileSystem* fileSystem = GetEngineContext()->fileSystem;
        DVASSERT(fileSystem != nullptr);
        propertiesHolder.reset(new PropertiesHolder("TArcProperties", fileSystem->GetCurrentDocumentsDirectory()));
    }

    virtual void OnLoopStarted()
    {
    }

    virtual void OnLoopStopped()
    {
        wrappersProcessor.Shoutdown();
        for (DataContext* context : contexts)
        {
            SafeDelete(context);
        }
        contexts.clear();
        SafeDelete(globalContext);

        if (pluginsManager != nullptr)
        {
            pluginsManager->UnloadPlugins();
            pluginsManager.reset();
        }
    }

    virtual void OnFrame(float32 delta)
    {
        if (isInFrame)
        {
            return;
        }

        isInFrame = true;
        SCOPE_EXIT
        {
            isInFrame = false;
        };

        if (syncRequested == false)
        {
            syncRequested = true;
            delayedExecutor.DelayedExecute([this]()
                                           {
                                               SyncWrappers();
                                           });
        }
    }

    virtual void OnWindowCreated(Window* w)
    {
    }

    void ForEachContext(const Function<void(DataContext&)>& functor) override
    {
        for (DataContext* context : contexts)
        {
            functor(*context);
        }
    }

    void ForEachContext(const Function<void(const DataContext&)>& functor) const override
    {
        for (const DataContext* context : contexts)
        {
            functor(*context);
        }
    }

    uint32 GetContextCount() const override
    {
        return static_cast<uint32>(contexts.size());
    }

    DataContext* GetGlobalContext() override
    {
        return globalContext;
    }

    DataContext* GetContext(DataContext::ContextID contextID) override
    {
        auto iter = std::find_if(contexts.begin(), contexts.end(), [contextID](const DataContext* context)
                                 {
                                     return context->GetID() == contextID;
                                 });

        if (iter == contexts.end())
        {
            return nullptr;
        }

        return *iter;
    }

    DataContext* GetActiveContext() override
    {
        return activeContext;
    }

    DataWrapper CreateWrapper(const ReflectedType* type) override
    {
        return wrappersProcessor.CreateWrapper(type, activeContext != nullptr ? activeContext : globalContext);
    }

    DataWrapper CreateWrapper(const DataWrapper::DataAccessor& accessor) override
    {
        return wrappersProcessor.CreateWrapper(accessor, activeContext != nullptr ? activeContext : globalContext);
    }

    PropertiesItem CreatePropertiesNode(const String& nodeName) const override
    {
        DVASSERT(propertiesHolder != nullptr);
        return propertiesHolder->CreateSubHolder(nodeName);
    }

    const PropertiesHolder& GetPropertiesHolder() override
    {
        return *propertiesHolder.get();
    }

    const EngineContext* GetEngineContext() override
    {
        const EngineContext* engineContext = engine.GetContext();
        DVASSERT(engineContext);
        return engineContext;
    }

    virtual UI* GetUI()
    {
        return nullptr;
    }

    void InitPluginManager(const String& applicationName, const String& pluginsFolder)
    {
        pluginsManager.reset(new TArcPluginManager(applicationName, pluginsFolder));
        Vector<String> errors;
        pluginsManager->LoadPlugins(errors);

        for (String errorMsg : errors)
        {
            DAVA::Logger::Warning(errorMsg.c_str());
        }
    }

    template <typename T>
    void LoadPluginModules()
    {
        if (pluginsManager == nullptr)
        {
            return;
        }

        Vector<TArcPlugin*> plugins = pluginsManager->GetPluginsWithBaseType(Type::Instance<T>());
        ModuleCollection* moduleCollection = ModuleCollection::Instance();
        for (TArcPlugin* plugin : plugins)
        {
            const ReflectedType* pluginType = plugin->GetModuleType();
            if (Type::Instance<T>() == Type::Instance<ConsoleModule>())
            {
                moduleCollection->AddConsoleModule([pluginType]() { return pluginType; });
            }
            else
            {
                moduleCollection->AddGuiModule([pluginType]() { return pluginType; });
            }
        }
    }

    virtual void BeforeContextSwitch(DataContext* currentContext, DataContext* newOne)
    {
    }
    virtual void AfterContextSwitch(DataContext* currentContext, DataContext* oldOne)
    {
    }

    const Vector<DataContext*>& GetContexts() const override
    {
        return contexts;
    }

    void SetActiveContext(DataContext* ctx) override
    {
        ActivateContext(ctx->GetID());
    }

    void ActivateContextImpl(DataContext* context)
    {
        if (context == activeContext)
        {
            return;
        }
        BeforeContextSwitch(activeContext, context);
        DataContext* oldContext = activeContext;
        activeContext = context;
        wrappersProcessor.SetContext(activeContext != nullptr ? activeContext : globalContext);
        AfterContextSwitch(activeContext, oldContext);
        SyncWrappers();
    }

    void SyncWrappers()
    {
        syncRequested = false;

        wrappersProcessor.Sync();
        core->syncSignal.Emit();
    }

    Engine& engine;
    Core* core;
    std::unique_ptr<TArcPluginManager> pluginsManager;

    DataContext* globalContext = nullptr;
    Vector<DataContext*> contexts;
    DataContext* activeContext = nullptr;
    DataWrappersProcessor wrappersProcessor;
    bool isInFrame = false;

    std::unique_ptr<PropertiesHolder> propertiesHolder;
    QtDelayedExecutor delayedExecutor;
    bool syncRequested = false;
};

class Core::ConsoleImpl : public Core::Impl
{
public:
    ConsoleImpl(Engine& engine, Core* core)
        : Impl(engine, core)
    {
        DVASSERT(engine.IsConsoleMode() == true);
        argv = engine.GetCommandLineAsArgv();
        argc = static_cast<int>(argv.size());
    }

    ~ConsoleImpl()
    {
        delete context;
        delete surface;
        delete application;
    }

    void OnLoopStarted() override
    {
        Impl::OnLoopStarted();
        application = new QGuiApplication(argc, argv.data());
        surface = new QOffscreenSurface();
        surface->create();

        context = new QOpenGLContext();
        if (context->create() == false)
        {
            throw std::runtime_error("OGL context creation failed");
        }

        if (context->makeCurrent(surface) == false)
        {
            throw std::runtime_error("MakeCurrent for offscreen surface failed");
        }

        rhi::Api renderer = rhi::RHI_GLES2;
        rhi::InitParam rendererParams;
        rendererParams.threadedRenderFrameCount = 1;
        rendererParams.threadedRenderEnabled = false;
        rendererParams.acquireContextFunc = []() {};
        rendererParams.releaseContextFunc = []() {};

        const KeyedArchive* options = engine.GetOptions();

        rendererParams.maxIndexBufferCount = options->GetInt32("max_index_buffer_count");
        rendererParams.maxVertexBufferCount = options->GetInt32("max_vertex_buffer_count");
        rendererParams.maxConstBufferCount = options->GetInt32("max_const_buffer_count");
        rendererParams.maxTextureCount = options->GetInt32("max_texture_count");

        rendererParams.maxTextureSetCount = options->GetInt32("max_texture_set_count");
        rendererParams.maxSamplerStateCount = options->GetInt32("max_sampler_state_count");
        rendererParams.maxPipelineStateCount = options->GetInt32("max_pipeline_state_count");
        rendererParams.maxDepthStencilStateCount = options->GetInt32("max_depthstencil_state_count");
        rendererParams.maxRenderPassCount = options->GetInt32("max_render_pass_count");
        rendererParams.maxCommandBuffer = options->GetInt32("max_command_buffer_count");
        rendererParams.maxPacketListCount = options->GetInt32("max_packet_list_count");

        rendererParams.shaderConstRingBufferSize = options->GetInt32("shader_const_buffer_size");

        rendererParams.window = nullptr;
        rendererParams.width = 1024;
        rendererParams.height = 768;
        rendererParams.scaleX = 1.0f;
        rendererParams.scaleY = 1.0f;
        Renderer::Initialize(renderer, rendererParams);

        const EngineContext* engineContext = engine.GetContext();
        VirtualCoordinatesSystem* vcs = engineContext->uiControlSystem->vcs;
        vcs->SetInputScreenAreaSize(rendererParams.width, rendererParams.height);
        vcs->SetPhysicalScreenSize(rendererParams.width, rendererParams.height);
        vcs->SetVirtualScreenSize(rendererParams.width, rendererParams.height);
        vcs->UnregisterAllAvailableResourceSizes();
        vcs->RegisterAvailableResourceSize(rendererParams.width, rendererParams.height, "Gfx");
        vcs->ScreenSizeChanged();

        Texture::SetGPULoadingOrder({ GPU_ORIGIN });

        ActivateContextImpl(globalContext);
        for (std::unique_ptr<ConsoleModule>& module : modules)
        {
            module->Init(this);
        }

        RhiEmptyFrame frame;
        for (std::unique_ptr<ConsoleModule>& module : modules)
        {
            module->PostInit();
        }
    }

    void OnFrame(float32 delta) override
    {
        context->makeCurrent(surface);
        Impl::OnFrame(delta);
        {
            RhiEmptyFrame frame;
            if (modules.front()->OnFrame() == ConsoleModule::eFrameResult::FINISHED)
            {
                if (exitCode == 0)
                {
                    exitCode = modules.front()->GetExitCode();
                }

                modules.front()->BeforeDestroyed();
                modules.pop_front();
            }

            if (modules.empty() == true)
            {
                engine.QuitAsync(exitCode);
            }
        }
        context->swapBuffers(surface);
    }

    void OnLoopStopped() override
    {
        DVASSERT(modules.empty());
        ActivateContextImpl(nullptr);

        rhi::ResetParam rendererParams;
        rendererParams.window = nullptr;
        rendererParams.width = 0;
        rendererParams.height = 0;
        rendererParams.scaleX = 1.f;
        rendererParams.scaleY = 1.f;
        Renderer::Reset(rendererParams);

        context->doneCurrent();
        surface->destroy();

        Impl::OnLoopStopped();
    }

    void PostInit() override
    {
        Impl::PostInit();
        LoadPluginModules<ConsoleModule>();
    }

    void AddModule(ConsoleModule* module) override
    {
        modules.push_back(std::unique_ptr<ConsoleModule>(module));
    }

private:
    void RegisterOperation(uint32 operationID, AnyFn&& fn) override
    {
    }
    DataContext::ContextID CreateContext(Vector<std::unique_ptr<TArcDataNode>>&& initialData) override
    {
        DVASSERT(false);
        return DataContext::ContextID();
    }
    void DeleteContext(DataContext::ContextID contextID) override
    {
    }
    void ActivateContext(DataContext::ContextID contextID) override
    {
    }
    RenderWidget* GetRenderWidget() const override
    {
        return nullptr;
    }

    void Invoke(uint32 operationId) override
    {
    }
    void Invoke(uint32 operationId, const Any& a) override
    {
    }
    void Invoke(uint32 operationId, const Any& a1, const Any& a2) override
    {
    }
    void Invoke(uint32 operationId, const Any& a1, const Any& a2, const Any& a3) override
    {
    }
    void Invoke(uint32 operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4) override
    {
    }
    void Invoke(uint32 operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5) override
    {
    }
    void Invoke(uint32 operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5, const Any& a6) override
    {
    }

    void RegisterInterface(ClientModule* module, const Type* lookupType, Any interface) override
    {
    }

    Any QueryInterface(const Type* lookupType) const override
    {
        return Any();
    }

private:
    Deque<std::unique_ptr<ConsoleModule>> modules;
    QGuiApplication* application = nullptr;
    QOffscreenSurface* surface = nullptr;
    QOpenGLContext* context = nullptr;
    int argc = 0;
    Vector<char*> argv;
    int exitCode = 0;
};

class Core::GuiImpl : public Core::Impl, public UIManager::Delegate
{
public:
    GuiImpl(Engine& engine, Core* core)
        : Impl(engine, core)
    {
#if defined(__DAVAENGINE_MACOS__)
        MakeAppForeground();
        FixOSXFonts();
#endif
        DVASSERT(engine.IsConsoleMode() == false);
    }

    ~GuiImpl()
    {
        DVASSERT(modules.empty());
    }

    void AddModule(ClientModule* module) override
    {
        modules.emplace_back(module);
    }

    void AddModule(ControllerModule* module) override
    {
        DVASSERT(controllerModule == nullptr);
        controllerModule = module;
        AddModule(static_cast<ClientModule*>(module));
    }

    void OnLoopStarted() override
    {
        qInstallMessageHandler(&DAVAMessageHandler);
        Impl::OnLoopStarted();

        PlatformApi::Qt::GetApplication()->setWindowIcon(QIcon(":/icons/appIcon.ico"));
        uiManager.reset(new UIManager(this, this, propertiesHolder->CreateSubHolder("UIManager")));
        DVASSERT(controllerModule != nullptr, "Controller Module hasn't been registered");

        for (std::unique_ptr<ClientModule>& module : modules)
        {
            module->Init(this, std::make_unique<UIProxy>(module.get(), uiManager.get()));
        }

        for (std::unique_ptr<ClientModule>& module : modules)
        {
            module->PostInit();
        }

        for (const auto& interfaceNode : interfaces)
        {
            for (std::unique_ptr<ClientModule>& module : modules)
            {
                module->OnInterfaceRegistered(interfaceNode.first);
            }
        }

        uiManager->InitializationFinished();
#if defined(__DAVAENGINE_MACOS__)
        RestoreMenuBar();
#endif
    }

    void OnFrame(float32 delta) override
    {
        Impl::OnFrame(delta);
    }

    void OnLoopStopped() override
    {
        for (const auto& interfaceNode : interfaces)
        {
            for (std::unique_ptr<ClientModule>& module : modules)
            {
                module->OnBeforeInterfaceUnregistered(interfaceNode.first);
            }
        }

        interfaces.clear();
        for (std::unique_ptr<ClientModule>& module : modules)
        {
            uiManager->ModuleDestroyed(module.get());
        }

        ActivateContextImpl(nullptr);
        controllerModule = nullptr;
        for (DataContext* context : contexts)
        {
            for (std::unique_ptr<ClientModule>& module : modules)
            {
                module->OnContextDeleted(context);
            }
        }

        modules.clear();
        uiManager.reset();
        Impl::OnLoopStopped();
    }

    void PostInit() override
    {
        Impl::PostInit();
        LoadPluginModules<ClientModule>();
    }

    void OnWindowCreated(Window* w) override
    {
        Impl::OnWindowCreated(w);
        controllerModule->OnRenderSystemInitialized(w);
    }

    void RegisterOperation(uint32 operationID, AnyFn&& fn) override
    {
        auto iter = globalOperations.find(operationID);
        if (iter != globalOperations.end())
        {
            Logger::Error("Global operation with ID %d, has already been registered", operationID);
        }

        globalOperations.emplace(operationID, fn);
    }

    DataContext::ContextID CreateContext(Vector<std::unique_ptr<TArcDataNode>>&& initialData) override
    {
        contexts.push_back(new DataContext(globalContext));
        DataContext* context = contexts.back();

        for (std::unique_ptr<TArcDataNode>& data : initialData)
        {
            context->CreateData(std::move(data));
        }
        initialData.clear();

        for (std::unique_ptr<ClientModule>& module : modules)
        {
            module->OnContextCreated(context);
        }

        return context->GetID();
    }

    void DeleteContext(DataContext::ContextID contextID) override
    {
        auto iter = std::find_if(contexts.begin(), contexts.end(), [contextID](const DataContext* context)
                                 {
                                     return context->GetID() == contextID;
                                 });

        if (iter == contexts.end())
        {
            throw std::runtime_error(Format("DeleteContext failed for contextID : %d", contextID));
        }

        if (activeContext != nullptr && activeContext->GetID() == contextID)
        {
            ActivateContextImpl(nullptr);
        }

        for (std::unique_ptr<ClientModule>& module : modules)
        {
            module->OnContextDeleted(*iter);
        }
        SafeDelete(*iter);
        contexts.erase(iter);
    }

    void ActivateContext(DataContext::ContextID contextID) override
    {
        if (activeContext != nullptr && activeContext->GetID() == contextID)
        {
            return;
        }

        if (activeContext == nullptr && contextID == DataContext::Empty)
        {
            return;
        }

        if (contextID == DataContext::Empty)
        {
            ActivateContextImpl(nullptr);
            return;
        }

        auto iter = std::find_if(contexts.begin(), contexts.end(), [contextID](const DataContext* context)
                                 {
                                     return context->GetID() == contextID;
                                 });

        if (iter == contexts.end())
        {
            throw std::runtime_error(Format("ActivateContext failed for contextID : %d", contextID));
        }

        ActivateContextImpl(*iter);
    }

    RenderWidget* GetRenderWidget() const override
    {
        return PlatformApi::Qt::GetRenderWidget();
    }

    void Invoke(uint32 operationId) override
    {
        InvokeImpl(operationId);
    }

    void Invoke(uint32 operationId, const Any& a) override
    {
        InvokeImpl(operationId, a);
    }
    void Invoke(uint32 operationId, const Any& a1, const Any& a2) override
    {
        InvokeImpl(operationId, a1, a2);
    }

    void Invoke(uint32 operationId, const Any& a1, const Any& a2, const Any& a3) override
    {
        InvokeImpl(operationId, a1, a2, a3);
    }

    void Invoke(uint32 operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4) override
    {
        InvokeImpl(operationId, a1, a2, a3, a4);
    }

    void Invoke(uint32 operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5) override
    {
        InvokeImpl(operationId, a1, a2, a3, a4, a5);
    }

    void Invoke(uint32 operationId, const Any& a1, const Any& a2, const Any& a3, const Any& a4, const Any& a5, const Any& a6) override
    {
        InvokeImpl(operationId, a1, a2, a3, a4, a5, a6);
    }

    template <typename... Args>
    void InvokeImpl(uint32 operationId, const Args&... args)
    {
        if (invokeListener != nullptr)
        {
            invokeListener->Invoke(operationId, args...);
        }

        AnyFn fn = FindOperation(operationId);
        if (!fn.IsValid())
        {
            Logger::Error("Operation with ID %d has not been registered yet", operationId);
            return;
        }

        try
        {
            fn.Invoke(args...);
        }
        catch (const Exception& e)
        {
            Logger::Error("Operation (%d) call failed: %s", operationId, e.what());
        }
    }

    void RegisterInterface(ClientModule* module, const Type* lookupType, Any interface) override
    {
        DVASSERT(interface.GetType() == lookupType->Pointer());
        interfaces[lookupType] = std::make_pair(module, interface);
    }

    Any QueryInterface(const Type* lookupType) const override
    {
        auto iter = interfaces.find(lookupType);
        if (iter == interfaces.end())
        {
            return Any();
        }

        return iter->second.second;
    }

    bool WindowCloseRequested(const WindowKey& key) override
    {
        DVASSERT(controllerModule != nullptr);
        bool result = true;
        QCloseEvent closeEvent;
        String requestWindowText;
        if (controllerModule->ControlWindowClosing(key, &closeEvent))
        {
            result = closeEvent.isAccepted();
        }
        else if (controllerModule->CanWindowBeClosedSilently(key, requestWindowText) == false)
        {
            if (requestWindowText.empty())
            {
                requestWindowText = "Some files have been modified\nDo you want to save changes?";
            }
            ModalMessageParams params;
            params.buttons = ModalMessageParams::Buttons(ModalMessageParams::SaveAll | ModalMessageParams::NoToAll | ModalMessageParams::Cancel);
            params.message = QString::fromStdString(requestWindowText);
            params.title = "Save Changes?";
            ModalMessageParams::Button resultButton = uiManager->ShowModalMessage(key, params);
            if (resultButton == ModalMessageParams::SaveAll)
            {
                result = controllerModule->SaveOnWindowClose(key);
            }
            else if (resultButton == ModalMessageParams::NoToAll)
            {
                controllerModule->RestoreOnWindowClose(key);
                result = true;
            }
            else
            {
                result = false;
            }
        }

        return result;
    }

    void OnWindowClosed(const WindowKey& key) override
    {
        controllerModule->OnWindowClosed(key);

        std::for_each(modules.begin(), modules.end(), [&key, this](std::unique_ptr<ClientModule>& module) {
            if (module.get() != controllerModule)
            {
                module->OnWindowClosed(key);
            }
        });
    }

    bool HasControllerModule() const
    {
        return controllerModule != nullptr;
    }

    UI* GetUI() override
    {
        return uiManager.get();
    }

    void SetInvokeListener(OperationInvoker* invoker)
    {
        invokeListener = invoker;
    }

private:
    void BeforeContextSwitch(DataContext* currentContext, DataContext* newOne) override
    {
        for (std::unique_ptr<ClientModule>& module : modules)
        {
            module->OnContextWillBeChanged(currentContext, newOne);
        }
    }

    void AfterContextSwitch(DataContext* currentContext, DataContext* oldOne) override
    {
        for (std::unique_ptr<ClientModule>& module : modules)
        {
            module->OnContextWasChanged(currentContext, oldOne);
        }
    }

    AnyFn FindOperation(int operationId)
    {
        AnyFn operation;
        auto iter = globalOperations.find(operationId);
        if (iter != globalOperations.end())
        {
            operation = iter->second;
        }

        return operation;
    }

private:
    Vector<std::unique_ptr<ClientModule>> modules;
    ControllerModule* controllerModule = nullptr;

    UnorderedMap<int, AnyFn> globalOperations;
    UnorderedMap<const Type*, std::pair<ClientModule*, Any>> interfaces;

    std::unique_ptr<UIManager> uiManager;
    OperationInvoker* invokeListener = nullptr;
};

Core::Core(Engine& engine)
    : Core(engine, true)
{
    Deprecated::InitTArcCore(this);
}

Core::Core(Engine& engine, bool connectSignals)
{
    Deprecated::InitTArcCore(this);
    if (engine.IsConsoleMode())
    {
        impl.reset(new ConsoleImpl(engine, this));
    }
    else
    {
        impl.reset(new GuiImpl(engine, this));
    }

    if (connectSignals)
    {
        engine.update.Connect(this, &Core::OnFrame);
        engine.gameLoopStarted.Connect(this, &Core::OnLoopStarted);
        engine.gameLoopStopped.Connect(this, &Core::OnLoopStopped);
        engine.windowCreated.Connect(this, &Core::OnWindowCreated);
    }
}

Core::~Core()
{
    Deprecated::InitTArcCore(nullptr);
}

const EngineContext* Core::GetEngineContext()
{
    return impl->GetEngineContext();
}

CoreInterface* Core::GetCoreInterface()
{
    return impl.get();
}

const CoreInterface* Core::GetCoreInterface() const
{
    return impl.get();
}

UI* Core::GetUI()
{
    return impl->GetUI();
}

bool Core::IsConsoleMode() const
{
    return impl->IsConsoleMode();
}

void Core::AddModule(ConsoleModule* module)
{
    impl->AddModule(module);
}

void Core::AddModule(ClientModule* module)
{
    impl->AddModule(module);
}

void Core::AddModule(ControllerModule* module)
{
    impl->AddModule(module);
}

void Core::OnLoopStarted()
{
    DVASSERT(impl);
    impl->OnLoopStarted();
}

void Core::OnLoopStopped()
{
    DVASSERT(impl);
    impl->OnLoopStopped();
}

void Core::OnFrame(float32 delta)
{
    DVASSERT(impl);
    impl->OnFrame(delta);
}

void Core::OnWindowCreated(Window* w)
{
    DVASSERT(impl);
    impl->OnWindowCreated(w);
}

void Core::OnTestClassShoutdown()
{
    DVASSERT(impl);
    impl->wrappersProcessor.Shoutdown();
}

bool Core::HasControllerModule() const
{
    GuiImpl* guiImpl = dynamic_cast<GuiImpl*>(impl.get());
    return guiImpl != nullptr && guiImpl->HasControllerModule();
}

void Core::SetInvokeListener(OperationInvoker* proxyInvoker)
{
    GuiImpl* guiImpl = dynamic_cast<GuiImpl*>(impl.get());
    DVASSERT(guiImpl != nullptr);
    guiImpl->SetInvokeListener(proxyInvoker);
}

void Core::PostInit()
{
    impl->PostInit();
}

void Core::InitPluginManager(const String& applicationName, const String& pluginsFolder)
{
    impl->InitPluginManager(applicationName, pluginsFolder);
}
} // namespace DAVA
