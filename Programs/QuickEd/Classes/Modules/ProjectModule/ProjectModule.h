#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Models/RecentMenuItems.h>
#include <TArc/Core/OperationRegistrator.h>
#include <TArc/Utils/QtConnections.h>
#include <TArc/Utils/QtDelayedExecutor.h>

namespace DAVA
{
class ResultList;
}
class ProjectData;

class ProjectModule : public DAVA::ClientModule
{
public:
    ProjectModule();
    ~ProjectModule() override;

private:
    void PostInit() override;
    void OnWindowClosed(const DAVA::WindowKey& key) override;

    void CreateActions();

    void OnOpenProject();
    void OnNewProject();
    void CreateProject(const QString& projectDir);

    bool CloseProject();
    void OpenProject(const DAVA::String& path);
    void OpenLastProject();
    void ShowResultList(const QString& title, const DAVA::ResultList& resultList);
    void LoadPlugins();
    void RegisterFolders();
    void UnregisterFolders();

    DAVA::QtConnections connections;
    std::unique_ptr<DAVA::RecentMenuItems> recentProjects;
    DAVA::QtDelayedExecutor delayedExecutor;

    DAVA_VIRTUAL_REFLECTION(ProjectModule, DAVA::ClientModule);
};

namespace ProjectModuleTesting
{
DECLARE_OPERATION_ID(CreateProjectOperation);
}
