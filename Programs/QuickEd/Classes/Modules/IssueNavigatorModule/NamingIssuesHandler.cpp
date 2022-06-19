#include "Classes/Modules/IssueNavigatorModule/NamingIssuesHandler.h"

#include "Classes/Model/ControlProperties/AbstractProperty.h"
#include "Classes/Model/ControlProperties/NameProperty.h"
#include "Classes/Model/ControlProperties/RootProperty.h"
#include "Classes/Model/PackageHierarchy/PackageControlsNode.h"
#include "Classes/Model/PackageHierarchy/PackageNode.h"
#include "Classes/Modules/DocumentsModule/DocumentData.h"
#include "Classes/Modules/IssueNavigatorModule/ControlNodeInfo.h"
#include "Classes/Modules/IssueNavigatorModule/IssueHelper.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/WindowSubSystem/UI.h>

#include <UI/UIControl.h>
#include <UI/UIControlHelpers.h>
#include <UI/UIControlSystem.h>

namespace NamingIssuesHandlerDetails
{
DAVA::String CreateDuplicationsMessage(const DAVA::String& name)
{
    return DAVA::Format("name '%s' is not unique", name.c_str());
}
DAVA::String CreateIncorrectSymbolsMessage(const DAVA::String& name)
{
    return DAVA::Format("name '%s' contains incorrect symbols", name.c_str());
}

void UpdateNameProperty(ControlNode* node)
{
    PackageNode* package = node->GetPackage();
    if (package != nullptr)
    {
        NameProperty* nameProperty = node->GetRootProperty()->GetNameProperty();
        node->GetPackage()->SetControlProperty(node, nameProperty, nameProperty->GetValue());
    }
}
}

NamingIssuesHandler::NamingIssuesHandler(DAVA::ContextAccessor* accessor_, DAVA::int32 sectionId_, IndexGenerator* indexGenerator_)
    : IssueHandler(accessor_, sectionId_)
    , packageListenerProxy(this, accessor_)
    , indexGenerator(indexGenerator_)
{
}

void NamingIssuesHandler::OnContextDeleted(DAVA::DataContext* context)
{
    auto found = std::find_if(packageIssues.begin(), packageIssues.end(), [context](const std::pair<PackageNode*, PackageIssues>& entry)
                              {
                                  return entry.second.context == context;
                              });

    if (found != packageIssues.end())
    {
        packageIssues.erase(found);
    }
}

void NamingIssuesHandler::ActivePackageNodeWasChanged(PackageNode* package)
{
    if (currentPackage != nullptr)
    {
        RemoveIssuesFromPanel();
    }

    currentPackage = package;
    DAVA::DataContext* currentContext = accessor->GetActiveContext();

    auto found = std::find_if(packageIssues.begin(), packageIssues.end(), [currentContext](const std::pair<PackageNode*, PackageIssues>& entry)
                              {
                                  return entry.second.context == currentContext;
                              });
    if (found != packageIssues.end() && found->first != currentPackage)
    {
        packageIssues.erase(found);
    }

    if (currentPackage != nullptr)
    {
        auto resultOfEmplace = packageIssues.emplace(package, PackageIssues());
        bool isNewPackage = resultOfEmplace.second;
        PackageIssues& issues = resultOfEmplace.first->second;
        issues.context = currentContext;
        symbolsIssues = &(issues.symbolsIssues);
        duplicationIssues = &(issues.duplicationIssues);

        if (isNewPackage)
        {
            SearchIssuesInPackage(currentPackage);
        }
        else
        {
            RestoreIssuesOnToPanel();
        }
    }
    else
    {
        symbolsIssues = nullptr;
        duplicationIssues = nullptr;
    }
}

