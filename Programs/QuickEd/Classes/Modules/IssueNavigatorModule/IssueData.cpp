#include "Modules/IssueNavigatorModule/IssueData.h"

DAVA_VIRTUAL_REFLECTION_IMPL(IssueData)
{
    DAVA::ReflectionRegistrator<IssueData>::Begin()
    .ConstructorByPointer()
    .Field("issues", &IssueData::issues)
    .End();
}

DAVA_REFLECTION_IMPL(IssueData::Issue)
{
    DAVA::ReflectionRegistrator<Issue>::Begin()
    .Field("sectionId", &Issue::sectionId)
    .Field("issueId", &Issue::id)
    .Field("message", &Issue::message)
    .Field("packagePath", &Issue::packagePath)
    .Field("pathToControl", &Issue::pathToControl)
    .Field("propertyName", &Issue::propertyName)
    .End();
}

IssueData::IssueData()
{
}

IssueData::~IssueData()
{
}

void IssueData::AddIssue(DAVA::int32 sectionId, DAVA::int32 id, const DAVA::String& message, const DAVA::String& packagePath,
                         const DAVA::String& pathToControl, const DAVA::String& propertyName)
{
    Issue issue;
    issue.sectionId = sectionId;
    issue.id = id;
    issue.message = message;
    issue.packagePath = packagePath;
    issue.pathToControl = pathToControl;
    issue.propertyName = propertyName;

    issues.push_back(issue);
}

void IssueData::AddIssue(const Issue& issue)
{
    issues.push_back(issue);
}

void IssueData::ChangeMessage(DAVA::int32 sectionId, DAVA::int32 id, const DAVA::String& message)
{
    for (Issue& issue : issues)
    {
        if (issue.sectionId == sectionId && issue.id == id)
        {
            issue.message = message;
            break;
        }
    }
}

void IssueData::ChangePathToControl(DAVA::int32 sectionId, DAVA::int32 id, const DAVA::String& pathToControl)
{
    for (Issue& issue : issues)
    {
        if (issue.sectionId == sectionId && issue.id == id)
        {
            issue.pathToControl = pathToControl;
            break;
        }
    }
}

void IssueData::RemoveIssue(DAVA::int32 sectionId, DAVA::int32 issueId)
{
    auto it = std::find_if(issues.begin(), issues.end(),
                           [sectionId, issueId](const Issue& i) {
                               return i.sectionId == sectionId && i.id == issueId;
                           });

    if (it != issues.end())
    {
        issues.erase(it);
    }
}

void IssueData::RemoveAllIssues()
{
    issues.clear();
}

const DAVA::Vector<IssueData::Issue>& IssueData::GetIssues() const
{
    return issues;
}

bool IssueData::Issue::operator==(const Issue& other) const
{
    return sectionId == other.sectionId && id == other.id && message == other.message && pathToControl == other.pathToControl;
}

bool IssueData::Issue::operator!=(const Issue& other) const
{
    return !operator==(other);
}
