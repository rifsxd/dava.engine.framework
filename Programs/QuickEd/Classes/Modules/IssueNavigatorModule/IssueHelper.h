#pragma once

#include <Base/BaseTypes.h>

class IndexGenerator
{
    DAVA::int32 nextIssueId = 0;

public:
    IndexGenerator(const IndexGenerator&) = delete;
    IndexGenerator() = default;
    ~IndexGenerator() = default;

    DAVA::int32 NextIssueId();
};
