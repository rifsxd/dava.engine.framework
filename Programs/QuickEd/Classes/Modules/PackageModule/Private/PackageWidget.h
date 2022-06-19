#pragma once

#include "Classes/EditorSystems/SelectionContainer.h"

#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>

#include <QWidget>
#include <QModelIndex>

namespace DAVA
{
class Any;
class ContextAccessor;
class UI;
}

struct PackageContext;
class PackageBaseNode;
class PackageModel;
class PackageNode;
class QItemSelection;
class PackageTreeView;

class PackageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PackageWidget(DAVA::ContextAccessor* accessor, DAVA::UI* ui, QWidget* parent = 0);

    PackageModel* GetPackageModel() const;

    void OnSelectionChanged(const DAVA::Any& selection);
    void OnPackageChanged(PackageContext* context, PackageNode* node);

signals:
    void SelectedNodesChanged(const SelectedNodes& selection);

public slots:
    void OnSelectionChangedFromView(const QItemSelection& proxySelected, const QItemSelection& proxyDeselected);
    void OnRename();
    void OnBeforeProcessNodes(const SelectedNodes& nodes);
    void OnAfterProcessNodes(const SelectedNodes& nodes);
    void ExpandToFirstChild();

private:
    void SetSelectedNodes(const SelectedNodes& selection);
    void CollectExpandedIndexes(PackageBaseNode* node);
    void LoadContext();
    void SaveContext() const;

    void DeselectNodeImpl(PackageBaseNode* node);
    void SelectNodeImpl(PackageBaseNode* node);

    QModelIndexList GetExpandedIndexes() const;
    void RestoreExpandedIndexes(const QModelIndexList& indexes);

private:
    friend class PackageModule;

    PackageModel* packageModel = nullptr;
    PackageTreeView* treeView = nullptr;

    SelectionContainer selectionContainer;
    SelectedNodes expandedNodes;
    PackageContext* currentContext = nullptr;
};

struct PackageContext
{
    QModelIndexList expandedIndexes;
};
