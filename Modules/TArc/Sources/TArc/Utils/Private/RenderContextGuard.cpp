#include "TArc/Utils/RenderContextGuard.h"

#include <Engine/Engine.h>
#include <Engine/PlatformApiQt.h>
#include <Engine/Window.h>

namespace DAVA
{
RenderContextGuard::RenderContextGuard()
{
    Engine* engine = Engine::Instance();
    DVASSERT(engine != nullptr);
    if (!engine->IsConsoleMode())
    {
        Window* window = engine->PrimaryWindow();
        PlatformApi::Qt::AcquireWindowContext(window);
    }
}

RenderContextGuard::~RenderContextGuard()
{
    Engine* engine = Engine::Instance();
    DVASSERT(engine != nullptr);
    if (!engine->IsConsoleMode())
    {
        Window* window = engine->PrimaryWindow();
        PlatformApi::Qt::ReleaseWindowContext(window);
    }
}
} // namespace DAVA
