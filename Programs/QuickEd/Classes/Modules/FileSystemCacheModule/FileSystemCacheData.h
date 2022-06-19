#pragma once

#include <TArc/DataProcessing/TArcDataNode.h>
#include <QStringList>

class FileSystemCache;

class FileSystemCacheData : public DAVA::TArcDataNode
{
public:
    FileSystemCacheData(const QStringList& extensions);
    ~FileSystemCacheData() override;

    QStringList GetFiles(const QString& extension) const;

private:
    friend class FileSystemCacheModule;

    FileSystemCache* GetFileSystemCache();
    std::unique_ptr<FileSystemCache> fileSystemCache;

    DAVA_VIRTUAL_REFLECTION(FileSystemCacheData, DAVA::TArcDataNode);
};
