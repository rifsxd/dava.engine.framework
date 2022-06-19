#include "UI/Spine/Private/SpineTrackEntry.h"

#include <Debug/DVAssert.h>

#include <spine/spine.h>

namespace DAVA
{
SpineTrackEntry::SpineTrackEntry(spTrackEntry* track)
    : trackPtr(track)
{
    DVASSERT(trackPtr);
}

bool SpineTrackEntry::IsLoop() const
{
    return trackPtr->loop != 0;
}

String SpineTrackEntry::GetName() const
{
    return trackPtr->animation ? String(trackPtr->animation->name) : String();
}
}