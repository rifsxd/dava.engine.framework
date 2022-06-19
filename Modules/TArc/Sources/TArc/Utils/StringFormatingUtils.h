#pragma once

#include "TArc/Qt/QtString.h"

#include <Base/BaseTypes.h>

namespace DAVA
{
void ReduceZeros(String& value);
void ReduceZeros(QString& value);
void FloatToString(float32 value, int32 precision, String& result);
void FloatToString(float32 value, int32 precision, QString& result);
void FloatToString(float64 value, int32 precision, QString& result);
} // namespace DAVA