#include "TArc/Controls/Private/ValidationUtils.h"

#include <Debug/DVAssert.h>

namespace DAVA
{
QValidator::State ConvertValidationState(M::ValidationResult::eState state)
{
    switch (state)
    {
    case M::ValidationResult::eState::Invalid:
        return QValidator::Invalid;
    case M::ValidationResult::eState::Intermediate:
        return QValidator::Intermediate;
    case M::ValidationResult::eState::Valid:
        return QValidator::Acceptable;
    default:
        break;
    }

    DVASSERT(false);
    return QValidator::Invalid;
}
} // namespace DAVA