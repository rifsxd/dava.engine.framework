#pragma once

#include "Classes/Application/QEGlobal.h"
#include "Classes/EditorSystems/EditorSystemsManager.h"
#include "Classes/UI/Preview/PreviewWidget.h"
#include "Classes/Utils/PackageListenerProxy.h"

#include <TArc/Core/ControllerModule.h>
#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Utils/QtConnections.h>
#include <TArc/Utils/QtDelayedExecutor.h>

class FindInDocumentController;
class EditorSystemsManager;
class PackageNode;
class ControlNode;

class DocumentsModule : public DAVA::ControllerModule, PackageListener
{
public:
    DocumentsModule();
    ~DocumentsModule() override;

protected:
    void OnRenderSystemInitialized(DAVA::Window* window) override;
    bool CanWindowBeClosedSilently(const DAVA::WindowKey& key, DAVA::String& requestWindowText) override;
    bool SaveOnWindowClose(const DAVA::WindowKey& key) override;
    void RestoreOnWindowClose(const DAVA::WindowKey& key) override;

    void PostInit() override;
    void OnWindowClosed(const DAVA::WindowKey& key) override;
    void OnInterfaceRegistered(const DAVA::Type* interfaceType) override;
    void OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType) override;

    void OnContextCreated(DAVA::DataContext* context) override;
    void OnContextDeleted(DAVA::DataContext* context) override;

    void OnContextWillBeChanged(DAVA::DataContext* current, DAVA::DataContext* newOne) override;

private:
    void InitCentralWidget();
    void InitGlobalData();

    void CreateDocumentsActions();
    void RegisterOperations();

    //Edit
    void CreateEditActions();
    void OnUndo();
    void OnRedo();

    //View
    void CreateViewActions();
    void CreateFindActions();

    void OpenPackageFiles(const QStringList& links);
    DAVA::DataContext::ContextID OpenDocument(const QString& path);
    DAVA::RefPtr<PackageNode> CreatePackage(const QString& path);

    void CloseDocument(DAVA::uint64 id);
    void CloseAllDocuments();
    void DeleteAllDocuments();
    void CloseDocuments(const DAVA::Set<DAVA::DataContext::ContextID>& ids);

    void ReloadCurrentDocument();
    void ReloadDocument(const DAVA::DataContext::ContextID& contextID);
    void ReloadDocuments(const DAVA::Set<DAVA::DataContext::ContextID>& ids);

    bool HasUnsavedDocuments() const;
    bool SaveDocument(const DAVA::DataContext::ContextID& contextID);
    bool SaveAllDocuments();
    bool SaveCurrentDocument();
    void DiscardUnsavedChanges();

    void SelectControl(const QString& documentPath, const QString& controlPath);

    void OnEmulationModeChanged(bool mode);

    //previewWidget helper functions
    void ChangeControlText(ControlNode* node);

    //documents watcher
    void OnFileChanged(const QString& path);
    void OnApplicationStateChanged(Qt::ApplicationState state);

    void ApplyFileChanges();
    DAVA::DataContext::ContextID GetContextByPath(const QString& path) const;

    void ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index) override;

    void OnSelectInFileSystem();
    void OnDroppingFile(bool droppingFile);

    QPointer<PreviewWidget> previewWidget;
    DAVA::QtConnections connections;

    DAVA::QtDelayedExecutor delayedExecutor;

    PackageListenerProxy packageListenerProxy;

    DAVA_VIRTUAL_REFLECTION(DocumentsModule, DAVA::ControllerModule);
};
