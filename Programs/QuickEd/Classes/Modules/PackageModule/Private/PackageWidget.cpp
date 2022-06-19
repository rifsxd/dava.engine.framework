#include "Classes/Modules/PackageModule/Private/PackageWidget.h"

#include "Classes/Model/PackageHierarchy/PackageBaseNode.h"
#include "Classes/Model/PackageHierarchy/PackageControlsNode.h"
#include "Classes/Model/PackageHierarchy/PackageNode.h"
#include "Classes/Modules/PackageModule/Private/PackageModel.h"
#include "Classes/Modules/PackageModule/Private/PackageTreeView.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/ActionUtils.h>

#include <Base/Any.h>
#include <Engine/Engine.h>
#include <Engine/Window.h>

#include <QHeaderView>
#include <QVBoxLayout>

PackageWidget::PackageWidget(DAVA::ContextAccessor* accessor, DAVA::UI* ui, QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(5);
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    treeView = new PackageTreeView(this);
    treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
    treeView->setAcceptDrops(true);
    treeView->setDragEnabled(true);
    treeView->setDragDropMode(QAbstractItemView::DragDrop);
    treeView->setDefaultDropAction(Qt::MoveAction);
    treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    treeView->setHeaderHidden(true);

    verticalLayout->addWidget(treeView);

    packageModel = new PackageModel(this);
    packageModel->SetAccessor(accessor);
    packageModel->SetUI(ui);

    treeView->setModel(packageModel);
    treeView->header()->setSectionResizeMode /*setResizeMode*/ (QHeaderView::ResizeToContents);

    connect(packageModel, &PackageModel::BeforeProcessNodes, this, &PackageWidget::OnBeforeProcessNodes);
    connect(packageModel, &PackageModel::AfterProcessNodes, this, &PackageWidget::OnAfterProcessNodes);
    connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PackageWidget::OnSelectionChangedFromView);
}

PackageModel* PackageWidget::GetPackageModel() const
{
    return packageModel;
}

void PackageWidget::OnPackageChanged(PackageContext* context, PackageNode* package)
{
    setEnabled(package != nullptr);

    bool isUpdatesEnabled = treeView->updatesEnabled();
    treeView->setUpdatesEnabled(false);

    SaveContext();
    currentContext = context;
    packageModel->Reset(package);
    ExpandToFirstChild();
    LoadContext();

    treeView->setColumnWidth(0, treeView->size().width());
    treeView->setUpdatesEnabled(isUpdatesEnabled);
}

void PackageWidget::LoadContext()
{
    if (nullptr != currentContext)
    {
        //restore expanded indexes
        RestoreExpandedIndexes(currentContext->expandedIndexes);
    }
}

void PackageWidget::SaveContext() const
{
    if (currentContext == nullptr)
    {
        return;
    }
    currentContext->expandedIndexes = GetExpandedIndexes();
}

void PackageWidget::OnSelectionChangedFromView(const QItemSelection& selectedIndexes, const QItemSelection& deselectedIndexes)
{
    if (nullptr == packageModel)
    {
        return;
    }

    SelectedNodes selected;
    SelectedNodes deselected;

    SelectedNodes selection = selectionContainer.selectedNodes;
    for (const auto& index : deselectedIndexes.indexes())
    {
        selection.erase(static_cast<PackageBaseNode*>(index.internalPointer()));
    }
    for (const auto& index : selectedIndexes.indexes())
    {
        selection.insert(static_cast<PackageBaseNode*>(index.internalPointer()));
    }

    SetSelectedNodes(selection);

    emit SelectedNodesChanged(selection);
}

void PackageWidget::OnRename()
{
    const auto& selected = treeView->selectionModel()->selectedIndexes();
    DVASSERT(selected.size() == 1);
    treeView->edit(selected.first());
}

void PackageWidget::CollectExpandedIndexes(PackageBaseNode* node)
{
    QModelIndex dstIndex = packageModel->indexByNode(node);
    if (treeView->isExpanded(dstIndex))
    {
        expandedNodes.insert(node);
    }
    for (int i = 0, count = node->GetCount(); i < count; ++i)
    {
        CollectExpandedIndexes(node->Get(i));
    }
}

QModelIndexList PackageWidget::GetExpandedIndexes() const
{
    QModelIndexList retval;
    QModelIndex index = treeView->model()->index(0, 0);
    while (index.isValid())
    {
        if (treeView->isExpanded(index))
        {
            retval << index;
        }
        index = treeView->indexBelow(index);
    }

    return retval;
}

void PackageWidget::OnBeforeProcessNodes(const SelectedNodes& nodes)
{
    for (const auto& node : nodes)
    {
        CollectExpandedIndexes(node);
    }
}

void PackageWidget::OnAfterProcessNodes(const SelectedNodes& nodes)
{
    if (nodes.empty())
    {
        return;
    }
    OnSelectionChanged(nodes);
    emit SelectedNodesChanged(selectionContainer.selectedNodes); //this is only way to select manually in package widget
    for (const auto& node : expandedNodes)
    {
        QModelIndex dstIndex = packageModel->indexByNode(node);
        treeView->expand(dstIndex);
    }
    expandedNodes.clear();
}

void PackageWidget::DeselectNodeImpl(PackageBaseNode* node)
{
    QModelIndex dstIndex = packageModel->indexByNode(node);
    DVASSERT(dstIndex.isValid());
    if (dstIndex.isValid())
    {
        treeView->selectionModel()->select(dstIndex, QItemSelectionModel::Deselect);
    }
}

void PackageWidget::SelectNodeImpl(PackageBaseNode* node)
{
    QModelIndex dstIndex = packageModel->indexByNode(node);
    DVASSERT(dstIndex.isValid());

    if (dstIndex.isValid())
    {
        QItemSelectionModel* selectionModel = treeView->selectionModel();
        selectionModel->setCurrentIndex(dstIndex, QItemSelectionModel::Select);
        treeView->scrollTo(dstIndex);
    }
}

void PackageWidget::RestoreExpandedIndexes(const QModelIndexList& indexes)
{
    for (auto& index : indexes)
    {
        DVASSERT(index.isValid());
        if (index.isValid())
        {
            treeView->setExpanded(index, true);
        }
    }
}

void PackageWidget::OnSelectionChanged(const DAVA::Any& selectionValue)
{
    disconnect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PackageWidget::OnSelectionChangedFromView);
    SelectedNodes selection = selectionValue.Cast<SelectedNodes>(SelectedNodes());
    SetSelectedNodes(selection);
    connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PackageWidget::OnSelectionChangedFromView);
}

void PackageWidget::SetSelectedNodes(const SelectedNodes& selection)
{
    //this code is used to synchronize last selected item with properties model
    SelectedNodes selected;
    SelectedNodes deselected;

    selectionContainer.GetNotExistedItems(selection, selected);
    SelectionContainer tmpContainer = { selection };
    tmpContainer.GetNotExistedItems(selectionContainer.selectedNodes, deselected);

    selectionContainer.selectedNodes = selection;

    for (PackageBaseNode* node : deselected)
    {
        DeselectNodeImpl(node);
    }

    for (PackageBaseNode* node : selected)
    {
        SelectNodeImpl(node);
    }
}

void PackageWidget::ExpandToFirstChild()
{
    treeView->expandToDepth(0);
}
