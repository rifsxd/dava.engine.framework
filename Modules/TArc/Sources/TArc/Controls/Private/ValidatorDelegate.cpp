#include "TArc/Controls/Private/ValidatorDelegate.h"

namespace DAVA
{
M::ValidationResult ValidatorDelegate::FixUp(const Any& value) const
{
    return Validate(value);
}
} // namespace DAVA