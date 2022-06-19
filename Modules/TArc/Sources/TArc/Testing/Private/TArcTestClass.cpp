#include "TArc/Core/ControllerModule.h"
#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/MockInvoker.h"
#include "TArc/WindowSubSystem/UI.h"

#include "TArc/SharedModules/SettingsModule/SettingsModule.h"
#include "TArc/SharedModules/ThemesModule/ThemesModule.h"

#include <Debug/DebuggerDetection.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Engine/PlatformApiQt.h>
#include <FileSystem/FileSystem.h>
#include <UnitTests/UnitTests.h>

#include <gmock/gmock-spec-builders.h>
#include <QAbstractEventDispatcher>
#include <QApplication>
#include <QTest>
#include <QTimer>

namespace DAVA
{
namespace TArcTestClassDetail
{
class TestControllerModule : public ControllerModule
{
protected:
    void OnRenderSystemInitialized(Window* w) override
    {
    }

    bool CanWindowBeClosedSilently(const WindowKey& key, String& requestWindowText) override
    {
        return true;
    }

    bool SaveOnWindowClose(const WindowKey& key) override
    {
        return true;
    }

    void RestoreOnWindowClose(const WindowKey& key) override
    {
    }

    void OnContextCreated(DataContext* context) override
    {
    }

    void OnContextDeleted(DataContext* context) override
    {
    }

