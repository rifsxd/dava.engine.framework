#pragma once

#include <TextureCompression/TextureConverter.h>

#include <TArc/Core/ContextAccessor.h>

#include <Base/BaseTypes.h>
#include <FileSystem/FilePath.h>
#include <Render/RenderBase.h>

#include <QObject>

class SpritesPacker;
class QAction;
class QDialog;

namespace DAVA
{
class AssetCacheClient;
class UI;
class ContextAccessor;
class WaitHandle;
class ContextAccessor;

class SpritesPackerModule final : public QObject
{
    Q_OBJECT

public:
    SpritesPackerModule(UI* ui, ContextAccessor* accessor);
    ~SpritesPackerModule() override;

    void RepackImmediately(const FilePath& projectPath, eGPUFamily gpu);
    void RepackWithDialog();

    bool IsRunning() const;

signals:
    void SpritesReloaded();

private:
    void SetupSpritesPacker(const FilePath& projectPath);
    void ShowPackerDialog();
    void ReloadObjects();

    void ConnectCacheClient();
    void DisconnectCacheClient();
    void DisconnectCacheClientInternal(AssetCacheClient* cacheClient);

    void ProcessSilentPacking(bool clearDirs, bool forceRepack, const eGPUFamily gpu, const TextureConverter::eConvertQuality quality);

    void CreateWaitDialog(const FilePath& projectPath);
    void CloseWaitDialog();

    void SetCacheClientForPacker();

private:
    AssetCacheClient* cacheClient = nullptr;

    std::unique_ptr<SpritesPacker> spritesPacker;
    QAction* reloadSpritesAction = nullptr;
    UI* ui = nullptr;
    ContextAccessor* accessor = nullptr;
    std::unique_ptr<WaitHandle> waitDialogHandle;
};
} // namespace DAVA
