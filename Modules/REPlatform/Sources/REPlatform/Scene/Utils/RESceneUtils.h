#pragma once

#include <Base/BaseTypes.h>
#include <FileSystem/FilePath.h>

namespace DAVA
{
class RESceneUtils
{
public:
    void CleanFolder(const FilePath& folderPathname);

    void SetInFolder(const FilePath& folderPathname);
    void SetOutFolder(const FilePath& folderPathname);

    bool CopyFile(const FilePath& filePathname);
    void PrepareFolderForCopyFile(const String& filename);

    FilePath GetNewFilePath(const FilePath& oldPathname) const;

    void AddFile(const FilePath& sourcePath);
    void CopyFiles();

private:
    void PrepareDestination();

public:
    FilePath dataFolder;
    FilePath dataSourceFolder;
    String workingFolder;

    Map<FilePath, FilePath> filesForCopy;
};
} // namespace DAVA
