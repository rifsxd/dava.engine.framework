#include "TArc/Utils/StringFormatingUtils.h"

#include <Utils/StringFormat.h>

namespace DAVA
{
template <typename T>
void ReduceZerosImpl(T& value)
{
    int32 zerosCount = 0;
    for (auto iter = value.rbegin(); iter != value.rend(); ++iter)
    {
        if ((*iter) == '.')
        {
            zerosCount = std::max(zerosCount - 1, 0);
            break;
        }
        if ((*iter) != '0')
        {
            break;
        }
        ++zerosCount;
    }

    value.resize(value.size() - zerosCount);
}

void ReduceZeros(String& value)
{
    ReduceZerosImpl(value);
}

void ReduceZeros(QString& value)
{
    ReduceZerosImpl(value);
}

void FloatToString(float32 value, int32 precision, String& result)
{
    String formatStr = Format("%%.%df", precision);
    result = Format(formatStr.c_str(), value);
    ReduceZeros(result);
}

void FloatToString(float32 value, int32 precision, QString& result)
{
    result = QString::number(value, 'f', precision);
    ReduceZeros(result);
}

void FloatToString(float64 value, int32 precision, QString& result)
{
    result = QString::number(value, 'f', precision);
    ReduceZeros(result);
}
} // namespace DAVA
