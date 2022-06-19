#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/DataProcessing/DataWrapper.h>
#include <TArc/DataProcessing/DataListener.h>

#include <TArc/Utils/QtConnections.h>

class FileSystemCacheModule : public DAVA::ClientModule, public DAVA::DataListener
{
    void PostInit() override;
    void OnWindowClosed(const DAVA::WindowKey& key) override;
    void OnDataChanged(const DAVA::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

    void CreateActions();
    void FastOpenDocument();

    DAVA::QtConnections connections;
    DAVA::DataWrapper projectDataWrapper;

    DAVA_VIRTUAL_REFLECTION(FileSystemCacheModule, DAVA::ClientModule);
};
