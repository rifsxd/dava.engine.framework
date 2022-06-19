#include "Classes/Modules/LibraryModule/Private/LibraryWidget.h"
#include "Classes/Modules/LibraryModule/Private/LibraryModel.h"

#include <Base/Any.h>
#include <Reflection/ReflectedTypeDB.h>
#include <UI/UIControl.h>

#include <QTreeView>
#include <QVBoxLayout>
#include <QHeaderView>

LibraryWidget::LibraryWidget(DAVA::ContextAccessor* accessor, DAVA::UI* ui, QWidget* parent)
    : QWidget(parent)
    , libraryModel(new LibraryModel(ui, accessor, this))
    , accessor(accessor)
{
    InitUI();
    treeView->setModel(libraryModel);
}

LibraryWidget::~LibraryWidget() = default;

void LibraryWidget::InitUI()
{
    QVBoxLayout* verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(5);
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    treeView = new QTreeView(this);
    treeView->setDragEnabled(true);
    treeView->setDragDropMode(QAbstractItemView::DragOnly);
    treeView->setDefaultDropAction(Qt::CopyAction);
    treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
    treeView->header()->setVisible(false);

    verticalLayout->addWidget(treeView);
}

void LibraryWidget::SetLibraryPackages(const DAVA::Vector<DAVA::RefPtr<PackageNode>>& projectLibraries)
{
    libraryModel->SetLibraryPackages(projectLibraries);
}

void LibraryWidget::SetCurrentPackage(PackageNode* package)
{
    libraryModel->SetPackageNode(package);
    treeView->setEnabled(package != nullptr);
    treeView->expandAll();
    treeView->collapse(libraryModel->GetDefaultControlsModelIndex());
}

QTreeView* LibraryWidget::GetTreeView()
{
    return treeView;
}
