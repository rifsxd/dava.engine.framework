#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <AssetCache/AssetCacheClient.h>

class ProjectData;
class SpritesPackerModule : public DAVA::ClientModule
{
public:
    SpritesPackerModule();
    ~SpritesPackerModule() override;

private:
    void OnReloadFinished();
    void OnReloadSprites();

    void PostInit() override;
    void OnWindowClosed(const DAVA::WindowKey& key) override;

    void CreateActions();
    bool IsUsingAssetCache() const;
    void SetUsingAssetCacheEnabled(bool enabled);
    void EnableCacheClient();
    void DisableCacheClient();

    DAVA::QtConnections connections;

    std::unique_ptr<DAVA::AssetCacheClient> cacheClient;
    DAVA::AssetCacheClient::ConnectionParams connectionParams;

    DAVA_VIRTUAL_REFLECTION(SpritesPackerModule, DAVA::ClientModule);
};
