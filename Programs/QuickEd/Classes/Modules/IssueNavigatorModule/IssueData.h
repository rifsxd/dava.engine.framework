#pragma once

#include <Reflection/Reflection.h>
#include <TArc/DataProcessing/TArcDataNode.h>

class IssueData : public DAVA::TArcDataNode
{
public:
    struct Issue
    {
        bool operator==(const Issue& other) const;
        bool operator!=(const Issue& other) const;

        DAVA::int32 sectionId = -1;
        DAVA::int32 id = -1;
        DAVA::String message;
        DAVA::String packagePath;
        DAVA::String pathToControl;
        DAVA::String propertyName;

        DAVA_REFLECTION(Issue);
    };

    IssueData();
    ~IssueData() override;

    void AddIssue(DAVA::int32 sectionId, DAVA::int32 id, const DAVA::String& message, const DAVA::String& packagePath,
                  const DAVA::String& pathToControl, const DAVA::String& propertyName);
    void AddIssue(const Issue& issue);
    void ChangeMessage(DAVA::int32 sectionId, DAVA::int32 id, const DAVA::String& message);
    void ChangePathToControl(DAVA::int32 sectionId, DAVA::int32 id, const DAVA::String& pathToControl);
    void RemoveIssue(DAVA::int32 sectionId, DAVA::int32 issueId);
    void RemoveAllIssues();

    const DAVA::Vector<Issue>& GetIssues() const;

private:
    DAVA::Vector<Issue> issues;

    DAVA_VIRTUAL_REFLECTION(IssueData, DAVA::TArcDataNode);
};
