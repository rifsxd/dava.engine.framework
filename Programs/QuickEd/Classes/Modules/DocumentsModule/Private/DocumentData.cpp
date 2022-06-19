#include "Modules/DocumentsModule/DocumentData.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

#include <Base/Hash.h>
#include <Command/Command.h>
#include <Command/CommandStack.h>

#include <QFileInfo>

DAVA_VIRTUAL_REFLECTION_IMPL(DocumentData)
{
    DAVA::ReflectionRegistrator<DocumentData>::Begin()
    .Field(packagePropertyName.c_str(), &DocumentData::GetPackageNode, nullptr)
    .Field(canSavePropertyName.c_str(), &DocumentData::CanSave, nullptr)
    .Field(canUndoPropertyName.c_str(), &DocumentData::CanUndo, nullptr)
    .Field(canRedoPropertyName.c_str(), &DocumentData::CanRedo, nullptr)
    .Field(undoTextPropertyName.c_str(), &DocumentData::GetUndoText, nullptr)
    .Field(redoTextPropertyName.c_str(), &DocumentData::GetRedoText, nullptr)
    .Field(currentNodePropertyName.c_str(), &DocumentData::GetCurrentNode, nullptr)
    .Field(selectionPropertyName.c_str(), &DocumentData::GetSelectedNodes, &DocumentData::SetSelectedNodes)
    .Field(selectionHashPropertyName.c_str(), &DocumentData::GetSelectionHash, nullptr)
    .Field(selectedControlsPropertyName.c_str(), &DocumentData::GetSelectedControls, nullptr)
    .Field(displayedRootControlsPropertyName.c_str(), &DocumentData::GetDisplayedRootControls, &DocumentData::SetDisplayedRootControls)
    .Field(guidesPropertyName.c_str(), &DocumentData::GetGuides, nullptr)
    .End();
}

DocumentData::DocumentData(const DAVA::RefPtr<PackageNode>& package_)
    : package(package_)
    , commandStack(new DAVA::CommandStack())
    , displayedRootControls(CompareByLCA)
{
    PackageControlsNode* controlsNode = package->GetPackageControlsNode();
    for (int index = 0; index < controlsNode->GetCount(); ++index)
    {
        displayedRootControls.insert(controlsNode->Get(index));
    }

    PackageControlsNode* prototypesNode = package->GetPrototypes();
    for (int index = 0; index < prototypesNode->GetCount(); ++index)
    {
        displayedRootControls.insert(prototypesNode->Get(index));
    }
}

DocumentData::~DocumentData() = default;

void DocumentData::ExecCommand(std::unique_ptr<DAVA::Command>&& command)
{
    commandStack->Exec(std::move(command));
}

void DocumentData::BeginBatch(const DAVA::String& batchName, DAVA::uint32 commandsCount)
{
    startedBatches++;
    commandStack->BeginBatch(batchName, commandsCount);
}

void DocumentData::EndBatch()
{
    DVASSERT(startedBatches != 0);
    startedBatches--;
    commandStack->EndBatch();
}

PackageNode* DocumentData::GetPackageNode() const
{
    return package.Get();
}

const SelectedNodes& DocumentData::GetSelectedNodes() const
{
    return selectedNodes;
}

size_t DocumentData::GetSelectionHash() const
{
    DAVA::Vector<size_t> hashData;
    hashData.reserve(selectedNodes.size() * 4);
    for (PackageBaseNode* node : selectedNodes)
    {
        hashData.push_back(reinterpret_cast<size_t>(node));
        PackageBaseNode* parent = node->GetParent();
        hashData.push_back(reinterpret_cast<size_t>(parent));
        hashData.push_back(parent->GetIndex(node));
        hashData.push_back(parent->GetCount());
    }
    DAVA::uint8* dataPtr = reinterpret_cast<DAVA::uint8*>(hashData.data());
    DAVA::uint32 bufferSize = static_cast<DAVA::uint32>(hashData.size()) * (sizeof(size_t) / sizeof(DAVA::uint8));
    return DAVA::BufferHash(dataPtr, bufferSize);
}

const DAVA::Set<ControlNode*>& DocumentData::GetSelectedControls() const
{
    return selectedControls;
}

const SortedControlNodeSet& DocumentData::GetDisplayedRootControls() const
{
    return displayedRootControls;
}

void DocumentData::SetSelectedNodes(const SelectedNodes& nodes)
{
    RefreshCurrentNode(nodes);

    selectedNodes = nodes;
    selectedControls.clear();
    for (PackageBaseNode* node : selectedNodes)
    {
        ControlNode* controlNode = dynamic_cast<ControlNode*>(node);
        if (controlNode != nullptr)
        {
            selectedControls.insert(controlNode);
        }
    }

    RefreshDisplayedRootControls();
}

void DocumentData::SetDisplayedRootControls(const SortedControlNodeSet& controls)
{
    displayedRootControls = controls;
}

QString DocumentData::GetName() const
{
    QFileInfo fileInfo(GetPackageAbsolutePath());
    return fileInfo.fileName();
}

QString DocumentData::GetPackageAbsolutePath() const
{
    return QString::fromStdString(package->GetPath().GetAbsolutePathname());
}

DAVA::FilePath DocumentData::GetPackagePath() const
{
    return package->GetPath();
}

