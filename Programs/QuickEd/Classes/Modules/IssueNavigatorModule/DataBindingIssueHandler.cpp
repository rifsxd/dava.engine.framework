#include "DataBindingIssueHandler.h"

#include "Modules/IssueNavigatorModule/IssueData.h"
#include "Modules/IssueNavigatorModule/IssueHelper.h"

#include "Modules/DocumentsModule/DocumentData.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

#include <Engine/Engine.h>
#include <UI/UIControlSystem.h>
#include <UI/UIControl.h>
#include <UI/UIControlHelpers.h>
#include <UI/DataBinding/UIDataBindingSystem.h>

#include <TArc/Core/ContextAccessor.h>

DataBindingIssueHandler::DataBindingIssueHandler(DAVA::ContextAccessor* accessor_, DAVA::int32 sectionId_, IndexGenerator* indexGenerator_)
    : IssueHandler(accessor_, sectionId_)
    , indexGenerator(indexGenerator_)
{
    DAVA::GetEngineContext()->uiControlSystem->GetSystem<DAVA::UIDataBindingSystem>()->SetIssueDelegate(this);
}

DataBindingIssueHandler::~DataBindingIssueHandler()
{
    DAVA::GetEngineContext()->uiControlSystem->GetSystem<DAVA::UIDataBindingSystem>()->SetIssueDelegate(nullptr);
}

DAVA::int32 DataBindingIssueHandler::GenerateNewId()
{
    return indexGenerator->NextIssueId();
}

void DataBindingIssueHandler::OnIssueAdded(DAVA::int32 id, const DAVA::String& message, const DAVA::UIControl* control, const DAVA::String& propertyName)
{
    if (accessor->GetActiveContext())
    {
        const DocumentData* data = accessor->GetActiveContext()->GetData<DocumentData>();
        DVASSERT(data != nullptr);

        AddIssue(id, message,
                 data->GetPackagePath().GetFrameworkPath(),
                 DAVA::UIControlHelpers::GetControlPath(control, MakeFunction(this, &DataBindingIssueHandler::IsControlOutOfScope)),
                 propertyName);
    }
}

void DataBindingIssueHandler::OnIssueChanged(DAVA::int32 id, const DAVA::String& message)
{
    ChangeMessage(id, message);
}

void DataBindingIssueHandler::OnIssueRemoved(DAVA::int32 id)
{
    RemoveIssue(id);
}
