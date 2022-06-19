#pragma once

#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
namespace Metas
{
class DisableEntityReparent
{
};
} // namespace Metas

namespace M
{
using DisableEntityReparent = Meta<Metas::DisableEntityReparent>;
}

} // namespace DAVA