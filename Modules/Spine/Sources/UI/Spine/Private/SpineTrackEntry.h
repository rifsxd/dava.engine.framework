#pragma once

#include <Base/BaseTypes.h>

struct spTrackEntry;

namespace DAVA
{
/** Public wrapper for Spine track entry structure.
Has getters for necessary track fields.
*/
class SpineTrackEntry
{
public:
    /** Constructor with specified pointer to spBone. */
    SpineTrackEntry(spTrackEntry* track);

    /** Return loopplayback flag of current track. */
    bool IsLoop() const;

    /** Return animation name of current track. */
    String GetName() const;

private:
    spTrackEntry* trackPtr = nullptr;
};
}