#include "TexturePacker/ResourcePacker2D.h"
#include "TexturePacker/DefinitionFile.h"
#include "TexturePacker/TexturePacker.h"

#include <CommandLine/CommandLineParser.h>
#include <Engine/Engine.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/FileList.h>
#include <Utils/StringUtils.h>
#include <Platform/DeviceInfo.h>
#include <Time/DateTime.h>
#include <Time/SystemTimer.h>
#include <Utils/MD5.h>
#include <Utils/StringFormat.h>
#include <Utils/UTF8Utils.h>
#include <Render/GPUFamilyDescriptor.h>
#include <Platform/Process.h>
#include <Render/TextureDescriptor.h>
#include <Logger/Logger.h>

namespace DAVA
{
const String ResourcePacker2D::VERSION = "0.0.7";
const String ResourcePacker2D::INTERNAL_LIBPSD_VERSION = "0.0.1";

namespace ResourcePacker2DDetails
{
List<FilePath> ReadIgnoresList(const FilePath& ignoresListPath, const FilePath& baseDir)
{
    List<FilePath> result;

    ScopedPtr<File> listFile(File::Create(ignoresListPath, File::OPEN | File::READ));
    if (listFile)
    {
        while (!listFile->IsEof())
        {
            String str = StringUtils::Trim(listFile->ReadLine());
            if (!str.empty())
            {
                result.emplace_back(baseDir + str);
            }
        }
    }
    else
    {
        Logger::Error("Can't open ignore list file %s", ignoresListPath.GetAbsolutePathname().c_str());
    }

    return result;
}

bool IsNameInHardcodedIgnoreList(const String& filename)
{
    static const Vector<String> hardcodeIgnoredNames = { ".DS_Store", "flags.txt", "Thumbs.db", ".gitignore" };
    auto found = std::find_if(hardcodeIgnoredNames.begin(), hardcodeIgnoredNames.end(), [&filename](const String& name)
                              {
                                  return (CompareCaseInsensitive(filename, name) == 0);
                              });
    return (found != hardcodeIgnoredNames.end());
}

bool IsPathInIgnoreList(const FilePath& path, const List<FilePath>& ignoredFiles)
{
    return (std::find(ignoredFiles.begin(), ignoredFiles.end(), path) != ignoredFiles.end());
}

void SplitFileName(const String& filename, String& basename, String& ext)
{
    const String::size_type dotpos = filename.rfind(String("."));
    basename = filename.substr(0, dotpos);
    ext = filename.substr(dotpos, filename.size());
}

String GetBasenameWithoutTag(const String& basename, const String& tag)
{
    String result = basename;
    const String::size_type tagPos = result.find(tag);
    result.erase(tagPos, tag.size());
    return result;
}

bool IsBasenameContainsTag(const String& basename, const String& tag)
{
    const String::size_type tagPos = basename.find(tag);
    bool isTagged = (tagPos != String::npos && tagPos > 0); // tag can't be at beginning of the basename
    if (isTagged)
    {
        if (basename.find(tag, tagPos + tag.size()) != String::npos)
        {
            Logger::Error("File name '%s' contains multiple '%s' tags", basename.c_str(), tag.c_str());
        }
    }
    return isTagged;
}
} // namespace ResourcePacker2DDetails

String ResourcePacker2D::GetProcessFolderName()
{
    return "$process/";
}

void ResourcePacker2D::SetConvertQuality(const TextureConverter::eConvertQuality arg)
{
    quality = arg;
}

void ResourcePacker2D::SetCanceled(bool arg)
{
    cancelled = arg;
    if (cancelled)
    {
        Logger::FrameworkDebug("Resource packing is cancelled");
    }
}
void ResourcePacker2D::InitFolders(const FilePath& inputPath, const FilePath& outputPath)
{
    DVASSERT(inputPath.IsDirectoryPathname() && outputPath.IsDirectoryPathname());

    inputGfxDirectory = inputPath;
    outputGfxDirectory = outputPath;
    rootDirectory = inputPath + "../";
    dataSourceDirectory = inputPath + "../../";
}

void ResourcePacker2D::PackResources(const Vector<eGPUFamily>& forGPUs)
{
    SetCanceled(false);

    Logger::FrameworkDebug("Starting resource packing");
    Logger::FrameworkDebug("\nInput: %s \nOutput: %s \nRoot: %s",
                           inputGfxDirectory.GetAbsolutePathname().c_str(),
                           outputGfxDirectory.GetAbsolutePathname().c_str(),
                           rootDirectory.GetAbsolutePathname().c_str());

    if (FileSystem::Instance()->Exists(inputGfxDirectory) == false)
    {
        AddError(Format("Input folder is not exist: '%s'", inputGfxDirectory.GetStringValue().c_str()));
        SetCanceled();
        return;
    }

    if (StringUtils::HasWhitespace(texturePostfix))
    {
        AddError(Format("Texture name postfix '%s' has whitespaces", texturePostfix.c_str()).c_str());
        SetCanceled();
        return;
    }

    for (eGPUFamily gpu : forGPUs)
    {
        Logger::FrameworkDebug("For GPU: %s", (GPU_INVALID != gpu) ? GlobalEnumMap<eGPUFamily>::Instance()->ToString(gpu) : "Unknown");
    }

    Vector<PackingAlgorithm> packAlgorithms;

    String alg = CommandLineParser::Instance()->GetCommandParam("-alg");
    if (alg.empty() || CompareCaseInsensitive(alg, "maxrect") == 0)
    {
        packAlgorithms.push_back(PackingAlgorithm::ALG_MAXRECTS_BEST_AREA_FIT);
        packAlgorithms.push_back(PackingAlgorithm::ALG_MAXRECTS_BEST_LONG_SIDE_FIT);
        packAlgorithms.push_back(PackingAlgorithm::ALG_MAXRECTS_BEST_SHORT_SIDE_FIT);
        packAlgorithms.push_back(PackingAlgorithm::ALG_MAXRECTS_BOTTOM_LEFT);
        packAlgorithms.push_back(PackingAlgorithm::ALG_MAXRRECT_BEST_CONTACT_POINT);
    }
    else if (CompareCaseInsensitive(alg, "maxrect_fast") == 0)
    {
        packAlgorithms.push_back(PackingAlgorithm::ALG_MAXRECTS_BEST_AREA_FIT);
    }
    else if (CompareCaseInsensitive(alg, "basic") == 0)
    {
        packAlgorithms.push_back(PackingAlgorithm::ALG_BASIC);
    }
    else
    {
        AddError(Format("Unknown algorithm: '%s'", alg.c_str()));
        SetCanceled();
        return;
    }

    if (tag.empty() == false && std::find(allTags.begin(), allTags.end(), tag) == allTags.end())
    {
        AddError(Format("Tag '%s' is not found in allTags", tag.c_str()));
        SetCanceled();
        return;
    }

    if (ignoresListPath.IsEmpty() == false)
    {
        ignoredFiles = ResourcePacker2DDetails::ReadIgnoresList(ignoresListPath, inputGfxDirectory);
    }

    requestedGPUs = forGPUs;
    outputDirModified = false;

    gfxDirName = inputGfxDirectory.GetLastDirectoryName();
    std::transform(gfxDirName.begin(), gfxDirName.end(), gfxDirName.begin(), ::tolower);

    FilePath processDirectoryPath = rootDirectory + GetProcessFolderName();
    FileSystem::Instance()->CreateDirectory(processDirectoryPath, true);

    FileSystem::Instance()->CreateDirectory(outputGfxDirectory, true);

    bool outputGfxDirChanged = RecalculateDirMD5(outputGfxDirectory, processDirectoryPath + gfxDirName + ".md5", true);
    if (outputGfxDirChanged)
    {
        if (Engine::Instance()->IsConsoleMode())
        {
            Logger::FrameworkDebug("[Gfx not available or changed - performing full repack]");
        }
        outputDirModified = true;

        // Remove whole output directory
        if (clearOutputDirectory)
        {
            bool isDeleted = FileSystem::Instance()->DeleteDirectory(outputGfxDirectory);
            if (isDeleted)
            {
                Logger::FrameworkDebug("Removed output directory: %s", outputGfxDirectory.GetAbsolutePathname().c_str());
            }
            else
            {
                if (FileSystem::Instance()->IsDirectory(outputGfxDirectory))
                {
                    AddError(Format("Can't delete directory [%s]", outputGfxDirectory.GetAbsolutePathname().c_str()));
                }
            }
        }
    }

    PackRecursively(inputGfxDirectory, outputGfxDirectory, packAlgorithms);

    // Put latest md5 after convertation
    RecalculateDirMD5(outputGfxDirectory, processDirectoryPath + gfxDirName + ".md5", true);
}

void ResourcePacker2D::RecalculateMD5ForOutputDir()
{
    gfxDirName = inputGfxDirectory.GetLastDirectoryName();
    std::transform(gfxDirName.begin(), gfxDirName.end(), gfxDirName.begin(), ::tolower);

    FilePath processDirectoryPath = rootDirectory + GetProcessFolderName();
    FileSystem::Instance()->CreateDirectory(processDirectoryPath, true);

    RecalculateDirMD5(outputGfxDirectory, processDirectoryPath + gfxDirName + ".md5", true);
}

bool ResourcePacker2D::ReadMD5FromFile(const FilePath& md5file, MD5::MD5Digest& digest) const
{
    ScopedPtr<File> file(File::Create(md5file, File::OPEN | File::READ));
    if (file)
    {
        auto bytesRead = file->Read(digest.digest.data(), static_cast<uint32>(digest.digest.size()));
        DVASSERT(bytesRead == MD5::MD5Digest::DIGEST_SIZE && "We should always read 16 bytes from md5 file");
        return true;
    }
    else
    {
        return false;
    }
}

void ResourcePacker2D::WriteMD5ToFile(const FilePath& md5file, const MD5::MD5Digest& digest) const
{
    ScopedPtr<File> file(File::Create(md5file, File::CREATE | File::WRITE));
    DVASSERT(file && "Can't create md5 file");

    auto bytesWritten = file->Write(digest.digest.data(), static_cast<uint32>(digest.digest.size()));
    DVASSERT(bytesWritten == MD5::MD5Digest::DIGEST_SIZE && "16 bytes should be always written for md5 file");
}

bool ResourcePacker2D::RecalculateParamsMD5(const String& params, const FilePath& md5file) const
{
    MD5::MD5Digest oldMD5Digest;
    MD5::MD5Digest newMD5Digest;

    bool oldMD5Read = ReadMD5FromFile(md5file, oldMD5Digest);

    MD5::ForData(reinterpret_cast<const uint8*>(params.data()), static_cast<uint32>(params.size()), newMD5Digest);

    WriteMD5ToFile(md5file, newMD5Digest);

    bool isChanged = true;
    if (oldMD5Read)
    {
        isChanged = !(oldMD5Digest == newMD5Digest);
    }
    return isChanged;
}
bool ResourcePacker2D::RecalculateDirMD5(const FilePath& pathname, const FilePath& md5file, bool isRecursive) const
{
    MD5::MD5Digest oldMD5Digest;
    MD5::MD5Digest newMD5Digest;

    bool oldMD5Read = ReadMD5FromFile(md5file, oldMD5Digest);

    MD5::ForDirectory(pathname, newMD5Digest, isRecursive, false);

    WriteMD5ToFile(md5file, newMD5Digest);

    bool isChanged = true;
    if (oldMD5Read)
    {
        isChanged = !(oldMD5Digest == newMD5Digest);
    }
    return isChanged;
}

bool ResourcePacker2D::RecalculateFileMD5(const FilePath& pathname, const FilePath& md5file) const
{
    FilePath md5FileName = FilePath::CreateWithNewExtension(md5file, ".md5");

    MD5::MD5Digest oldMD5Digest;
    MD5::MD5Digest newMD5Digest;

    bool oldMD5Read = ReadMD5FromFile(md5file, oldMD5Digest);

    MD5::ForFile(pathname, newMD5Digest);

    WriteMD5ToFile(md5file, newMD5Digest);

    bool isChanged = true;
    if (oldMD5Read)
    {
        isChanged = !(oldMD5Digest == newMD5Digest);
    }
    return isChanged;
}

Vector<String> ResourcePacker2D::FetchFlags(const FilePath& flagsPathname)
{
    Vector<String> tokens;

    ScopedPtr<File> file(File::Create(flagsPathname, File::READ | File::OPEN));
    if (!file)
    {
        AddError(Format("Failed to open file: %s", flagsPathname.GetAbsolutePathname().c_str()));
        return tokens;
    }

    String tokenString = file->ReadLine();
    Split(tokenString, " ", tokens, false);

    return tokens;
}

uint32 ResourcePacker2D::GetMaxTextureSize() const
{
    uint32 maxTextureSize = TexturePacker::DEFAULT_TEXTURE_SIZE;
    String tsizeValue = CommandLineParser::Instance()->GetParamForFlag("--tsize");
    if (!tsizeValue.empty())
    {
        uint32 fetchedValue;
        int fetchedCount = sscanf(tsizeValue.c_str(), "%u", &fetchedValue);
        if (fetchedCount == 1 && IsPowerOf2(fetchedValue))
        {
            maxTextureSize = fetchedValue;
        }
        else
        {
            Logger::Warning("--tsize value '%s' is incorrect: should be uint of power of 2. Using default value: %u", tsizeValue.c_str(), maxTextureSize);
        }
    }

    return maxTextureSize;
}

void ResourcePacker2D::PackRecursively(const FilePath& inputDir, const FilePath& outputDir, const Vector<PackingAlgorithm>& packAlgorithms, const Vector<String>& passedFlags)
{
    using namespace ResourcePacker2DDetails;

    DVASSERT(inputDir.IsDirectoryPathname() && outputDir.IsDirectoryPathname());

    if (cancelled)
    {
        return;
    }

    uint64 packTime = SystemTimer::GetMs();

    String inputRelativePath = inputDir.GetRelativePathname(rootDirectory);
    FilePath processDir = rootDirectory + GetProcessFolderName() + inputRelativePath;
    FileSystem::Instance()->CreateDirectory(processDir, true);

    if (forceRepack)
    {
        FileSystem::Instance()->DeleteDirectoryFiles(processDir, false);
    }

    FileSystem::Instance()->CreateDirectory(outputDir);

    Vector<String> currentFlags;

    const auto flagsPathname = inputDir + "flags.txt";
    if (FileSystem::Instance()->Exists(flagsPathname))
    {
        currentFlags = FetchFlags(flagsPathname);
    }
    else
    {
        currentFlags = passedFlags;
    }

    CommandLineParser::Instance()->SetFlags(currentFlags);
    String mergedFlags;
    Merge(currentFlags, ' ', mergedFlags);

    String packingParams = mergedFlags;

    for (eGPUFamily gpu : requestedGPUs)
    {
        packingParams += String("GPU = ") + GlobalEnumMap<eGPUFamily>::Instance()->ToString(gpu);
    }

    packingParams += String("PackerVersion = ") + VERSION;
    packingParams += String("LibPSDVersion = ") + INTERNAL_LIBPSD_VERSION;
    for (const auto& algorithm : packAlgorithms)
    {
        packingParams += String("PackerAlgorithm = ") + GlobalEnumMap<DAVA::PackingAlgorithm>::Instance()->ToString(static_cast<int>(algorithm));
    }

    if (tag.empty() == false)
    {
        packingParams += String("Tag = ") + tag;
    }

    ScopedPtr<FileList> fileList(new FileList(inputDir));
    fileList->Sort();

    uint64 allFilesSize = 0;

    struct PickedFile
    {
        String name;
        String basename;
        String ext;
        uint32 index = 0;
        bool tagged = false;
        String outName;
        String outBasename;
    };
    List<PickedFile> pickedFiles;
    List<PickedFile*> taggedFiles;

    for (uint32 fi = 0; fi < fileList->GetCount(); ++fi)
    {
        if (fileList->IsDirectory(fi) == true)
        {
            continue;
        }

        String filename = fileList->GetFilename(fi);
        if (IsNameInHardcodedIgnoreList(filename) || IsPathInIgnoreList(fileList->GetPathname(fi), ignoredFiles))
        {
            continue;
        }

        PickedFile file;
        file.name = std::move(filename);
        file.index = fi;
        SplitFileName(file.name, file.basename, file.ext);
        file.tagged = IsBasenameContainsTag(file.basename, tag);

        if (file.tagged == true)
        {
            file.outBasename = GetBasenameWithoutTag(file.basename, tag);
            file.outName = file.outBasename + file.ext;
            pickedFiles.emplace_back(std::move(file));
            taggedFiles.push_back(&(pickedFiles.back()));
        }
        else
        {
            bool containsExcludedTag = false;
            for (const String& tagToExclude : allTags)
            {
                if (tagToExclude != tag && IsBasenameContainsTag(file.basename, tagToExclude) == true)
                {
                    containsExcludedTag = true;
                    break;
                }
            }
            if (containsExcludedTag)
            {
                continue;
            }

            file.outBasename = file.basename;
            file.outName = file.name;
            pickedFiles.emplace_back(std::move(file));
        }
    }

    for (PickedFile* taggedFile : taggedFiles)
    {
        List<PickedFile>::iterator substitutedFileFound = std::find_if(pickedFiles.begin(), pickedFiles.end(), [taggedFile](PickedFile& next)
                                                                       {
                                                                           return (next.name == taggedFile->outName);
                                                                       });

        if (substitutedFileFound != pickedFiles.end() && substitutedFileFound->tagged == false)
        {
            pickedFiles.erase(substitutedFileFound);
        }
    }

    for (const PickedFile& file : pickedFiles)
    {
        packingParams += file.name;
        allFilesSize += fileList->GetFileSize(file.index);
    }

    packingParams += Format("FilesSize = %llu", allFilesSize);
    packingParams += Format("FilesCount = %u", pickedFiles.size());
    packingParams += Format("DescriptorVersion = %i", TextureDescriptor::CURRENT_VERSION);

    bool inputDirModified = RecalculateDirMD5(inputDir, processDir + "dir.md5", false);
    bool paramsModified = RecalculateParamsMD5(packingParams, processDir + "params.md5");

    bool modified = outputDirModified || inputDirModified || paramsModified;
    if (modified)
    {
        if (pickedFiles.empty() == false)
        {
            AssetCache::CacheItemKey cacheKey;
            if (IsUsingCache())
            {
                MD5::MD5Digest digest;

                ReadMD5FromFile(processDir + "dir.md5", digest);
                cacheKey.SetPrimaryKey(digest);

                ReadMD5FromFile(processDir + "params.md5", digest);
                cacheKey.SetSecondaryKey(digest);
            }

            bool needRepack = (false == GetFilesFromCache(cacheKey, inputDir, outputDir));
            if (needRepack)
            {
                // read textures margins settings
                bool useTwoSideMargin = CommandLineParser::Instance()->IsFlagSet("--add2sidepixel");
                uint32 marginInPixels = useTwoSideMargin ? 0 : 1;
                if (CommandLineParser::Instance()->IsFlagSet("--add0pixel"))
                    marginInPixels = 0;
                else if (CommandLineParser::Instance()->IsFlagSet("--add1pixel"))
                    marginInPixels = 1;
                else if (CommandLineParser::Instance()->IsFlagSet("--add2pixel"))
                    marginInPixels = 2;
                else if (CommandLineParser::Instance()->IsFlagSet("--add4pixel"))
                    marginInPixels = 4;

                uint32 maxTextureSize = GetMaxTextureSize();

                bool withAlpha = CommandLineParser::Instance()->IsFlagSet("--disableCropAlpha");
                bool useLayerNames = CommandLineParser::Instance()->IsFlagSet("--useLayerNames");
                bool verbose = CommandLineParser::Instance()->GetVerbose();

                if (clearOutputDirectory)
                {
                    FileSystem::Instance()->DeleteDirectoryFiles(outputDir, false);
                }

                DefinitionFile::Collection definitionFileList;
                Vector<PickedFile*> justCopyList;
                definitionFileList.reserve(pickedFiles.size());
                for (PickedFile& file : pickedFiles)
                {
                    if (cancelled)
                    {
                        break;
                    }

                    DAVA::RefPtr<DefinitionFile> defFile(new DefinitionFile());

                    bool shouldAcceptFile = false;

                    FilePath path = fileList->GetPathname(file.index);
                    if (CompareCaseInsensitive(file.ext, ".psd") == 0)
                    {
                        shouldAcceptFile = defFile->LoadPSD(path, processDir, maxTextureSize,
                                                            withAlpha, useLayerNames, verbose, file.outBasename);
                    }
                    else if (CompareCaseInsensitive(file.ext, ".pngdef") == 0)
                    {
                        shouldAcceptFile = defFile->LoadPNGDef(path, processDir, file.outBasename);
                    }
                    else if (TextureDescriptor::IsSupportedTextureExtension(file.ext) == true)
                    {
                        shouldAcceptFile = defFile->LoadImage(path, processDir, file.outBasename);
                    }
                    else
                    {
                        justCopyList.push_back(&file);
                    }

                    if (shouldAcceptFile)
                    {
                        definitionFileList.push_back(defFile);
                    }
                }

                if (!definitionFileList.empty())
                {
                    TexturePacker packer;
                    packer.SetConvertQuality(quality);

                    if (isLightmapsPacking)
                    {
                        packer.SetUseOnlySquareTextures();
                        packer.SetMaxTextureSize(2048);
                    }
                    else
                    {
                        if (CommandLineParser::Instance()->IsFlagSet("--square"))
                        {
                            packer.SetUseOnlySquareTextures();
                        }
                        packer.SetMaxTextureSize(maxTextureSize);
                    }

                    packer.SetTwoSideMargin(useTwoSideMargin);
                    packer.SetTexturesMargin(marginInPixels);
                    packer.SetAlgorithms(packAlgorithms);
                    packer.SetTexturePostfix(texturePostfix);

                    if (CommandLineParser::Instance()->IsFlagSet("--split"))
                    {
                        packer.PackToTexturesSeparate(outputDir, definitionFileList, requestedGPUs);
                    }
                    else
                    {
                        packer.PackToTextures(outputDir, definitionFileList, requestedGPUs);
                    }

                    Set<String> currentErrors = packer.GetErrors();
                    if (!currentErrors.empty())
                    {
                        errors.insert(currentErrors.begin(), currentErrors.end());
                    }
                }

                for (const PickedFile* file : justCopyList)
                {
                    FilePath srcPath = inputDir + file->name;
                    FilePath destPath = outputDir + file->outName;
                    if (!FileSystem::Instance()->CopyFile(srcPath, destPath))
                    {
                        Logger::Error("Can't copy %s to %s", srcPath.GetStringValue().c_str(), destPath.GetStringValue().c_str());
                    }
                }

                packTime = SystemTimer::GetMs() - packTime;

                if (Engine::Instance()->IsConsoleMode())
                {
                    Logger::Info("[%u files packed with flags: %s]", static_cast<uint32>(definitionFileList.size()), mergedFlags.c_str());
                }

                const char* result = definitionFileList.empty() ? "[unchanged]" : "[REPACKED]";
                Logger::Info("[%s - %.2lf secs] - %s", inputDir.GetAbsolutePathname().c_str(),
                             static_cast<float64>(packTime) / 1000.0, result);

                AddFilesToCache(cacheKey, inputDir, outputDir);
            }
        }
        else if (outputDirModified || inputDirModified)
        {
            Logger::Info("[%s] - empty directory. Clearing output folder", inputDir.GetAbsolutePathname().c_str());
            FileSystem::Instance()->DeleteDirectoryFiles(outputDir, false);
        }
    }
    else
    {
        Logger::Info("[%s] - unchanged", inputDir.GetAbsolutePathname().c_str());
    }

    const auto& flagsToPass = CommandLineParser::Instance()->IsFlagSet("--recursive") ? currentFlags : passedFlags;

    for (uint32 fi = 0; fi < fileList->GetCount(); ++fi)
    {
        if (fileList->IsDirectory(fi))
        {
            String filename = fileList->GetFilename(fi);
            if (!fileList->IsNavigationDirectory(fi) && (filename != "$process") && (filename != ".svn"))
            {
                if ((filename.size() > 0) && (filename[0] != '.'))
                {
                    FilePath input = inputDir + filename;
                    input.MakeDirectoryPathname();

                    FilePath output = outputDir + filename;
                    output.MakeDirectoryPathname();

                    PackRecursively(input, output, packAlgorithms, flagsToPass);
                }
            }
        }
    }
}

void ResourcePacker2D::SetCacheClient(AssetCacheClient* cacheClient_, const String& comment)
{
    cacheClient = cacheClient_;

    cacheItemDescription.machineName = UTF8Utils::EncodeToUTF8(DeviceInfo::GetName());

    DateTime timeNow = DateTime::Now();
    cacheItemDescription.creationDate = UTF8Utils::EncodeToUTF8(timeNow.GetLocalizedDate()) + "_" + UTF8Utils::EncodeToUTF8(timeNow.GetLocalizedTime());

    cacheItemDescription.comment = comment;
}

void ResourcePacker2D::SetTexturePostfix(const String& postfix)
{
    texturePostfix = postfix;
}

void ResourcePacker2D::SetTag(const String& tag_)
{
    tag = tag_;
}

void DAVA::ResourcePacker2D::SetAllTags(const Vector<String>& tags)
{
    allTags = tags;
}

void ResourcePacker2D::SetIgnoresFile(const String& ignoresPath)
{
    ignoresListPath = ignoresPath;
}

bool ResourcePacker2D::GetFilesFromCache(const AssetCache::CacheItemKey& key, const FilePath& inputPath, const FilePath& outputPath)
{
#ifdef __DAVAENGINE_WIN_UAP__
    //no cache client in win uap
    return false;
#else

    if (!IsUsingCache())
    {
        return false;
    }

    String requestedDataRelativePath = "..." + inputPath.GetRelativePathname(dataSourceDirectory);

    AssetCache::CachedItemValue retrievedData;
    AssetCache::Error requestError = cacheClient->RequestFromCacheSynchronously(key, &retrievedData);
    if (requestError == AssetCache::Error::NO_ERRORS)
    {
        Logger::Info("%s - retrieved from cache", requestedDataRelativePath.c_str());
        retrievedData.ExportToFolder(outputPath);
        return true;
    }
    else
    {
        String errorInfo = AssetCache::ErrorToString(requestError);
        if (requestError == AssetCache::Error::OPERATION_TIMEOUT)
        {
            errorInfo.append(Format(" (%u ms)", cacheClient->GetTimeoutMs()));
        }

        Logger::Info("%s - can't retrieve from cache: %s", requestedDataRelativePath.c_str(), errorInfo.c_str());
    }

    return false;
#endif
}

bool ResourcePacker2D::AddFilesToCache(const AssetCache::CacheItemKey& key, const FilePath& inputPath, const FilePath& outputPath)
{
#ifdef __DAVAENGINE_WIN_UAP__
    //no cache client in win uap
    return false;
#else
    if (!IsUsingCache())
    {
        return false;
    }

    AssetCache::CachedItemValue value;

    ScopedPtr<FileList> outFilesList(new FileList(outputPath));
    for (uint32 fi = 0; fi < outFilesList->GetCount(); ++fi)
    {
        if (!outFilesList->IsDirectory(fi))
        {
            value.Add(outFilesList->GetPathname(fi));
        }
    }

    String addedDataRelativePath = "..." + inputPath.GetRelativePathname(dataSourceDirectory);

    if (!value.IsEmpty())
    {
        value.UpdateValidationData();
        value.SetDescription(cacheItemDescription);

        AssetCache::Error addError = cacheClient->AddToCacheSynchronously(key, value);
        if (addError == AssetCache::Error::NO_ERRORS)
        {
            Logger::Info("%s - added to cache", addedDataRelativePath.c_str());
            return true;
        }
        else
        {
            String errorInfo = AssetCache::ErrorToString(addError);
            if (addError == AssetCache::Error::OPERATION_TIMEOUT)
            {
                errorInfo.append(Format(" (%u ms)", cacheClient->GetTimeoutMs()));
            }

            Logger::Info("%s - can't add to cache: %s", addedDataRelativePath.c_str(), errorInfo.c_str());
        }
    }
    else
    {
        Logger::Info("%s - empty folder", addedDataRelativePath.c_str());
    }

    return false;

#endif
}

const Set<String>& ResourcePacker2D::GetErrors() const
{
    return errors;
}

void ResourcePacker2D::AddError(const String& errorMsg)
{
    Logger::Error(errorMsg.c_str());
    errors.insert(errorMsg);
}

bool ResourcePacker2D::IsUsingCache() const
{
#ifdef __DAVAENGINE_WIN_UAP__
    //no cache in win uap
    return false;
#else
    return (cacheClient != nullptr) && cacheClient->IsConnected();
#endif
}
};
