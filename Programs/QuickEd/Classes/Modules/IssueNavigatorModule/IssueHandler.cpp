#include "Modules/IssueNavigatorModule/IssueHandler.h"

#include "Modules/IssueNavigatorModule/IssueData.h"

#include "Modules/DocumentsModule/DocumentData.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

#include <TArc/Core/ContextAccessor.h>

#include <UI/UIControl.h>

IssueHandler::IssueHandler(DAVA::ContextAccessor* accessor_, DAVA::int32 sectionId_)
    : sectionId(sectionId_)
    , accessor(accessor_)
{
}

IssueHandler::~IssueHandler()
{
}

IssueData* IssueHandler::GetIssueData()
{
    if (accessor->GetActiveContext() == nullptr)
    {
        return nullptr;
    }
    return accessor->GetActiveContext()->GetData<IssueData>();
}

bool IssueHandler::IsControlOutOfScope(const DAVA::UIControl* control) const
{
    const DocumentData* data = accessor->GetActiveContext()->GetData<DocumentData>();
    DVASSERT(data != nullptr);

    if (control == nullptr)
    {
        return true;
    }

    PackageControlsNode* controls = data->GetPackageNode()->GetPackageControlsNode();
    for (ControlNode* controlNode : *controls)
    {
        if (controlNode->GetControl()->GetParent() == control)
        {
            return true;
        }
    }

    PackageControlsNode* prototypes = data->GetPackageNode()->GetPrototypes();
    for (ControlNode* controlNode : *prototypes)
    {
        if (controlNode->GetControl()->GetParent() == control)
        {
            return true;
        }
    }
    return false;
}

void IssueHandler::AddIssue(DAVA::int32 id, const DAVA::String& message,
                            const DAVA::String& packagePath, const DAVA::String& controlPath,
                            const DAVA::String& propertyName)
{
    IssueData* issueData = GetIssueData();
    if (issueData)
    {
        issueData->AddIssue(sectionId, id, message, packagePath, controlPath, propertyName);
    }
}

void IssueHandler::AddIssue(const IssueData::Issue& issue)
{
    IssueData* issueData = GetIssueData();
    if (issueData)
    {
        if (issue.sectionId == sectionId)
        {
            issueData->AddIssue(issue);
        }
        else
        {
            DVASSERT(false);
        }
    }
}

void IssueHandler::ChangeMessage(DAVA::int32 id, const DAVA::String& message)
{
    IssueData* issueData = GetIssueData();
    if (issueData)
    {
        issueData->ChangeMessage(sectionId, id, message);
    }
}

void IssueHandler::ChangePathToControl(DAVA::int32 id, const DAVA::String& pathToControl)
{
    IssueData* issueData = GetIssueData();
    if (issueData)
    {
        issueData->ChangePathToControl(sectionId, id, pathToControl);
    }
}

void IssueHandler::RemoveIssue(DAVA::int32 id)
{
    IssueData* issueData = GetIssueData();
    if (issueData)
    {
        issueData->RemoveIssue(sectionId, id);
    }
}
