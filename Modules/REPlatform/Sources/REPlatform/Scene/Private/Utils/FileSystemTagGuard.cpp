#include "REPlatform/Scene/Utils/FileSystemTagGuard.h"

#include <Engine/Engine.h>
#include <FileSystem/FileSystem.h>

namespace DAVA
{
FileSystemTagGuard::FileSystemTagGuard(const String newFilenamesTag)
{
    FileSystem* fs = GetEngineContext()->fileSystem;
    oldFilenamesTag = fs->GetFilenamesTag();
    fs->SetFilenamesTag(newFilenamesTag);
}

FileSystemTagGuard::~FileSystemTagGuard()
{
    FileSystem* fs = GetEngineContext()->fileSystem;
    fs->SetFilenamesTag(oldFilenamesTag);
}
} // namespace DAVA
