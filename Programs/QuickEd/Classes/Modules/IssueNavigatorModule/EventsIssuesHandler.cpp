#include "Classes/Modules/IssueNavigatorModule/EventsIssuesHandler.h"

#include "Classes/Model/ControlProperties/AbstractProperty.h"
#include "Classes/Model/ControlProperties/ComponentPropertiesSection.h"
#include "Classes/Model/ControlProperties/IntrospectionProperty.h"
#include "Classes/Model/ControlProperties/RootProperty.h"
#include "Classes/Model/PackageHierarchy/PackageControlsNode.h"
#include "Classes/Model/PackageHierarchy/PackageNode.h"
#include "Classes/Modules/DocumentsModule/DocumentData.h"
#include "Classes/Modules/IssueNavigatorModule/IssueHelper.h"

#include <Base/Type.h>
#include <Functional/Function.h>
#include <UI/Events/UIInputEventComponent.h>
#include <UI/Events/UIMovieEventComponent.h>
#include <UI/UIControl.h>
#include <UI/UIControlHelpers.h>
#include <UI/UIControlSystem.h>

#include <TArc/Core/ContextAccessor.h>

EventsIssuesHandler::EventsIssuesHandler(DAVA::ContextAccessor* accessor_, DAVA::int32 sectionId_, IndexGenerator* indexGenerator_)
    : IssueHandler(accessor_, sectionId_)
    , indexGenerator(indexGenerator_)
    , packageListenerProxy(this, accessor_)
{
    using namespace DAVA;
    componentsAndProperties[Type::Instance<UIInputEventComponent>()] = {
        FastName("onTouchDown"),
        FastName("onTouchUpInside"),
        FastName("onTouchUpOutside"),
        FastName("onValueChanged"),
        FastName("onHoverSet"),
        FastName("onHoverRemoved")
    };
    componentsAndProperties[Type::Instance<UIMovieEventComponent>()] = {
        FastName("onStart"),
        FastName("onStop")
    };
}

DAVA::String EventsIssuesHandler::CreateIncorrectSymbolsMessage(EventIssue& eventIssue)
{
    return DAVA::Format("Node '%s' event property '%s' contains incorrect symbols", eventIssue.node->GetName().c_str(), eventIssue.propertyName.c_str());
}

PackageNode* EventsIssuesHandler::GetPackage() const
{
    return accessor->GetActiveContext()->GetData<DocumentData>()->GetPackageNode();
}

bool EventsIssuesHandler::IsRootControl(const ControlNode* node) const
{
    PackageNode* package = GetPackage();
    return ((node->GetParent() == package->GetPackageControlsNode()) || (node->GetParent() == package->GetPrototypes()));
}

DAVA::String EventsIssuesHandler::GetPathToControl(const ControlNode* node) const
{
    auto GetParentNode = [&](const ControlNode* node) -> ControlNode*
    {
        return IsRootControl(node) ? nullptr : dynamic_cast<ControlNode*>(node->GetParent());
    };

    DAVA::String pathToControl = node->GetName();

    for (const ControlNode* nextNode = GetParentNode(node);
         nextNode != nullptr;
         nextNode = GetParentNode(nextNode))
    {
        pathToControl = nextNode->GetName() + "/" + pathToControl;
    }

    return pathToControl;
}

void EventsIssuesHandler::CreateIssue(ControlNode* node, const DAVA::Type* componentType, const DAVA::String& propertyName)
{
    DAVA::int32 issueId = indexGenerator->NextIssueId();

    EventIssue eventIssue;
    eventIssue.node = node;
    eventIssue.componentType = componentType;
    eventIssue.propertyName = DAVA::FastName(propertyName.c_str());
    eventIssue.issueId = issueId;
    eventIssue.wasFixed = false;
    issues.push_back(eventIssue);

    IssueData::Issue issue;
    issue.sectionId = sectionId;
    issue.id = issueId;
    issue.message = CreateIncorrectSymbolsMessage(eventIssue);
    issue.packagePath = GetPackage()->GetPath().GetFrameworkPath();
    issue.pathToControl = GetPathToControl(node);
    issue.propertyName = propertyName;

    node->AddIssue(issue.id);
    AddIssue(issue);
}

void EventsIssuesHandler::UpdateNodeIssue(EventIssue& eventIssue)
{
    ControlNode* node = eventIssue.node;
    DAVA::int32 issueId = eventIssue.issueId;
    ChangeMessage(issueId, CreateIncorrectSymbolsMessage(eventIssue));
    ChangePathToControl(issueId, GetPathToControl(node));
}

void EventsIssuesHandler::RemoveIssuesIf(MatchFunction matchPred)
{
    auto RemoveIssueFn = [&](EventIssue& issue)
    {
        if (matchPred(issue))
        {
            DAVA::int32 issueId = issue.issueId;
            RemoveIssue(issueId);
            issue.node->RemoveIssue(issueId);
            return true;
        }
        return false;
    };

    auto it = std::remove_if(issues.begin(), issues.end(), RemoveIssueFn);
    issues.erase(it, issues.end());
}

void EventsIssuesHandler::RemoveNodeIssues(ControlNode* node, bool recursive)
{
    RemoveIssuesIf([&](const EventIssue& issue) { return issue.node == node; });

    if (recursive)
    {
        for (ControlNode* child : *node)
        {
            RemoveNodeIssues(child, recursive);
        }
    }
}