void NamingIssuesHandler::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    if (property->GetName() == "Name")
    {
        ValidateNameSymbolsCorrectness(node);
        ValidateNameUniqueness(node);

        for (std::pair<ControlNode* const, IssueData::Issue>& symbolsIssue : *symbolsIssues)
        {
            if (node->IsParentOf(symbolsIssue.first))
            {
                UpdateSymbolsIssue(symbolsIssue);
            }
        }

        for (auto& issuePair : *duplicationIssues)
        {
            DuplicationsIssue& issue = issuePair.second;
            if (issue.controls.empty() == false)
            {
                ControlNode* firstControl = *(issue.controls.begin());
                if (node->IsParentOf(firstControl))
                {
                    UpdateDuplicationsIssue(issue);
                }
            }
        };
    }
}

void NamingIssuesHandler::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index)
{
    ValidateNameUniqueness(node);
    ValidateNameSymbolsCorrectness(node);
    ValidateNameSymbolsCorrectnessForChildren(node);
}

void NamingIssuesHandler::ControlWasRemoved(ControlNode* node, ControlsContainerNode* from)
{
    RemoveSymbolsIssuesRecursively(node);
    RemoveFromDuplicationsIssue(node);
}

void NamingIssuesHandler::ValidateNameSymbolsCorrectness(ControlNode* node)
{
    using namespace DAVA;

    FastName name = FastName(node->GetName());
    if (UIControlHelpers::IsControlNameValid(name, UIControlHelpers::StrictCheck) == false)
    {
        auto it = symbolsIssues->find(node);
        if (it == symbolsIssues->end())
        {
            CreateSymbolsIssue(node);
        }
        else
        {
            UpdateSymbolsIssue(*it);
        }
    }
    else
    {
        RemoveSymbolsIssue(node);
    }
}

void NamingIssuesHandler::ValidateNameUniqueness(ControlNode* node)
{
    using namespace DAVA;

    if (ControlNodeInfo::IsRootControl(node))
    {
        String nameString = node->GetName();
        FastName name = FastName(nameString);

        DuplicationsIssuesMap::iterator it = FindInDuplicationsIssues(node);
        if (it != duplicationIssues->end())
        {
            const FastName& nameFromIssue = it->first;
            if (nameFromIssue != name)
            {
                RemoveFromDuplicationsIssue(node);
            }
            else
            {
                return;
            }
        }

        UnorderedSet<ControlNode*> controlsWithSameNames = GetControlsByName(nameString);
        if (controlsWithSameNames.size() > 1)
        {
            if (duplicationIssues->find(name) == duplicationIssues->end())
            {
                CreateDuplicationsIssue(node);
            }

            DuplicationsIssue& issue = (*duplicationIssues)[name];
            for (ControlNode* sameNameNode : controlsWithSameNames)
            {
                if (issue.controls.find(sameNameNode) == issue.controls.end())
                {
                    AddToDuplicationsIssue(issue, sameNameNode);
                }
            }
        }
    }
}

void NamingIssuesHandler::CreateSymbolsIssue(ControlNode* node)
{
    using namespace DAVA;

    IssueData::Issue& issue = (*symbolsIssues)[node];

    issue.sectionId = sectionId;
    issue.id = indexGenerator->NextIssueId();
    issue.message = NamingIssuesHandlerDetails::CreateIncorrectSymbolsMessage(node->GetName());
    issue.packagePath = GetPackage()->GetPath().GetFrameworkPath();
    issue.pathToControl = ControlNodeInfo::GetPathToControl(node);
    issue.propertyName = "Name";

    node->AddIssue(issue.id);
    AddIssue(issue);

    NotificationParams notificationParams;
    notificationParams.title = "Incorrect symbols in control name";
    notificationParams.message = Result(Result::RESULT_ERROR, issue.message);
}

