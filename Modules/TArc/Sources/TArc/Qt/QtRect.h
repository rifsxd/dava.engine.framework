#pragma once

#include <Base/Any.h>
#include <QRect>

namespace DAVA
{
template <>
struct AnyCompare<QRect>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        return v1.Get<QRect>() == v2.Get<QRect>();
    }
};
}
