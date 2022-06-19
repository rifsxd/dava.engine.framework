#include "ResourceArchiverModule/ResourceArchiver.h"

#include <AssetCache/AssetCacheClient.h>

#include <FileSystem/FileSystem.h>
#include <FileSystem/File.h>
#include <FileSystem/FilePath.h>
#include <FileSystem/FileList.h>
#include <FileSystem/Private/PackFormatSpec.h>
#include <FileSystem/Private/PackMetaData.h>
#include <Utils/UTF8Utils.h>
#include <Utils/StringUtils.h>
#include <Utils/StringFormat.h>
#include <Utils/CRC32.h>
#include <Compression/LZ4Compressor.h>
#include <Compression/ZipCompressor.h>
#include <Platform/DeviceInfo.h>
#include <Time/DateTime.h>
#include <Logger/Logger.h>
#include <Engine/Engine.h>
#include <Job/JobManager.h>

#include <sqlite_modern_cpp.h>
#include <algorithm>

ENUM_DECLARE(DAVA::Compressor::Type)
{
    ENUM_ADD_DESCR(static_cast<int>(DAVA::Compressor::Type::Lz4), "lz4");
    ENUM_ADD_DESCR(static_cast<int>(DAVA::Compressor::Type::Lz4HC), "lz4hc");
    ENUM_ADD_DESCR(static_cast<int>(DAVA::Compressor::Type::RFC1951), "rfc1951");
    ENUM_ADD_DESCR(static_cast<int>(DAVA::Compressor::Type::None), "none");
};

