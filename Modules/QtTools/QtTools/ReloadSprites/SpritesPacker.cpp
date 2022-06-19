#include "SpritesPacker.h"

#include <AssetCache/AssetCacheClient.h>

#include <Render/2D/Sprite.h>
#include <Logger/Logger.h>
#include <Time/SystemTimer.h>

#include <QDir>
#include <QDirIterator>

using namespace DAVA;

SpritesPacker::SpritesPacker(QObject* parent)
    : QObject(parent)
    , running(false)
{
}

void SpritesPacker::AddTask(const QDir& inputDir, const QDir& outputDir)
{
    tasks.push_back(qMakePair(inputDir, outputDir));
}

void SpritesPacker::ClearTasks()
{
    tasks.clear();
}

void SpritesPacker::ReloadSprites(bool clearDirs, bool forceRepack, const eGPUFamily gpu, const TextureConverter::eConvertQuality quality)
{
    SetRunning(true);

    for (const auto& task : tasks)
    {
        const auto& inputDir = task.first;
        const auto& outputDir = task.second;
        if (!outputDir.exists())
        {
            outputDir.mkpath(".");
        }

        const FilePath inputFilePath = FilePath(inputDir.absolutePath().toStdString()).MakeDirectoryPathname();
        const FilePath outputFilePath = FilePath(outputDir.absolutePath().toStdString()).MakeDirectoryPathname();

        resourcePacker2D.forceRepack = forceRepack;
        resourcePacker2D.clearOutputDirectory = clearDirs;
        resourcePacker2D.SetConvertQuality(quality);
        resourcePacker2D.InitFolders(inputFilePath, outputFilePath);
        DAVA::int64 packTime = SystemTimer::GetMs();
        resourcePacker2D.PackResources({ gpu });
        packTime = SystemTimer::GetMs() - packTime;
        Logger::Info("Sprites reload time: %.2lf sec", static_cast<float64>(packTime) / 1000.0);
        if (resourcePacker2D.IsCancelled())
        {
            break;
        }
    }

    if (IsUsingCache())
    {
        cacheClient->DumpStats();
        cacheClient->ClearStats();
    }

    SetRunning(false);
}

void SpritesPacker::Cancel()
{
    resourcePacker2D.SetCanceled();
}

bool SpritesPacker::IsRunning() const
{
    return running;
}

void SpritesPacker::SetRunning(bool arg)
{
    if (arg != running)
    {
        running = arg;
        if (!arg)
        {
            emit Finished();
        }
        String message = String("Sprites packer ") + (arg ? "started" : (resourcePacker2D.IsCancelled() ? "canceled" : "finished"));
        Logger::FrameworkDebug(message.c_str());
        emit RunningStateChanged(arg);
    }
}

const DAVA::ResourcePacker2D& SpritesPacker::GetResourcePacker() const
{
    return resourcePacker2D;
}

void SpritesPacker::SetCacheClient(AssetCacheClient* cacheClient_, const String& comment)
{
    cacheClient = cacheClient_;
    resourcePacker2D.SetCacheClient(cacheClient, comment);
}

bool SpritesPacker::IsUsingCache() const
{
    return resourcePacker2D.IsUsingCache();
}
