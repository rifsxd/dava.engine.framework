#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Controls/PropertyPanel/TimerUpdater.h"
#include "TArc/Controls/PropertyPanel/PropertiesView.h"

#include <Time/SystemTimer.h>

DAVA_TARC_TESTCLASS(TimerUpdaterTests)
{
    DAVA_TEST (FullUpdateTest)
    {
        TEST_VERIFY(updater == nullptr);
        TEST_VERIFY(currentTestFinished == false);
        updater.reset(new DAVA::TimerUpdater(500, -1));
        updater->update.Connect([this](DAVA::PropertiesView::UpdatePolicy policy)
                                {
                                    TEST_VERIFY(policy == DAVA::PropertiesView::FullUpdate);
                                    currentTestFinished = true;
                                });
    }

    DAVA_TEST (FullFastUpdateTest)
    {
        TEST_VERIFY(updater == nullptr);
        TEST_VERIFY(currentTestFinished == false);
        updater.reset(new DAVA::TimerUpdater(500, 100));
        updater->update.Connect([this](DAVA::PropertiesView::UpdatePolicy policy)
                                {
                                    static DAVA::int32 counter = 1;
                                    if (counter == 5)
                                    {
                                        TEST_VERIFY(policy == DAVA::PropertiesView::FullUpdate);
                                    }
                                    else
                                    {
                                        TEST_VERIFY(policy == DAVA::PropertiesView::FastUpdate);
                                    }
                                    if (counter == 6)
                                    {
                                        currentTestFinished = true;
                                    }

                                    ++counter;
                                });
    }

    void TearDown(const DAVA::String& testName) override
    {
        updater.reset();
        currentTestFinished = false;
    }

    bool TestComplete(const DAVA::String& testName) const override
    {
        return currentTestFinished;
    }

    std::shared_ptr<DAVA::TimerUpdater> updater;
    bool currentTestFinished = false;
};