bool DocumentData::CanSave() const
{
    return (documentExists == false || commandStack->IsClean() == false);
}

bool DocumentData::CanUndo() const
{
    return commandStack->CanUndo();
}

bool DocumentData::CanRedo() const
{
    return commandStack->CanRedo();
}

bool DocumentData::CanClose() const
{
    return startedBatches == 0;
}

QString DocumentData::GetRedoText() const
{
    const DAVA::Command* command = commandStack->GetRedoCommand();
    DAVA::String text = (command != nullptr ? command->GetDescription() : "");
    return QString::fromStdString(text);
}

bool DocumentData::IsDocumentExists() const
{
    return documentExists;
}

PackageBaseNode* DocumentData::GetCurrentNode() const
{
    return currentNode;
}

QString DocumentData::GetUndoText() const
{
    const DAVA::Command* command = commandStack->GetUndoCommand();
    DAVA::String text = (command != nullptr ? command->GetDescription() : "");
    return QString::fromStdString(text);
}

PackageNode::Guides DocumentData::GetGuides() const
{
    if (displayedRootControls.size() == 1)
    {
        PackageBaseNode* firstNode = *displayedRootControls.begin();
        return package->GetGuides(firstNode->GetName());
    }
    return PackageNode::Guides();
}

void DocumentData::RefreshLayout()
{
    package->RefreshPackageStylesAndLayout(true);
}

void DocumentData::RefreshAllControlProperties()
{
    package->GetPackageControlsNode()->RefreshControlProperties();
    package->GetPrototypes()->RefreshControlProperties();
}

void DocumentData::RefreshDisplayedRootControls()
{
    SortedControlNodeSet newDisplayedRootControls(CompareByLCA);
    for (PackageBaseNode* selectedNode : selectedNodes)
    {
        ControlNode* root = GetRootControlNode(dynamic_cast<ControlNode*>(selectedNode));
        if (nullptr != root)
        {
            newDisplayedRootControls.insert(root);
        }
    }
    if (newDisplayedRootControls.empty() == false)
    {
        displayedRootControls = newDisplayedRootControls;
    }
}

void DocumentData::RefreshCurrentNode(const SelectedNodes& arg)
{
    if (arg.empty())
    {
        currentNodesHistory.clear();
        currentNode = nullptr;
        return;
    }

    const SelectedNodes& currentSelection = selectedNodes;

    SortedPackageBaseNodeSet newSelection(CompareByLCA);
    std::set_difference(arg.begin(),
                        arg.end(),
                        currentSelection.begin(),
                        currentSelection.end(),
                        std::inserter(newSelection, newSelection.end()));

    SelectedNodes removedSelection;
    std::set_difference(currentSelection.begin(),
                        currentSelection.end(),
                        arg.begin(),
                        arg.end(),
                        std::inserter(removedSelection, removedSelection.end()));

    for (PackageBaseNode* node : removedSelection)
    {
        currentNodesHistory.remove(node);
    }

    if (newSelection.empty())
    {
        currentNode = currentNodesHistory.back();
    }
    else
    {
        //take any node from new selection. If this node is higher than cached selection top node, display properties for most top node from new selection
        //otherwise if this node is lower than cached selection bottom node, display properties for most bottom node from new selection
        PackageBaseNode* newSelectedNode = *newSelection.begin();
        DVASSERT(newSelectedNode != nullptr);

        bool selectionAddedToTop = true;
        if (currentSelection.empty() == false)
        {
            PackageBaseNode* currentTopNode = *currentSelection.begin();
            selectionAddedToTop = CompareByLCA(newSelectedNode, currentTopNode);
        }

        if (selectionAddedToTop)
        {
            for (auto reverseIter = newSelection.rbegin(); reverseIter != newSelection.rend(); ++reverseIter)
            {
                currentNodesHistory.push_back(*reverseIter);
            }
            currentNode = newSelectedNode;
        }
        else
        {
            for (PackageBaseNode* node : newSelection)
            {
                currentNodesHistory.push_back(node);
            }
            currentNode = *newSelection.rbegin();
        }
    }
}

DAVA::FastName DocumentData::packagePropertyName{ "package" };
DAVA::FastName DocumentData::canSavePropertyName{ "can save" };
DAVA::FastName DocumentData::canUndoPropertyName{ "can undo" };
DAVA::FastName DocumentData::canRedoPropertyName{ "can redo" };
DAVA::FastName DocumentData::undoTextPropertyName{ "undo text" };
DAVA::FastName DocumentData::redoTextPropertyName{ "redo text" };
DAVA::FastName DocumentData::currentNodePropertyName{ "current node" };
DAVA::FastName DocumentData::selectionPropertyName{ "selection" };
DAVA::FastName DocumentData::selectionHashPropertyName{ "selectionHash" };
DAVA::FastName DocumentData::selectedControlsPropertyName{ "selected controls" };
DAVA::FastName DocumentData::displayedRootControlsPropertyName{ "displayed root controls" };
DAVA::FastName DocumentData::guidesPropertyName{ "guides" };

template <>
bool DAVA::AnyCompare<PackageNode::Guides>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2)
{
    return v1.Get<PackageNode::Guides>() == v2.Get<PackageNode::Guides>();
}
