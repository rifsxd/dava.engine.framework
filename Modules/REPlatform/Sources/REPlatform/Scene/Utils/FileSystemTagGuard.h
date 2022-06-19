#pragma once

#include <Base/String.h>

namespace DAVA
{
class FileSystemTagGuard final
{
public:
    FileSystemTagGuard(const String newFilenamesTag);
    ~FileSystemTagGuard();

private:
    String oldFilenamesTag;
};
} // namespace DAVA
