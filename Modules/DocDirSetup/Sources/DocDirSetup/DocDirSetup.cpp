#include "DocDirSetup/DocDirSetup.h"
#include "ApplicationSupportPath.h"

namespace DAVA
{
namespace DocumentsDirectorySetup
{
FilePath GetEngineDocumentsPath()
{
    return FileSystem::GetUserDocumentsPath() + "DAVAProject/";
}

FilePath GetApplicationDocDirectory(FileSystem* fs, const String& appName)
{
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
    return GetApplicationSupportPath() + appName + "/";
#else
    return GetEngineDocumentsPath() + appName + "/";
#endif
}

FileSystem::eCreateDirectoryResult CreateApplicationDocDirectory(FileSystem* fs, const String& appName)
{
    FilePath docDirectory = GetApplicationDocDirectory(fs, appName);
    return fs->CreateDirectory(docDirectory, true);
}

void SetApplicationDocDirectory(FileSystem* fs, const String& appName)
{
    FilePath docDirectory = GetApplicationDocDirectory(fs, appName);
    fs->CreateDirectory(docDirectory, true);
    fs->SetCurrentDocumentsDirectory(docDirectory);
}
}
}
