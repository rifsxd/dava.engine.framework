#pragma once

#include "Classes/Modules/LibraryModule/LibraryData.h"
#include "Classes/Utils/PackageListenerProxy.h"

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

class ProjectData;

namespace DAVA
{
class FieldBinder;
class QtAction;
}

class LibraryModule : public DAVA::ClientModule, PackageListener
{
    // ClientModule
    void PostInit() override;

    void InitData();
    void InitUI();
    void BindFields();
    void CreateActions();

    void AddProjectControls(const ProjectData* projectData, const DAVA::Vector<DAVA::RefPtr<PackageNode>>& libraryPackages);
    void RemoveProjectControls();

    void AddProjectPinnedControls(const ProjectData* projectData, const DAVA::Vector<DAVA::RefPtr<PackageNode>>& libraryPackages);
    void AddProjectLibraryControls(const ProjectData* projectData, const DAVA::Vector<DAVA::RefPtr<PackageNode>>& libraryPackages);
    void AddDefaultControls();

    void AddPrototypes(const PackageNode* packageNode, const QUrl& menuPoint, const QUrl& toolbarMenuPoint);

    void AddPackagePrototypes(PackageNode* packageNode);
    void RemovePackagePrototypes();

    void AddImportedPrototypes(const PackageNode* importedPackage);
    void RemoveImportedPrototypes(const PackageNode* importedPackage);

    void ClearActions(LibraryData::ActionsMap&);

    void AddControlAction(ControlNode* controlNode, bool isPrototype, const QUrl& menuPoint, const QUrl& toolbarMenuPoint, LibraryData::ActionsMap& actionsMap);
    void RemoveControlAction(ControlNode* node, LibraryData::ActionsMap& actionsMap);

    void OnProjectPathChanged(const DAVA::Any& projectPath);
    void OnControlCreateTriggered(ControlNode* node, bool makePrototype);

    // PackageListenerProxy
    void ActivePackageNodeWasChanged(PackageNode* node) override;
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int row) override;
    void ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index) override;
    void ImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from) override;

    DAVA::Vector<DAVA::RefPtr<PackageNode>> LoadLibraryPackages(ProjectData* projectData);

    LibraryData* GetLibraryData();

    QString GenerateUniqueName();

    std::unique_ptr<DAVA::FieldBinder> fieldBinder;
    DAVA::QtConnections connections;

    PackageListenerProxy packageListenerProxy;

    DAVA::uint64 uniqueNumber = 0;

    DAVA_VIRTUAL_REFLECTION(LibraryModule, DAVA::ClientModule);
};