namespace DAVA
{
namespace ResourceArchiver
{
struct CollectedFile
{
    FilePath absPath;
    String archivePath;
};

void CollectAllFilesInDirectory(const FilePath& dirPath, const String& dirArchivePath, bool addHidden, Vector<CollectedFile>& collectedFiles)
{
    ScopedPtr<FileList> fileList(new FileList(dirPath, addHidden));
    for (uint32 file = 0; file < fileList->GetCount(); ++file)
    {
        if (fileList->IsNavigationDirectory(file))
        {
            continue;
        }

        if (fileList->IsDirectory(file))
        {
            String directoryName = fileList->GetFilename(file);
            FilePath subDirAbsolute = dirPath + (directoryName + '/');
            String subDirArchive = dirArchivePath + (directoryName + '/');
            CollectAllFilesInDirectory(subDirAbsolute, subDirArchive, addHidden, collectedFiles);
        }
        else
        {
            if (fileList->IsHidden(file) && addHidden == false)
            {
                continue;
            }

            CollectedFile collectedFile;
            collectedFile.absPath = fileList->GetPathname(file);
            collectedFile.archivePath = dirArchivePath + fileList->GetFilename(file);
            collectedFiles.push_back(collectedFile);
        }
    }
}

bool WriteHeaderBlock(File* outputFile, const PackFormat::PackFile::FooterBlock& footer)
{
    DVASSERT(outputFile != nullptr);

    uint32 sizeToWrite = sizeof(PackFormat::PackFile::FooterBlock);
    uint32 written = outputFile->Write(&footer, sizeToWrite);
    if (written != sizeToWrite)
    {
        Logger::Error("Can't write header block to archive");
        return false;
    }

    return true;
}

bool WriteRawData(File* outputFile, const Vector<uint8>& srcBuffer)
{
    DVASSERT(outputFile != nullptr);

    uint32 sizeToWrite = static_cast<uint32>(srcBuffer.size());
    uint32 written = outputFile->Write(srcBuffer.data(), sizeToWrite);
    if (written != sizeToWrite)
    {
        Logger::Error("Can't write data block to archive");
        return false;
    }

    return true;
}

Vector<uint8> SpliceFileNames(const Vector<CollectedFile>& collectedFiles)
{
    Vector<uint8> sortedNamesOriginal;
    StringStream ss;
    for (const CollectedFile& file : collectedFiles)
    {
        ss << file.archivePath << '\0';
    }
    String strNames(ss.str());
    sortedNamesOriginal.assign(strNames.begin(), strNames.end());
    return sortedNamesOriginal;
}

MD5::MD5Digest CalculateSourcesMD5(const Vector<CollectedFile>& collectedFiles)
{
    MD5 md5;
    md5.Init();
    for (const CollectedFile& collectedFile : collectedFiles)
    {
        md5.Update(reinterpret_cast<const uint8*>(collectedFile.archivePath.data()), static_cast<uint32>(collectedFile.archivePath.size()));

        if (!collectedFile.absPath.IsEmpty())
        {
            MD5::MD5Digest fileDigest;
            MD5::ForFile(collectedFile.absPath, fileDigest);
            md5.Update(fileDigest.digest.data(), static_cast<uint32>(fileDigest.digest.size()));
        }
    }

    md5.Final();
    return md5.GetDigest();
}

MD5::MD5Digest CalculateParamsMD5(Vector<String> params)
{
    MD5 md5;
    md5.Init();
    for (const String& param : params)
    {
        md5.Update(reinterpret_cast<const uint8*>(param.data()), static_cast<uint32>(param.size()));
    }

    md5.Final();
    return md5.GetDigest();
}

void ConstructCacheKeys(AssetCache::CacheItemKey& keyForArchive, AssetCache::CacheItemKey& keyForLog, Vector<CollectedFile>& collectedFiles, const DAVA::String& compression)
{
    MD5::MD5Digest sourcesMD5 = CalculateSourcesMD5(collectedFiles);
    keyForArchive.SetPrimaryKey(sourcesMD5);
    keyForLog.SetPrimaryKey(sourcesMD5);

    MD5::MD5Digest paramsMD5Archive = CalculateParamsMD5({ compression, "key for archive file" });
    MD5::MD5Digest paramsMD5Log = CalculateParamsMD5({ compression, "this one is for log file" });
    keyForArchive.SetSecondaryKey(paramsMD5Archive);
    keyForLog.SetSecondaryKey(paramsMD5Log);
}

bool RetrieveFileFromCache(AssetCacheClient* assetCacheClient, const AssetCache::CacheItemKey& key, const FilePath& outputFile)
{
    DVASSERT(assetCacheClient != nullptr);

    AssetCache::CachedItemValue retrievedData;
    AssetCache::Error result = assetCacheClient->RequestFromCacheSynchronously(key, &retrievedData);
    if (result == AssetCache::Error::NO_ERRORS)
    {
        if (retrievedData.ExportToFile(outputFile))
        {
            Logger::Info("%s is retrieved from cache", outputFile.GetAbsolutePathname().c_str());
            return true;
        }
        else
        {
            Logger::Error("Can't export retrieved data to file %s", outputFile.GetAbsolutePathname().c_str());
            return false;
        }
    }
    else
    {
        Logger::Info("Failed to retrieve %s from cache: %s", outputFile.GetAbsolutePathname().c_str(), AssetCache::ErrorToString(result).c_str());
        return false;
    }
}

bool RetrieveFromCache(AssetCacheClient* assetCacheClient, const AssetCache::CacheItemKey& keyForArchive, const AssetCache::CacheItemKey& keyForLog, const FilePath& pathToArchive, const FilePath& pathToLog)
{
    bool archiveIsRetrieved = RetrieveFileFromCache(assetCacheClient, keyForArchive, pathToArchive);
    if (archiveIsRetrieved && !pathToLog.IsEmpty())
    {
        RetrieveFileFromCache(assetCacheClient, keyForLog, pathToLog);
    }

    return archiveIsRetrieved;
}

bool AddFileToCache(AssetCacheClient* assetCacheClient, const AssetCache::CachedItemValue::Description& description, const AssetCache::CacheItemKey& key, const FilePath& pathToFile)
{
    DVASSERT(assetCacheClient != nullptr);

    AssetCache::CachedItemValue value;
    value.Add(pathToFile);
    value.UpdateValidationData();
    value.SetDescription(description);
    AssetCache::Error result = assetCacheClient->AddToCacheSynchronously(key, value);
    if (result == AssetCache::Error::NO_ERRORS)
    {
        Logger::Info("%s is added to cache", pathToFile.GetAbsolutePathname().c_str());
        return true;
    }
    else
    {
        Logger::Info("Failed to add %s to cache: %s", pathToFile.GetAbsolutePathname().c_str(), AssetCache::ErrorToString(result).c_str());
        return false;
    }
}

bool AddToCache(AssetCacheClient* assetCacheClient, const AssetCache::CacheItemKey& keyForArchive, const AssetCache::CacheItemKey& keyForLog, const FilePath& pathToArchive, const FilePath& pathToLog)
{
    DateTime timeNow = DateTime::Now();
    AssetCache::CachedItemValue::Description cacheItemDescription;
    cacheItemDescription.machineName = UTF8Utils::EncodeToUTF8(DeviceInfo::GetName());
    cacheItemDescription.creationDate = UTF8Utils::EncodeToUTF8(timeNow.GetLocalizedDate()) + "_" + UTF8Utils::EncodeToUTF8(timeNow.GetLocalizedTime());
    cacheItemDescription.comment = Format("Resource archive %s", pathToArchive.GetAbsolutePathname().c_str());

    bool archiveIsAdded = AddFileToCache(assetCacheClient, cacheItemDescription, keyForArchive, pathToArchive);
    if (archiveIsAdded && !pathToLog.IsEmpty())
    {
        AddFileToCache(assetCacheClient, cacheItemDescription, keyForLog, pathToLog);
    }

    return archiveIsAdded;
}

bool CollectFilesFromDB(const FilePath& baseDirPath, const FilePath& metaDbPath, Vector<CollectedFile>& collectedFiles)
{
    try
    {
        sqlite::database db(metaDbPath.GetAbsolutePathname());

        size_t numFiles = 0;

        db << "SELECT count(*) FROM files"
        >> [&](int64 countFiles)
        {
            DVASSERT(countFiles > 0);
            numFiles = static_cast<size_t>(countFiles);
        };

        collectedFiles.clear();
        collectedFiles.reserve(numFiles);

        FileSystem* fs = GetEngineContext()->fileSystem;

        UnorderedMap<String, String> lowerPathMap;

        db << "SELECT path, pack_index FROM files"
        >> [&](String path, int64 /*packIndex*/) // HACK we have to do request as "SELECT path, pack_index FROM files" to save order from sqlite
        {
            std::transform(begin(path), end(path), begin(path), [](char c) { return c == '\\' ? '/' : c; });

            if (path.find("../") != String::npos)
            {
                Logger::Error("incorrect relative file path: %s (found \"../\")", path.c_str());
                DAVA_THROW(Exception, "incorrect relative file path");
            }

            if (path.find("./") != String::npos)
            {
                Logger::Error("incorrect relative file path: %s (found \"./\")", path.c_str());
                DAVA_THROW(Exception, "incorrect relative file path");
            }

            String lower = path;
            std::transform(begin(lower), end(lower), begin(lower), [](char c) { return ::tolower(c); });

            auto it = lowerPathMap.find(lower);
            if (it != end(lowerPathMap))
            {
                Logger::Error("incorrect path differ only char register or the same: %s and %s", it->second.c_str(), path.c_str());
                DAVA_THROW(Exception, "incorrect relative file path");
            }
            else
            {
                lowerPathMap.emplace(lower, path);
            }

            FilePath fullPath = baseDirPath + path;
            if (!fs->Exists(fullPath))
            {
                Logger::Error("can't find file: %s", fullPath.GetAbsolutePathname().c_str());
                DAVA_THROW(Exception, "incorrect relative file path, file not found");
            }
            else
            {
                CollectedFile collectedFile;
                collectedFile.absPath = fullPath;
                collectedFile.archivePath = path;
                collectedFiles.push_back(collectedFile);
            }
        };
    }
    catch (std::exception& ex)
    {
        Logger::Error("%s", ex.what());
        return false;
    }

    return true;
}

bool CollectFiles(const Vector<String>& sources, const FilePath& baseDir, const bool addHiddenFiles, Vector<CollectedFile>& collectedFiles)
{
    for (const String& source : sources)
    {
        FilePath sourcePath;
        if (FilePath::IsAbsolutePathname(source))
        {
            sourcePath = source;
            if (sourcePath == baseDir)
            {
                Logger::Error("Source path is the same as base dir: %s", baseDir.GetAbsolutePathname().c_str());
                return false;
            }
        }
        else
        {
            sourcePath = baseDir + source;
        }

        const String resultPathString = sourcePath.GetStringValue();
        if (String::npos != resultPathString.find_first_of("../") || String::npos != resultPathString.find_first_of("..\\"))
        {
            Logger::Error("Source path contains reletive parent folder: \"../\"");
            return false;
        }

        if (false == FileSystem::Instance()->Exists(sourcePath))
        {
            Logger::Error("Source '%s' is not existing", sourcePath.GetAbsolutePathname().c_str());
            return false;
        }

        if (!addHiddenFiles && FileSystem::Instance()->IsHidden(sourcePath))
        {
            continue;
        }

        String archivePath;
        if (sourcePath.StartsWith(baseDir))
        {
            archivePath = sourcePath.GetRelativePathname(baseDir);
        }
        else
        {
            Logger::Error("Source '%s' doesn't belong to base dir %s and will be placed in archive root", sourcePath.GetAbsolutePathname().c_str(), baseDir.GetAbsolutePathname().c_str());
            return false;
        }

        if (sourcePath.IsDirectoryPathname())
        {
            CollectAllFilesInDirectory(sourcePath, archivePath, addHiddenFiles, collectedFiles);
        }
        else
        {
            CollectedFile collectedFile;
            collectedFile.absPath = sourcePath;
            collectedFile.archivePath = archivePath;
            collectedFiles.push_back(collectedFile);
        }
    }

    std::stable_sort(collectedFiles.begin(), collectedFiles.end(), [](const CollectedFile& left, const CollectedFile& right) -> bool
                     {
                         return left.archivePath < right.archivePath;
                     });

    // removing duplicate files
    auto pointerAtDuplicates = std::unique(collectedFiles.begin(), collectedFiles.end(), [](const CollectedFile& left, const CollectedFile& right) -> bool
                                           {
                                               if (left.absPath == right.absPath)
                                               {
                                                   Logger::Error("Skipping duplicate %s archivePath: %s", left.absPath.GetAbsolutePathname().c_str(), left.archivePath.c_str());
                                                   return true;
                                               }
                                               else
                                               {
                                                   return false;
                                               }
                                           });
    if (pointerAtDuplicates != end(collectedFiles))
    {
        Logger::Error("Duplicated path to files are strange! Why do you duplicate it? Stop...");
        return false;
    }

    // check colliding files (they are different but have same archivePath and thus one will rewrite another during unpack)
    pointerAtDuplicates = std::unique(collectedFiles.begin(), collectedFiles.end(), [](const CollectedFile& left, const CollectedFile& right) -> bool
                                      {
                                          if (left.archivePath == right.archivePath)
                                          {
                                              Logger::Error("'%s' and '%s' will be having the same path '%s' in archive",
                                                            left.absPath.GetAbsolutePathname().c_str(), right.absPath.GetAbsolutePathname().c_str(), right.archivePath.c_str());
                                              return true;
                                          }
                                          else
                                          {
                                              return false;
                                          }
                                      });
    if (pointerAtDuplicates != collectedFiles.end())
    {
        return false;
    }

    return true;
}

const Compressor* GetCompressor(Compressor::Type compressorType)
{
    static const std::unique_ptr<Compressor> lz4Compressor(new LZ4Compressor);
    static const std::unique_ptr<Compressor> lz4HCCompressor(new LZ4HCCompressor);
    static const std::unique_ptr<Compressor> zipCompressor(new ZipCompressor);

    switch (compressorType)
    {
    case Compressor::Type::Lz4:
        return lz4Compressor.get();
    case Compressor::Type::Lz4HC:
        return lz4HCCompressor.get();
    case Compressor::Type::RFC1951:
        return zipCompressor.get();
    default:
    {
        DVASSERT(false, Format("Unexpected compressor type: %u", compressorType).c_str());
        return nullptr;
    }
    }
}

bool Pack(const Vector<CollectedFile>& collectedFiles,
          const DAVA::Compressor::Type compressionType,
          const FilePath& metaDb,
          File* outputFile,
          bool dummyFileData)
{
    // validate input params
    if (collectedFiles.empty())
    {
        Logger::Error("no files to commpress");
        return false;
    }

    const Compressor* compressor = nullptr;
    if (compressionType != Compressor::Type::None)
    {
        compressor = GetCompressor(compressionType);
        if (compressor == nullptr)
        {
            Logger::Error("Can't compressor");
            return false;
        }
    }

    if (!metaDb.Exists())
    {
        Logger::Error("metaDB not exist");
        return false;
    }

    if (outputFile == nullptr)
    {
        Logger::Error("outputFile is nullptr");
        return false;
    }

    // load metadata
    // CREATE TABLE IF NOT EXISTS files(path TEXT PRIMARY KEY, pack_index INTEGER NOT NULL);
    // CREATE TABLE IF NOT EXISTS packs(index INTEGER PRIMARY KEY, name TEXT UNIQUE, dependency TEXT NOT NULL);
    std::unique_ptr<PackMetaData> meta;

    try
    {
        meta.reset(new PackMetaData(metaDb));
    }
    catch (std::exception& ex)
    {
        Logger::Error("can't open or parse metaDb: %s", ex.what());
        return false;
    }

    const size_t numOfFiles = collectedFiles.size();
    PackFormat::PackFile packFile;
    packFile.filesTable.data.files.resize(numOfFiles);
    Vector<Vector<uint8>> useBuffers;
    useBuffers.resize(numOfFiles);

    FileSystem* fs = FileSystem::Instance();
    JobManager* jobManager = GetEngineContext()->jobManager;
    DVASSERT(jobManager != nullptr);

    std::atomic<uint32> countLoadedFiles{ 0 };

    for (uint32 fileIndex = 0; fileIndex < numOfFiles; ++fileIndex)
    {
        jobManager->CreateWorkerJob([&, fileIndex]()
                                    {
                                        const CollectedFile& collectedFile = collectedFiles[fileIndex];
                                        PackFormat::FileTableEntry& fileEntry = packFile.filesTable.data.files[fileIndex];

                                        Vector<uint8> origFileBuffer;
                                        Vector<uint8> compressedFileBuffer;

                                        bool useCompressedBuffer = (compressionType != Compressor::Type::None);
                                        Compressor::Type useCompression = compressionType;

                                        if (dummyFileData)
                                        {
                                            origFileBuffer.resize(1);
                                            origFileBuffer[0] = 0;

                                            useCompressedBuffer = false;
                                            useCompression = Compressor::Type::None;
                                        }
                                        else
                                        {
                                            if (!fs->ReadFileContents(collectedFile.absPath, origFileBuffer))
                                            {
                                                Logger::Error("Can't read contents of: ", collectedFile.absPath.GetAbsolutePathname().c_str());
                                                return;
                                            }

                                            if (origFileBuffer.empty())
                                            {
                                                useCompressedBuffer = false;
                                                useCompression = Compressor::Type::None;
                                            }

                                            if (useCompressedBuffer)
                                            {
                                                if (!compressor->Compress(origFileBuffer, compressedFileBuffer))
                                                {
                                                    Logger::Error("Can't compress contents of: %s", collectedFile.absPath.GetAbsolutePathname().c_str());
                                                    return;
                                                }

                                                if (compressedFileBuffer.size() < origFileBuffer.size())
                                                {
                                                    useCompressedBuffer = true;
                                                }
                                                else
                                                {
                                                    useCompressedBuffer = false;
                                                    useCompression = Compressor::Type::None;
                                                }
                                            }
                                        }

                                        Vector<uint8>& useBuffer = (useCompressedBuffer ? compressedFileBuffer : origFileBuffer);

                                        fileEntry.startPosition = 0; // later fill this field
                                        fileEntry.originalSize = static_cast<uint32>(origFileBuffer.size());
                                        fileEntry.compressedSize = static_cast<uint32>(useBuffer.size());
                                        fileEntry.type = useCompression;
                                        fileEntry.compressedCrc32 = CRC32::ForBuffer(useBuffer.data(), useBuffer.size());
                                        fileEntry.originalCrc32 = CRC32::ForBuffer(origFileBuffer.data(), origFileBuffer.size());
                                        if (!meta)
                                        {
                                            fileEntry.metaIndex = 0; // do it or your crc32 randomly change on same files
                                        }
                                        else
                                        {
                                            // we have PackArchive with vector of FileInfo's
                                            // from PackArchive we can get fileIndex
                                            // with fileIndex from PackMetaData we can get packIndex
                                            // and later use metaIndex(packIndex) directly from FileInfo
                                            // files table example
                                            //|--------------------------------------|
                                            //|file_path(sorted)----------|pack_index|
                                            //|3d/gfx/uber_file.pvr       |         0|
                                            //|--------------------------------------|
                                            // packs table example
                                            //|--------------------------------------|
                                            //|pack_index|pack_name-----|pack_dep----|
                                            //|         0|group_pack_1  |group_pack_0|
                                            //|--------------------------------------|
                                            // so packIndex(metaIndex) is duplicated in FileInfo's for now.
                                            fileEntry.metaIndex = meta->GetPackIndexForFile(fileIndex);
                                        }
                                        useBuffers[fileIndex] = std::move(useBuffer);
                                        ++countLoadedFiles;
                                    });

        //HACK wait jobs executed (internaly 1024 max jobs)
        if (fileIndex > 0 && fileIndex % 1000 == 0)
        {
            jobManager->WaitWorkerJobs();
        }
    } // end for fileIndex

    jobManager->WaitWorkerJobs();

    if (numOfFiles != countLoadedFiles)
    {
        return false;
    }

    // write all compressed content to output file and set startPosition fileEntry
    for (size_t fileIndex = 0, dataOffset = 0; fileIndex < numOfFiles; ++fileIndex)
    {
        PackFormat::FileTableEntry& fileEntry = packFile.filesTable.data.files[fileIndex];
        fileEntry.startPosition = dataOffset;

        const Vector<uint8>& useBuffer = useBuffers[fileIndex];
        if (!WriteRawData(outputFile, useBuffer))
        {
            Logger::Error("can't write buffer to output file");
            return false;
        }
        dataOffset += useBuffer.size();
    }
    useBuffers.clear();
    useBuffers.shrink_to_fit(); // free memory

    Vector<uint8> metaBytes;
    if (meta)
    {
        metaBytes = meta->Serialize();
        if (!WriteRawData(outputFile, metaBytes))
        {
            DAVA_THROW(Exception, "can't write metadata");
        }
    }

    PackFormat::PackFile::FooterBlock& footerBlock = packFile.footer;
    PackFormat::PackFile::FilesTableBlock::Names& namesBlock = packFile.filesTable.names;
    PackFormat::PackFile::FilesTableBlock::FilesData& filesDataBlock = packFile.filesTable.data;

    Vector<uint8> namesOriginal = SpliceFileNames(collectedFiles);
    namesBlock.compressedNames.reserve(namesOriginal.size());

    if (!namesOriginal.empty())
    {
        if (!LZ4HCCompressor().Compress(namesOriginal, namesBlock.compressedNames))
        {
            Logger::Error("Can't compress names block");
            return false;
        }
    }

    namesBlock.compressedCrc32 = CRC32::ForBuffer(namesBlock.compressedNames.data(), namesBlock.compressedNames.size());

    const uint32 fileDataSize = static_cast<uint32>(filesDataBlock.files.size() * sizeof(PackFormat::FileTableEntry));
    uint32 fileTableSize = fileDataSize + static_cast<uint32>(namesBlock.compressedNames.size() + sizeof(namesBlock.compressedCrc32));

    if (0 == fileDataSize)
    {
        fileTableSize = 0;
    }

    // place in one buffer full FileTableBlock first all filesData then compressed names and compressedNamesCrc32
    Vector<uint8> tmpFileTable;
    tmpFileTable.resize(fileTableSize);

    if (!filesDataBlock.files.empty())
    {
        // copy files data
        std::copy_n(filesDataBlock.files.data(), filesDataBlock.files.size(), reinterpret_cast<PackFormat::FileTableEntry*>(tmpFileTable.data()));
    }

    if (!namesBlock.compressedNames.empty())
    {
        // copy compressed names data
        std::copy_n(namesBlock.compressedNames.data(), namesBlock.compressedNames.size(), &tmpFileTable.at(fileDataSize));
        // copy compressed crc32
        std::copy_n(&namesBlock.compressedCrc32, 1, reinterpret_cast<uint32*>(&tmpFileTable.at(fileTableSize - sizeof(namesBlock.compressedCrc32))));
    }

    if (fileTableSize > 0)
    {
        const uint32 numBytesWrite = outputFile->Write(tmpFileTable.data(), static_cast<uint32>(tmpFileTable.size()));
        if (numBytesWrite != tmpFileTable.size())
        {
            Logger::Error("Can't write fileTableBlock");
            return false;
        }
    }

    footerBlock.info.filesTableSize = fileTableSize;
    footerBlock.info.filesTableCrc32 = CRC32::ForBuffer(tmpFileTable.data(), tmpFileTable.size());
    footerBlock.info.packArchiveMarker = PackFormat::FILE_MARKER;
    footerBlock.info.numFiles = static_cast<uint32>(filesDataBlock.files.size());
    footerBlock.info.namesSizeCompressed = static_cast<uint32>(namesBlock.compressedNames.size());
    footerBlock.info.namesSizeOriginal = static_cast<uint32>(namesOriginal.size());

    footerBlock.infoCrc32 = CRC32::ForBuffer(&footerBlock.info, sizeof(footerBlock.info));
    if (meta)
    {
        footerBlock.metaDataCrc32 = CRC32::ForBuffer(&metaBytes[0], metaBytes.size());
        footerBlock.metaDataSize = static_cast<uint32>(metaBytes.size());
    }
    else
    {
        footerBlock.metaDataCrc32 = 0;
        footerBlock.metaDataSize = 0;
    }

    if (!WriteHeaderBlock(outputFile, footerBlock))
    {
        Logger::Error("Can't write footerBlock");
        return false;
    }

    return true;
}

bool Pack(const Vector<CollectedFile>& collectedFiles, DAVA::Compressor::Type compressionType, const FilePath& archivePath, const FilePath& metaDb, bool dummyFileData)
{
    ScopedPtr<File> outputFile(File::Create(archivePath, File::CREATE | File::WRITE));
    if (!outputFile)
    {
        Logger::Error("Can't create %s", archivePath.GetAbsolutePathname().c_str());
        return false;
    }

    if (!Pack(collectedFiles, compressionType, metaDb, outputFile, dummyFileData))
    {
        outputFile.reset();
        if (!FileSystem::Instance()->DeleteFile(archivePath))
        {
            Logger::Error("Can't delete %s", archivePath.GetAbsolutePathname().c_str());
        }
        return false;
    }

    return true;
}

bool CreateArchive(const Params& params)
{
    if (!params.baseDirPath.IsDirectoryPathname() || !FileSystem::Instance()->IsDirectory(params.baseDirPath))
    {
        Logger::Error("Base dir '%s' is not a valid path", params.baseDirPath.GetAbsolutePathname().c_str());
        return false;
    }

    Vector<CollectedFile> collectedFiles;

    if (!CollectFilesFromDB(params.baseDirPath, params.metaDbPath, collectedFiles))
    {
        Logger::Error("Collecting files error");
        return false;
    }

    if (Pack(collectedFiles, params.compressionType, params.archivePath, params.metaDbPath, params.dummyFileData))
    {
        return true;
    }

    Logger::Info("packing failed");
    return false;
}

} // namespace Archiver
} // namespace DAVA
