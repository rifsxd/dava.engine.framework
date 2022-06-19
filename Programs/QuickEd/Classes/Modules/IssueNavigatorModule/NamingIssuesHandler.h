#pragma once

#include "Classes/Modules/IssueNavigatorModule/IssueHandler.h"
#include "Classes/Modules/IssueNavigatorModule/IssueData.h"
#include "Classes/Utils/PackageListenerProxy.h"

#include <Base/BaseTypes.h>

namespace DAVA
{
class ContextAccessor;
class UI;
class UIControl;
}

class ControlNode;
class IndexGenerator;

class NamingIssuesHandler : public IssueHandler, PackageListener
{
public:
    NamingIssuesHandler(DAVA::ContextAccessor* accessor, DAVA::int32 sectionId, IndexGenerator* indexGenerator);
    ~NamingIssuesHandler() override = default;

    // IssuesHandler
    void OnContextDeleted(DAVA::DataContext* current) override;

    // PackageListener
    void ActivePackageNodeWasChanged(PackageNode* node) override;
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index) override;
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;

private:
    struct DuplicationsIssue
    {
        IssueData::Issue issue;
        DAVA::UnorderedSet<ControlNode*> controls;
    };
    using DuplicationsIssuesMap = DAVA::UnorderedMap<DAVA::FastName, DuplicationsIssue>;

    struct PackageIssues
    {
        DAVA::DataContext* context = nullptr;
        DAVA::UnorderedMap<ControlNode*, IssueData::Issue> symbolsIssues;
        DuplicationsIssuesMap duplicationIssues;
    };

    void ValidateNameSymbolsCorrectnessForChildren(ControlsContainerNode* node);
    void ValidateNameSymbolsCorrectness(ControlNode* node);
    void ValidateNameUniqueness(ControlNode* node);

    void CreateSymbolsIssue(ControlNode* node);
    void CreateDuplicationsIssue(ControlNode* node);
    void AddToDuplicationsIssue(DuplicationsIssue& issue, ControlNode* node);
    void UpdateSymbolsIssue(std::pair<ControlNode* const, IssueData::Issue>& symbolsIssue);
    void UpdateDuplicationsIssue(DuplicationsIssue& issue);

    void RemoveSymbolsIssuesRecursively(ControlNode* node);
    void RemoveSymbolsIssue(ControlNode* node);
    void RemoveFromDuplicationsIssue(ControlNode* node);

    void RemoveIssuesFromPanel();
    void RestoreIssuesOnToPanel();

    void SearchIssuesInPackage(PackageNode* package);

    PackageNode* GetPackage() const;
    DAVA::UnorderedSet<ControlNode*> GetControlsByName(const DAVA::String& name);

    DuplicationsIssuesMap::iterator FindInDuplicationsIssues(ControlNode* node);

private:
    IndexGenerator* indexGenerator = nullptr;

    DAVA::UnorderedMap<PackageNode*, PackageIssues> packageIssues;
    PackageNode* currentPackage = nullptr;
    DAVA::UnorderedMap<ControlNode*, IssueData::Issue>* symbolsIssues = nullptr;
    DuplicationsIssuesMap* duplicationIssues = nullptr;

    PackageListenerProxy packageListenerProxy;
};
