#pragma once

#include "Classes/Model/PackageHierarchy/PackageNode.h"

#include <Base/BaseTypes.h>

#include <QWidget>

class LibraryModel;
class QTreeView;

namespace DAVA
{
class ContextAccessor;
class UI;
}

class LibraryWidget : public QWidget
{
    Q_OBJECT

public:
    LibraryWidget(DAVA::ContextAccessor* accessor, DAVA::UI* ui, QWidget* parent = nullptr);
    ~LibraryWidget() override;

    void SetLibraryPackages(const DAVA::Vector<DAVA::RefPtr<PackageNode>>& projectLibrary);
    void SetCurrentPackage(PackageNode* package);

    QTreeView* GetTreeView();

private:
    void InitUI();

private:
    QTreeView* treeView;
    LibraryModel* libraryModel = nullptr;
    DAVA::ContextAccessor* accessor = nullptr;
};