    void PostInit() override
    {
        ContextManager* ctxManager = GetContextManager();
        DataContext::ContextID id = ctxManager->CreateContext(Vector<std::unique_ptr<TArcDataNode>>());
        ctxManager->ActivateContext(id);
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(TestControllerModule, ControllerModule)
    {
        ReflectionRegistrator<TestControllerModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};
}

const double TArcTestClass::testTimeLimit = 10.0; // seconds

TArcTestClass::~TArcTestClass()
{
    DVASSERT(core != nullptr);

    QWidget* focusWidget = PlatformApi::Qt::GetApplication()->focusWidget();
    if (focusWidget != nullptr)
    {
        focusWidget->clearFocus();
    }

    FilePath prevDocPath = documentsPath;
    Core* c = core.release();
    c->OnTestClassShoutdown();
    c->syncSignal.DisconnectAll();
    c->SetInvokeListener(nullptr);
    mockInvoker.reset();
    QTimer::singleShot(0, [c, prevDocPath]()
                       {
                           RenderWidget* widget = PlatformApi::Qt::GetRenderWidget();
                           DVASSERT(widget != nullptr);
                           widget->SetFrameBlocked(true);
                           widget->setParent(nullptr); // remove it from Qt hierarchy to avoid Widget deletion.
                           widget->show();
                           widget->SetFrameBlocked(false);

                           c->OnLoopStopped();
                           delete c;

                           const EngineContext* ctx = GetEngineContext();
                           FilePath tmpDirectory = ctx->fileSystem->GetCurrentDocumentsDirectory();
                           if (ctx->fileSystem->Exists(tmpDirectory))
                           {
                               ctx->fileSystem->DeleteDirectory(tmpDirectory, true);
                           }
                           ctx->fileSystem->SetCurrentDocumentsDirectory(prevDocPath);
                       });
}

void TArcTestClass::Init()
{
    updateForCurrentTestCalled = false;
    if (core == nullptr)
    {
        const EngineContext* ctx = GetEngineContext();
        documentsPath = ctx->fileSystem->GetCurrentDocumentsDirectory();
        FilePath tmpDirectory = ctx->fileSystem->GetTempDirectoryPath() + "/SelfTestFolder/";
        if (ctx->fileSystem->Exists(tmpDirectory))
        {
            ctx->fileSystem->DeleteDirectory(tmpDirectory, true);
        }

        ctx->fileSystem->CreateDirectory(tmpDirectory, true);
        ctx->fileSystem->SetCurrentDocumentsDirectory(tmpDirectory);

        WriteInitialSettings();

        using namespace std::chrono;
        TestInfo::TimePoint startTimePoint = TestInfo::Clock::now();
        auto timeoutCrashHandler = [startTimePoint]()
        {
            double elapsedSeconds = duration_cast<duration<double>>(TestInfo::Clock::now() - startTimePoint).count();
            if (elapsedSeconds > 10 * 60) // 10 minutes
            {
                TEST_VERIFY_WITH_MESSAGE(false, "Timeout fail");
                std::terminate();
            }
        };

        QAbstractEventDispatcher* dispatcher = qApp->eventDispatcher();
        connections.AddConnection(dispatcher, &QAbstractEventDispatcher::aboutToBlock, timeoutCrashHandler);
        connections.AddConnection(dispatcher, &QAbstractEventDispatcher::awake, timeoutCrashHandler);

        Engine* e = Engine::Instance();
        DVASSERT(e != nullptr);
        DVASSERT(e->IsConsoleMode() == false);
        core.reset(new Core(*e, false));

        mockInvoker.reset(new MockInvoker());
        core->SetInvokeListener(mockInvoker.get());

        core->PostInit();

        InitColorPickerOptions(false);
        core->CreateModule<SettingsModule>();
        core->CreateModule<ThemesModule>();
        CreateTestedModules();
        if (!core->HasControllerModule())
        {
            core->CreateModule<TArcTestClassDetail::TestControllerModule>();
        }

        core->OnLoopStarted();
        Window* w = e->PrimaryWindow();
        DVASSERT(w);
        core->OnWindowCreated(w);
        core->syncSignal.Connect(this, &TArcTestClass::AfterWrappersSync);
    }
}

void TArcTestClass::DirectUpdate(float32 timeElapsed, const String& testName)
{
    if (core != nullptr)
    {
        core->OnFrame(timeElapsed);
        updateForCurrentTestCalled = true;
    }
}

bool TArcTestClass::DirectTestComplete(const String& testName) const
{
    if (core == nullptr)
    {
        return false;
    }

    if (core->GetUI()->HasActiveWaitDalogues() == true)
    {
        return false;
    }

    auto iter = std::find_if(tests.begin(), tests.end(), [&testName](const TestInfo& testInfo)
                             {
                                 return testInfo.name == testName;
                             });

    DVASSERT(iter != tests.end());
    using namespace std::chrono;
    double elapsedSeconds = duration_cast<duration<double>>(TestInfo::Clock::now() - iter->startTime).count();
    bool checkTimeLimit = true;
    checkTimeLimit = !IsDebuggerPresent();
    if (checkTimeLimit == true && elapsedSeconds > testTimeLimit)
    {
        TEST_VERIFY(::testing::Mock::VerifyAndClear());
        return true;
    }

    bool hasNotSatisfied = ::testing::Mock::HasNotSatisfiedExpectation();
    if (hasNotSatisfied == false)
    {
        TEST_VERIFY(::testing::Mock::VerifyAndClear());
    }
    return !hasNotSatisfied && updateForCurrentTestCalled;
}

MockInvoker* TArcTestClass::GetMockInvoker()
{
    return mockInvoker.get();
}

DataContext* TArcTestClass::GetActiveContext()
{
    return core->GetCoreInterface()->GetActiveContext();
}

const DataContext* TArcTestClass::GetActiveContext() const
{
    const Core* corePtr = core.get();
    return corePtr->GetCoreInterface()->GetActiveContext();
}

DataContext* TArcTestClass::GetGlobalContext()
{
    return core->GetCoreInterface()->GetGlobalContext();
}

const DataContext* TArcTestClass::GetGlobalContext() const
{
    const Core* corePtr = core.get();
    return corePtr->GetCoreInterface()->GetGlobalContext();
}

DataWrapper TArcTestClass::CreateWrapper(const ReflectedType* type)
{
    return core->GetCoreInterface()->CreateWrapper(type);
}

UI* TArcTestClass::GetUI()
{
    return core->GetUI();
}

ContextAccessor* TArcTestClass::GetAccessor()
{
    return core->GetCoreInterface();
}

const ContextAccessor* TArcTestClass::GetAccessor() const
{
    const Core* corePtr = core.get();
    return corePtr->GetCoreInterface();
}

ContextManager* TArcTestClass::GetContextManager()
{
    return core->GetCoreInterface();
}

const ContextManager* TArcTestClass::GetContextManager() const
{
    const Core* corePtr = core.get();
    return corePtr->GetCoreInterface();
}

PropertiesItem TArcTestClass::CreatePropertiesItem(const String& name) const
{
    return core->GetCoreInterface()->CreatePropertiesNode(name);
}

QWidget* TArcTestClass::GetWindow(const WindowKey& wndKey) const
{
    UI* manager = core->GetUI();
    QWidget* wnd = manager->GetWindow(wndKey);

    return wnd;
}

QList<QWidget*> TArcTestClass::LookupWidget(const WindowKey& wndKey, const QString& objectName) const
{
    return GetWindow(wndKey)->findChildren<QWidget*>(objectName);
}

QWidget* TArcTestClass::GetRenderWidgetTestTarget()
{
    return GetContextManager()->GetRenderWidget()->findChild<QWidget*>(RenderWidget::BackendWidgetName);
}

void TArcTestClass::SelectByMouseRect(QWidget* widget, QPoint startPoint, QPoint endPoint)
{
    QTest::mousePress(widget, Qt::LeftButton, Qt::KeyboardModifiers(), startPoint);
    QMouseEvent event((QEvent::MouseMove), endPoint, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(widget, &event);
    if (QCursor::pos() != endPoint)
        QCursor::setPos(endPoint);

    QTest::mouseRelease(widget, Qt::LeftButton, Qt::KeyboardModifiers(), endPoint);
}

void TArcTestClass::CreateTestedModules()
{
}

TArcTestClassHolder::TArcTestClassHolder(std::unique_ptr<TArcTestClass>&& testClass_)
    : testClass(std::move(testClass_))
{
}

void TArcTestClassHolder::InitTimeStampForTest(const String& testName)
{
    testClass->InitTimeStampForTest(testName);
}

void TArcTestClassHolder::SetUp(const String& testName)
{
    currentTestFinished = false;
    AddCall([this, testName]()
            {
                testClass->Init();
                testClass->SetUp(testName);
            });
}

void TArcTestClassHolder::TearDown(const String& testName)
{
    DVASSERT(currentTestFinished == true);
    testClass->TearDown(testName);
}

void TArcTestClassHolder::Update(float32 timeElapsed, const String& testName)
{
    if (currentTestFinished == true)
    {
        return;
    }

    testClass->DirectUpdate(timeElapsed, testName);
    AddCall([this, timeElapsed, testName]()
            {
                testClass->Update(timeElapsed, testName);
            });
}

bool TArcTestClassHolder::TestComplete(const String& testName) const
{
    if (currentTestFinished == true)
    {
        return true;
    }

    TArcTestClassHolder* nonConst = const_cast<TArcTestClassHolder*>(this);
    AddCall([nonConst, testName]()
            {
                bool testCompleted = nonConst->testClass->TestComplete(testName);
                if (testCompleted == true)
                {
                    testCompleted = nonConst->testClass->DirectTestComplete(testName);
                }

                nonConst->currentTestFinished = testCompleted;
            });

    return false;
}

UnitTests::TestCoverageInfo TArcTestClassHolder::FilesCoveredByTests() const
{
    return testClass->FilesCoveredByTests();
}

const String& TArcTestClassHolder::TestName(size_t index) const
{
    return testClass->TestName(index);
}

size_t TArcTestClassHolder::TestCount() const
{
    return testClass->TestCount();
}

void TArcTestClassHolder::RunTest(size_t index)
{
    AddCall([this, index]()
            {
                testClass->RunTest(index);
            });
}

void TArcTestClassHolder::AddCall(const Function<void()>& call) const
{
    const_cast<TArcTestClassHolder*>(this)->AddCallImpl(call);
}

void TArcTestClassHolder::AddCallImpl(const Function<void()>& call)
{
    callsQueue.push_back(call);
    if (pendingEventProcess == false)
    {
        pendingEventProcess = true;
        executor.DelayedExecute(MakeFunction(this, &TArcTestClassHolder::ProcessCallsImpl));
    }
}

void TArcTestClassHolder::ProcessCalls() const
{
    const_cast<TArcTestClassHolder*>(this)->ProcessCallsImpl();
}

void TArcTestClassHolder::ProcessCallsImpl()
{
    DVASSERT(pendingEventProcess == true);
    Vector<Function<void()>> queue = callsQueue;
    callsQueue.clear();
    pendingEventProcess = false;

    for (const Function<void()>& fn : queue)
    {
        fn();
        if (currentTestFinished == true)
        {
            callsQueue.clear();
            break;
        }
    }
}
} // namespace DAVA
