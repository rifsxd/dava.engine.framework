#pragma once

#include <Reflection/ReflectedMeta.h>
#include <QValidator>

namespace DAVA
{
QValidator::State ConvertValidationState(M::ValidationResult::eState state);
} // namespace DAVA