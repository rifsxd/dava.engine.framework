#pragma once

#include <FileSystem/FilePath.h>
#include <FileSystem/ResourceArchive.h>

namespace DAVA
{
namespace ResourceArchiver
{
struct Params
{
    Compressor::Type compressionType = Compressor::Type::Lz4HC;
    FilePath archivePath;
    FilePath baseDirPath;
    FilePath metaDbPath;
    bool dummyFileData = false;
};

bool CreateArchive(const Params& params);

} // namespace Archive
} // namespace DAVA
