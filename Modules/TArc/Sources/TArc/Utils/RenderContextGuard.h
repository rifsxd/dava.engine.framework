#pragma once

namespace DAVA
{
class RenderContextGuard final
{
public:
    RenderContextGuard();
    ~RenderContextGuard();
};
} // namespace DAVA