void EventsIssuesHandler::RemoveComponentIssues(ControlNode* node, const DAVA::Type* componentType)
{
    RemoveIssuesIf([&](const EventIssue& issue) { return issue.node == node && issue.componentType == componentType; });
}

void EventsIssuesHandler::RemoveAllIssues()
{
    RemoveIssuesIf([&](const EventIssue& issue) { return true; });
}

void EventsIssuesHandler::SearchIssuesInPackage(PackageNode* package)
{
    if (package)
    {
        PackageControlsNode* packageControlsNode = package->GetPackageControlsNode();
        PackageControlsNode* packagePrototypesNode = package->GetPrototypes();

        ValidateNodeForChildren(packageControlsNode);
        ValidateNodeForChildren(packagePrototypesNode);
    }
}

//////////////////////////////////////////////////////////////////////////

void EventsIssuesHandler::ActivePackageNodeWasChanged(PackageNode* package)
{
    RemoveAllIssues();
    SearchIssuesInPackage(package);
}

void EventsIssuesHandler::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    ComponentPropertiesSection* componentSection = dynamic_cast<ComponentPropertiesSection*>(property->GetParent());
    if (componentSection)
    {
        auto it = componentsAndProperties.find(componentSection->GetComponentType());
        if (it != componentsAndProperties.end())
        {
            if (it->second.find(DAVA::FastName(property->GetName())) != it->second.end())
            {
                ValidateProperty(node, componentSection->GetComponentType(), property, true);
            }
        }
    }

    if (property->GetName() == "Name")
    {
        // Refresh all children issues
        for (EventIssue& issue : issues)
        {
            if (node->IsParentOf(issue.node))
            {
                UpdateNodeIssue(issue);
            }
        }
    }
}

void EventsIssuesHandler::ControlComponentWasAdded(ControlNode* node, ComponentPropertiesSection* section)
{
    DVASSERT(section != nullptr);
    if (componentsAndProperties.find(section->GetComponentType()) != componentsAndProperties.end())
    {
        ValidateSection(node, section, true);
    }
}

void EventsIssuesHandler::ControlComponentWasRemoved(ControlNode* node, ComponentPropertiesSection* section)
{
    DVASSERT(section != nullptr);
    if (componentsAndProperties.find(section->GetComponentType()) != componentsAndProperties.end())
    {
        RemoveComponentIssues(node, section->GetComponentType());
    }
}

void EventsIssuesHandler::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index)
{
    ValidateNode(node);
    ValidateNodeForChildren(node);
}

void EventsIssuesHandler::ControlWasRemoved(ControlNode* node, ControlsContainerNode* from)
{
    RemoveNodeIssues(node, true);
}

//////////////////////////////////////////////////////////////////////////

void EventsIssuesHandler::ValidateNodeForChildren(ControlsContainerNode* container)
{
    for (ControlNode* node : *container)
    {
        ValidateNode(node);
        ValidateNodeForChildren(node);
    }
}

void EventsIssuesHandler::ValidateNode(ControlNode* node)
{
    using namespace DAVA;

    for (EventIssue& issue : issues)
    {
        issue.wasFixed = (issue.node == node);
    }

    for (ComponentPropertiesSection* componentSection : node->GetRootProperty()->GetComponents())
    {
        ValidateSection(node, componentSection, false);
    }

    RemoveIssuesIf([&](const EventIssue& issue) { return issue.wasFixed; });
}

void EventsIssuesHandler::ValidateSection(ControlNode* node, ComponentPropertiesSection* componentSection, bool removeFixedIssues)
{
    using namespace DAVA;

    // Is event component
    const Type* componentType = componentSection->GetComponentType();
    auto it = componentsAndProperties.find(componentType);
    if (it != componentsAndProperties.end())
    {
        Set<FastName>& componentProperties = it->second;
        // For each component from section properties
        for (auto& property : *componentSection)
        {
            // Is event property?
            FastName propertyName(property->GetName());
            if (componentProperties.find(propertyName) != componentProperties.end())
            {
                ValidateProperty(node, componentType, property, removeFixedIssues);
            }
        }
    }
}

void EventsIssuesHandler::ValidateProperty(ControlNode* node, const DAVA::Type* componentType, AbstractProperty* property, bool removeFixedIssues)
{
    using namespace DAVA;

    Any propertyValue = property->GetValue();
    FastName propertyName(property->GetName());
    FastName event = propertyValue.CanCast<FastName>() ? propertyValue.Cast<FastName>() : FastName();
    // Check event format
    if (UIControlHelpers::IsEventNameValid(event) == false)
    {
        auto IssueMatcher = [&](const EventIssue& issue)
        {
            return issue.node == node && issue.componentType == componentType && issue.propertyName == propertyName;
        };
        auto issuesIt = std::find_if(issues.begin(), issues.end(), IssueMatcher);

        if (issuesIt == issues.end())
        {
            CreateIssue(node, componentType, property->GetName());
        }
        else
        {
            UpdateNodeIssue(*issuesIt);
            // Mark unfixed issue
            issuesIt->wasFixed = false;
        }
    }
    else
    {
        if (removeFixedIssues)
        {
            RemoveIssuesIf([&](const EventIssue& issue) { return issue.node == node && issue.componentType == componentType && issue.propertyName == propertyName; });
        }
    }
}
