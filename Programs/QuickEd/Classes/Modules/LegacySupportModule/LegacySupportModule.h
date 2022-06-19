#pragma once

#include "Modules/LegacySupportModule/Private/Project.h"

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <TArc/DataProcessing/DataWrapper.h>
#include <TArc/DataProcessing/DataListener.h>

class Project;

class LegacySupportModule : public DAVA::ClientModule, private DAVA::DataListener
{
    void PostInit() override;
    void OnWindowClosed(const DAVA::WindowKey& key) override;

    void OnDataChanged(const DAVA::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

    void InitMainWindow();

    DAVA::QtConnections connections;
    DAVA::DataWrapper projectDataWrapper;
    std::unique_ptr<Project> project;

    DAVA_VIRTUAL_REFLECTION(LegacySupportModule, DAVA::ClientModule);
};
