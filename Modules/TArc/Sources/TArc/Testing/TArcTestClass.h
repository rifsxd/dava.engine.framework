#pragma once

#include "TArc/Core/Core.h"
#include "TArc/Testing/MockInvoker.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Utils/QtDelayedExecutor.h"

#include <UnitTests/TestClass.h>
#include <UnitTests/UnitTests.h>

#include <QList>
#include <QWidget>

namespace DAVA
{
class TArcTestClass : public UnitTests::TestClass
{
    static const double testTimeLimit;

public:
    ~TArcTestClass();

    void Init();
    void DirectUpdate(float32 timeElapsed, const String& testName);
    bool DirectTestComplete(const String& testName) const;

    virtual void WriteInitialSettings()
    {
    }
    virtual void CreateTestedModules();

    UI* GetUI();
    MockInvoker* GetMockInvoker();
    DataContext* GetActiveContext();
    const DataContext* GetActiveContext() const;
    DataContext* GetGlobalContext();
    const DataContext* GetGlobalContext() const;
    DataWrapper CreateWrapper(const ReflectedType* type);
    ContextAccessor* GetAccessor();
    const ContextAccessor* GetAccessor() const;
    ContextManager* GetContextManager();
    const ContextManager* GetContextManager() const;

    PropertiesItem CreatePropertiesItem(const String& name) const;

    template <typename... Args>
    void InvokeOperation(int operationId, const Args&... args);

    QWidget* GetWindow(const WindowKey& wndKey) const;
    QList<QWidget*> LookupWidget(const WindowKey& wndKey, const QString& objectName) const;
    QWidget* GetRenderWidgetTestTarget();
    void SelectByMouseRect(QWidget* widget, QPoint startPoint, QPoint endPoint);

    template <typename T>
    T* LookupSingleWidget(const WindowKey& wndKey, const QString& objectName) const
    {
        QList<QWidget*> widgetsList = LookupWidget(wndKey, objectName);
        T* result = nullptr;
        foreach (QWidget* w, widgetsList)
        {
            TEST_VERIFY(result == nullptr);
            result = qobject_cast<T*>(w);
            if (result != nullptr)
            {
                break;
            }
        }
        TEST_VERIFY(result != nullptr);
        return result;
    }

protected:
    virtual void AfterWrappersSync()
    {
    }

    std::unique_ptr<Core> core;
    std::unique_ptr<MockInvoker> mockInvoker;
    QtConnections connections;
    bool updateForCurrentTestCalled = false;
    FilePath documentsPath;
};

template <typename... Args>
inline void TArcTestClass::InvokeOperation(int operationId, const Args&... args)
{
    core->GetCoreInterface()->Invoke(operationId, args...);
}

class TArcTestClassHolder final : public UnitTests::TestClass
{
public:
    TArcTestClassHolder(std::unique_ptr<TArcTestClass>&& testClass);

    void InitTimeStampForTest(const String& testName) override;
    void SetUp(const String& testName) override;
    void TearDown(const String& testName) override;
    void Update(float32 timeElapsed, const String& testName) override;
    bool TestComplete(const String& testName) const override;
    UnitTests::TestCoverageInfo FilesCoveredByTests() const override;

    const String& TestName(size_t index) const override;
    size_t TestCount() const override;
    void RunTest(size_t index) override;

private:
    void AddCall(const Function<void()>& call) const;
    void AddCallImpl(const Function<void()>& call);
    void ProcessCalls() const;
    void ProcessCallsImpl();

private:
    std::unique_ptr<TArcTestClass> testClass;
    bool pendingEventProcess = false;
    Vector<Function<void()>> callsQueue;
    QtDelayedExecutor executor;
    bool currentTestFinished = false;
};

template <typename T>
class TArcTestClassHolderFactory : public UnitTests::TestClassFactoryBase
{
public:
    UnitTests::TestClass* CreateTestClass()
    {
        return new TArcTestClassHolder(std::make_unique<T>());
    }
};
} // namespace DAVA