void NamingIssuesHandler::CreateDuplicationsIssue(ControlNode* node)
{
    using namespace DAVA;

    String nameString = node->GetName();
    FastName name = FastName(nameString);
    DVASSERT(duplicationIssues->find(name) == duplicationIssues->end());

    DuplicationsIssue& duplicationsIssue = (*duplicationIssues)[name];

    IssueData::Issue& issue = duplicationsIssue.issue;
    issue.sectionId = sectionId;
    issue.id = indexGenerator->NextIssueId();
    issue.message = NamingIssuesHandlerDetails::CreateDuplicationsMessage(nameString);
    issue.packagePath = GetPackage()->GetPath().GetFrameworkPath();
    issue.pathToControl = ControlNodeInfo::GetPathToControl(node);
    issue.propertyName = "Name";

    AddIssue(issue);

    DAVA::NotificationParams notificationParams;
    notificationParams.title = "Duplicated name";
    notificationParams.message = Result(Result::RESULT_ERROR, issue.message);
}

void NamingIssuesHandler::AddToDuplicationsIssue(DuplicationsIssue& duplicationIssue, ControlNode* node)
{
    duplicationIssue.controls.insert(node);
    node->AddIssue(duplicationIssue.issue.id);

    // hint: updating name property will make package widget to refresh visual look of node item
    NamingIssuesHandlerDetails::UpdateNameProperty(node);
}

void NamingIssuesHandler::UpdateSymbolsIssue(std::pair<ControlNode* const, IssueData::Issue>& symbolsIssue)
{
    using namespace DAVA;

    ControlNode* node = symbolsIssue.first;
    IssueData::Issue& issue = symbolsIssue.second;
    issue.message = NamingIssuesHandlerDetails::CreateIncorrectSymbolsMessage(node->GetName());
    issue.pathToControl = ControlNodeInfo::GetPathToControl(node);
    ChangeMessage(issue.id, issue.message);
    ChangePathToControl(issue.id, issue.pathToControl);

    NotificationParams notificationParams;
    notificationParams.title = "Incorrect symbols in control name";
    notificationParams.message = Result(Result::RESULT_ERROR, issue.message);
}

void NamingIssuesHandler::UpdateDuplicationsIssue(DuplicationsIssue& duplicationIssue)
{
    using namespace DAVA;

    if (duplicationIssue.controls.empty() == false)
    {
        ControlNode* node = *(duplicationIssue.controls.begin());
        IssueData::Issue& issue = duplicationIssue.issue;
        issue.message = NamingIssuesHandlerDetails::CreateDuplicationsMessage(node->GetName());
        issue.pathToControl = ControlNodeInfo::GetPathToControl(node);
        ChangeMessage(issue.id, issue.message);
        ChangePathToControl(issue.id, issue.pathToControl);

        DAVA::NotificationParams notificationParams;
        notificationParams.title = "Duplicated name";
        notificationParams.message = Result(Result::RESULT_ERROR, issue.message);
    }
}

void NamingIssuesHandler::ValidateNameSymbolsCorrectnessForChildren(ControlsContainerNode* container)
{
    for (ControlNode* node : *container)
    {
        ValidateNameSymbolsCorrectness(node);
        ValidateNameSymbolsCorrectnessForChildren(node);
    }
}

void NamingIssuesHandler::RemoveSymbolsIssuesRecursively(ControlNode* node)
{
    RemoveSymbolsIssue(node);
    for (ControlNode* child : *node)
    {
        RemoveSymbolsIssuesRecursively(child);
    }
}

void NamingIssuesHandler::RemoveSymbolsIssue(ControlNode* node)
{
    auto it = symbolsIssues->find(node);
    if (it != symbolsIssues->end())
    {
        DAVA::int32 issueId = it->second.id;
        RemoveIssue(issueId);
        node->RemoveIssue(issueId);
        symbolsIssues->erase(it);
    }
}

