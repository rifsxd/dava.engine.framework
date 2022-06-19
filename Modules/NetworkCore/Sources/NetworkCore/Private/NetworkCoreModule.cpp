#include "NetworkCore/NetworkCoreModule.h"
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
NetworkCoreModule::NetworkCoreModule(Engine* engine)
    : IModule(engine)
{
    statusList.emplace_back(eStatus::ES_UNKNOWN);
}

void NetworkCoreModule::Init()
{
    statusList.emplace_back(eStatus::ES_INIT);
}

void NetworkCoreModule::Shutdown()
{
    statusList.emplace_back(eStatus::ES_SHUTDOWN);
}

DAVA_VIRTUAL_REFLECTION_IMPL(NetworkCoreModule)
{
    ReflectionRegistrator<NetworkCoreModule>::Begin()
    .End();
}
}
