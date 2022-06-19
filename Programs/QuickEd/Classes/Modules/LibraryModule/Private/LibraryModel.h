#pragma once

#include "Base/BaseTypes.h"
#include "Base/RefPtr.h"
#include "Model/PackageHierarchy/PackageListener.h"
#include <QStandardItemModel>

class PackageNode;
class PackageBaseNode;
class AbstractProperty;
class ControlNode;
class ControlsContainerNode;
class ImportedPackagesNode;

namespace DAVA
{
class UI;
class ContextAccessor;
}

class LibraryModel : public QStandardItemModel, PackageListener
{
    Q_OBJECT
    enum
    {
        POINTER_DATA = Qt::UserRole + 1,
        PROTOTYPE
    };

public:
    LibraryModel(DAVA::UI* ui, DAVA::ContextAccessor* accessor, QObject* parent = nullptr);
    ~LibraryModel() override;

    void SetLibraryPackages(const DAVA::Vector<DAVA::RefPtr<PackageNode>>& libraryPackages);

    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    void SetPackageNode(PackageNode* package);

    QModelIndex GetDefaultControlsModelIndex() const;

private:
    QVariant data(const QModelIndex& index, int role) const override;

    QModelIndex indexByNode(const void* node, const QStandardItem* item) const;
    void AddControl(ControlNode* node, QStandardItem* rootItem, bool makePrototype);
    void AddPackageControls(PackageControlsNode* packageControls, QStandardItem* rootItem, bool makePrototype);
    QStandardItem* CreatePackageControlsItem(PackageNode* package, bool makePrototype);

    //Package Signals
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int row) override;
    void ControlWillBeRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ImportedPackageWasAdded(PackageNode* node, ImportedPackagesNode* to, int index) override;
    void ImportedPackageWillBeRemoved(PackageNode* node, ImportedPackagesNode* from) override;
    DAVA::RefPtr<PackageNode> package;

    DAVA::Vector<DAVA::RefPtr<PackageNode>> libraryPackages;
    DAVA::Vector<QStandardItem*> libraryRootItems;

    QStandardItem* defaultControlsRootItem = nullptr;
    QStandardItem* controlsRootItem = nullptr;
    QStandardItem* importedPackageRootItem = nullptr;

    DAVA::Vector<DAVA::RefPtr<ControlNode>> defaultControls;

    DAVA::UI* ui = nullptr;
    DAVA::ContextAccessor* accessor = nullptr;
};