void NamingIssuesHandler::RemoveFromDuplicationsIssue(ControlNode* control)
{
    DuplicationsIssuesMap::iterator it = FindInDuplicationsIssues(control);
    if (it != duplicationIssues->end())
    {
        DuplicationsIssue& duplicationsIssue = it->second;

        auto RemoveIssueFromControl = [&duplicationsIssue](ControlNode* control)
        {
            control->RemoveIssue(duplicationsIssue.issue.id);
            duplicationsIssue.controls.erase(control);
            // hint: updating name property will make package widget to refresh visual look of node item
            NamingIssuesHandlerDetails::UpdateNameProperty(control);
        };

        RemoveIssueFromControl(control);

        if (duplicationsIssue.controls.size() == 1)
        {
            ControlNode* lastControl = *(duplicationsIssue.controls.begin());
            RemoveIssueFromControl(lastControl);

            RemoveIssue(duplicationsIssue.issue.id);
            duplicationIssues->erase(it);
        }
    }
}

void NamingIssuesHandler::RemoveIssuesFromPanel()
{
    std::for_each(symbolsIssues->begin(), symbolsIssues->end(), [this](const std::pair<ControlNode*, IssueData::Issue>& issuePair)
                  {
                      RemoveIssue(issuePair.second.id);
                  });

    std::for_each(duplicationIssues->begin(), duplicationIssues->end(), [this](const std::pair<DAVA::FastName, DuplicationsIssue>& issuePair)
                  {
                      RemoveIssue(issuePair.second.issue.id);
                  });
}

void NamingIssuesHandler::RestoreIssuesOnToPanel()
{
    std::for_each(symbolsIssues->begin(), symbolsIssues->end(), [this](const std::pair<ControlNode*, IssueData::Issue>& issuePair)
                  {
                      AddIssue(issuePair.second);
                  });

    std::for_each(duplicationIssues->begin(), duplicationIssues->end(), [this](const std::pair<DAVA::FastName, DuplicationsIssue>& issuePair)
                  {
                      AddIssue(issuePair.second.issue);
                  });
}

void NamingIssuesHandler::SearchIssuesInPackage(PackageNode* package)
{
    PackageControlsNode* packageControlsNode = package->GetPackageControlsNode();
    PackageControlsNode* packagePrototypesNode = package->GetPrototypes();

    auto ValidateTopControlsUniqueness = [&](ControlsContainerNode* container)
    {
        if (container != nullptr)
        {
            for (ControlNode* node : *packageControlsNode)
            {
                ValidateNameUniqueness(node);
            }
        }
    };

    ValidateTopControlsUniqueness(packageControlsNode);
    ValidateTopControlsUniqueness(packagePrototypesNode);

    ValidateNameSymbolsCorrectnessForChildren(packageControlsNode);
    ValidateNameSymbolsCorrectnessForChildren(packagePrototypesNode);
}

PackageNode* NamingIssuesHandler::GetPackage() const
{
    return accessor->GetActiveContext()->GetData<DocumentData>()->GetPackageNode();
}

DAVA::UnorderedSet<ControlNode*> NamingIssuesHandler::GetControlsByName(const DAVA::String& name)
{
    DAVA::UnorderedSet<ControlNode*> result;

    auto getControlsByNameFromContainer = [&name, &result](ControlsContainerNode* container)
    {
        if (container != nullptr)
        {
            for (ControlNode* node : *container)
            {
                if (node->GetName() == name)
                {
                    result.insert(node);
                }
            }
        }
    };

    PackageNode* package = GetPackage();
    PackageControlsNode* packageControlsNode = package->GetPackageControlsNode();
    PackageControlsNode* packagePrototypesNode = package->GetPrototypes();

    getControlsByNameFromContainer(packageControlsNode);
    getControlsByNameFromContainer(packagePrototypesNode);

    return result;
}

NamingIssuesHandler::DuplicationsIssuesMap::iterator NamingIssuesHandler::FindInDuplicationsIssues(ControlNode* node)
{
    DuplicationsIssuesMap::iterator it = std::find_if(duplicationIssues->begin(), duplicationIssues->end(), [node](const std::pair<DAVA::FastName, DuplicationsIssue>& entry)
                                                      {
                                                          const DuplicationsIssue& issue = entry.second;
                                                          return (issue.controls.find(node) != issue.controls.end());
                                                      });

    return it;
}
