#include "Engine/Engine.h"
#include "UnitTests/UnitTests.h"

#if !defined(__DAVAENGINE_ANDROID__)

#include "NetworkCore/NetworkCoreModule.h"

using namespace DAVA;

DAVA_TESTCLASS (NetworkCoreTest)
{
    DAVA_TEST (CheckStatus)
    {
        const ModuleManager& moduleManager = *GetEngineContext()->moduleManager;
        NetworkCoreModule* networkCore = moduleManager.GetModule<NetworkCoreModule>();

        auto statusList = networkCore->StatusList();

        TEST_VERIFY(statusList.size() == 2);
        TEST_VERIFY(statusList[0] == NetworkCoreModule::ES_UNKNOWN);
        TEST_VERIFY(statusList[1] == NetworkCoreModule::ES_INIT);
    }
};

#endif // !defined(__DAVAENGINE_ANDROID__)
