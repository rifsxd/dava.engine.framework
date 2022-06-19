#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockListener.h"

#include "TArc/DataProcessing/TArcDataNode.h"
#include "Reflection/ReflectionRegistrator.h"

class DataWrapperTestData : public DAVA::TArcDataNode
{
public:
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(DataWrapperTestData, DAVA::TArcDataNode)
    {
        DAVA::ReflectionRegistrator<DataWrapperTestData>::Begin()
        .End();
    }
};

DAVA_TARC_TESTCLASS(DataWrapperTest)
{
    struct DummyHolder
    {
        DAVA::DataWrapper wrapper;
    };
    BEGIN_FILES_COVERED_BY_TESTS()
    FIND_FILES_IN_TARGET(TArc)
    DECLARE_COVERED_FILES("DataWrapper.cpp")
    DECLARE_COVERED_FILES("DataListener.cpp")
    END_FILES_COVERED_BY_TESTS()

    DAVA_TEST (DestroyDataWrapperTest)
    {
        using namespace ::testing;
        DAVA::DataContext* ctx = GetActiveContext();
        ctx->CreateData(std::make_unique<DataWrapperTestData>());

        holder = new DummyHolder();
        holder->wrapper = CreateWrapper(DAVA::ReflectedTypeDB::Get<DataWrapperTestData>());
        holder->wrapper.SetListener(&listener);

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(this, &DataWrapperTest::FirstAfterSync));
    }

    void FirstAfterSync()
    {
        using namespace ::testing;
        TEST_VERIFY(listener.HasWrappers() == true);
        delete holder;
        holder = nullptr;

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(this, &DataWrapperTest::SecondAfterSync));
    }

    void SecondAfterSync()
    {
        TEST_VERIFY(listener.HasWrappers() == false);
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    DummyHolder* holder = nullptr;
    DAVA::MockListener listener;
};
