#include "REPlatform/Scene/Utils/RESceneUtils.h"

#include <FileSystem/FileSystem.h>
#include <Logger/Logger.h>
#include <Render/Renderer.h>
#include <Render/RenderHelper.h>
#include <Render/RHI/rhi_Public.h>
#include <Utils/StringFormat.h>

namespace DAVA
{
void RESceneUtils::CleanFolder(const FilePath& folderPathname)
{
    bool ret = FileSystem::Instance()->DeleteDirectory(folderPathname);
    if (!ret)
    {
        bool folderExists = FileSystem::Instance()->IsDirectory(folderPathname);
        if (folderExists)
        {
            Logger::Error("[CleanFolder] ret = %d, folder = %s", ret, folderPathname.GetAbsolutePathname().c_str());
        }
    }
}

void RESceneUtils::SetInFolder(const FilePath& folderPathname)
{
    DVASSERT(folderPathname.IsDirectoryPathname());
    dataSourceFolder = folderPathname;
}

void RESceneUtils::SetOutFolder(const FilePath& folderPathname)
{
    DVASSERT(folderPathname.IsDirectoryPathname());
    dataFolder = folderPathname;
}

bool RESceneUtils::CopyFile(const FilePath& filePathname)
{
    String workingPathname = filePathname.GetRelativePathname(dataSourceFolder);
    PrepareFolderForCopyFile(workingPathname);

    bool retCopy = FileSystem::Instance()->CopyFile(dataSourceFolder + workingPathname, dataFolder + workingPathname);
    if (!retCopy)
    {
        Logger::Error("Can't copy %s from %s to %s",
                      workingPathname.c_str(),
                      dataSourceFolder.GetAbsolutePathname().c_str(),
                      dataFolder.GetAbsolutePathname().c_str());
    }

    return retCopy;
}

void RESceneUtils::PrepareFolderForCopyFile(const String& filename)
{
    FilePath newFolderPath = (dataFolder + filename).GetDirectory();

    if (!FileSystem::Instance()->IsDirectory(newFolderPath))
    {
        FileSystem::eCreateDirectoryResult retCreate = FileSystem::Instance()->CreateDirectory(newFolderPath, true);
        if (FileSystem::DIRECTORY_CANT_CREATE == retCreate)
        {
            Logger::Error("Can't create folder %s", newFolderPath.GetAbsolutePathname().c_str());
        }
    }

    FileSystem::Instance()->DeleteFile(dataFolder + filename);
}

FilePath RESceneUtils::GetNewFilePath(const FilePath& oldPathname) const
{
    String workingPathname = oldPathname.GetRelativePathname(dataSourceFolder);
    return dataFolder + workingPathname;
}

void RESceneUtils::AddFile(const FilePath& sourcePath)
{
    String workingPathname = sourcePath.GetRelativePathname(dataSourceFolder);
    FilePath destinationPath = dataFolder + workingPathname;

    if (sourcePath != destinationPath)
    {
        DVASSERT(!sourcePath.IsEmpty());
        DVASSERT(!destinationPath.IsEmpty());

        filesForCopy[sourcePath] = destinationPath;
    }
}

void RESceneUtils::CopyFiles()
{
    PrepareDestination();

    auto endIt = filesForCopy.end();
    for (auto it = filesForCopy.begin(); it != endIt; ++it)
    {
        bool retCopy = false;

        if (FileSystem::Instance()->Exists(it->first))
        {
            FileSystem::Instance()->DeleteFile(it->second);
            retCopy = FileSystem::Instance()->CopyFile(it->first, it->second);
        }

        if (!retCopy)
        {
            Logger::Error("Can't copy %s to %s",
                          it->first.GetAbsolutePathname().c_str(),
                          it->second.GetAbsolutePathname().c_str());
        }
    }
}

void RESceneUtils::PrepareDestination()
{
    Set<FilePath> folders;

    Map<FilePath, FilePath>::const_iterator endMapIt = filesForCopy.end();
    for (Map<FilePath, FilePath>::const_iterator it = filesForCopy.begin(); it != endMapIt; ++it)
    {
        folders.insert(it->second.GetDirectory());
    }

    Set<FilePath>::const_iterator endSetIt = folders.end();
    for (Set<FilePath>::const_iterator it = folders.begin(); it != endSetIt; ++it)
    {
        if (!FileSystem::Instance()->Exists(*it))
        {
            FileSystem::eCreateDirectoryResult retCreate = FileSystem::Instance()->CreateDirectory((*it), true);
            if (FileSystem::DIRECTORY_CANT_CREATE == retCreate)
            {
                Logger::Error("Can't create folder %s", (*it).GetAbsolutePathname().c_str());
            }
        }
    }
}
} // namespace DAVA
