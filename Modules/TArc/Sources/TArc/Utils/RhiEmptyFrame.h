#pragma once

#include <TArc/Utils/RenderContextGuard.h>

namespace DAVA
{
class RhiEmptyFrame
{
public:
    RhiEmptyFrame();
    ~RhiEmptyFrame();

private:
    RenderContextGuard ctxGuard;
};
}
