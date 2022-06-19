#pragma once

#include <Base/Any.h>
#include <QByteArray>

namespace DAVA
{
template <>
struct AnyCompare<QByteArray>
{
    static bool IsEqual(const Any& v1, const Any& v2)
    {
        return v1.Get<QByteArray>() == v2.Get<QByteArray>();
    }
};
}
