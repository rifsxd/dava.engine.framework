#include "LoggerService/LogConsumer.h"
#include <Utils/StringFormat.h>
#include <Network/Base/Endpoint.h>

namespace DAVA
{
namespace Net
{
void LogConsumer::OnPacketReceived(const std::shared_ptr<IChannel>& channel, const void* buffer, size_t length)
{
    String data(static_cast<const char8*>(buffer), length);
    String endp(channel->RemoteEndpoint().ToString());

    String output = Format("[%s] %s", endp.c_str(), data.c_str());
    newDataNotifier.Emit(output);
}

} // namespace Net
} // namespace DAVA