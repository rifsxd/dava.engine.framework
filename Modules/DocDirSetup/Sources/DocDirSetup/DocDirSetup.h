#pragma once

#include <Base/BaseTypes.h>
#include <FileSystem/FileSystem.h>

namespace DAVA
{
namespace DocumentsDirectorySetup
{
/** get <user doc>/DavaProject path. This is the documents directory by default*/
FilePath GetEngineDocumentsPath();

/** get path to application documents. Depends on OS. For Mac it returns path inside of to app support dir. Otherwise it returns path inside of user doc directory*/
FilePath GetApplicationDocDirectory(FileSystem* fs, const String& appName);

/** create directory for app documents. See GetApplicationDocDirectory description*/
FileSystem::eCreateDirectoryResult CreateApplicationDocDirectory(FileSystem* fs, const String& appName);

/** create directory for app documents and set it as current doc dir. See GetApplicationDocDirectory description*/
void SetApplicationDocDirectory(FileSystem* fs, const String& appName);
}
}
