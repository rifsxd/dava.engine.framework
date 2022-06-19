#include "RichContentIssuesHandler.h"

#include "Model/ControlProperties/ComponentPropertiesSection.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/IssueNavigatorModule/IssueData.h"
#include "Modules/IssueNavigatorModule/IssueHelper.h"

#include <Engine/Engine.h>
#include <Logger/Logger.h>
#include <UI/UIControl.h>
#include <UI/UIControlHelpers.h>
#include <UI/UIControlSystem.h>
#include <UI/RichContent/UIRichContentAliasesComponent.h>
#include <UI/RichContent/UIRichContentComponent.h>
#include <UI/RichContent/UIRichContentSystem.h>

#include <TArc/Core/ContextAccessor.h>

RichContentIssuesHandler::RichContentIssuesHandler(DAVA::ContextAccessor* accessor_, DAVA::int32 sectionId_, IndexGenerator* indexGenerator_)
    : IssueHandler(accessor_, sectionId_)
    , indexGenerator(indexGenerator_)
    , packageListenerProxy(this, accessor_)
{
    using namespace DAVA;
    UIRichContentSystem* sys = GetEngineContext()->uiControlSystem->GetSystem<UIRichContentSystem>();
    sys->onTextXMLParsingError.Connect(this, &RichContentIssuesHandler::OnTextXMLParsingError);
    sys->onAliasXMLParsingError.Connect(this, &RichContentIssuesHandler::OnAliasXMLParsingError);
    sys->onBeginProcessComponent.Connect(this, &RichContentIssuesHandler::OnBeginProcessComponent);
    sys->onEndProcessComponent.Connect(this, &RichContentIssuesHandler::OnEndProcessComponent);
    sys->onRemoveComponent.Connect(this, &RichContentIssuesHandler::OnRemoveComponent);
}

RichContentIssuesHandler::~RichContentIssuesHandler()
{
    using namespace DAVA;
    UIRichContentSystem* sys = GetEngineContext()->uiControlSystem->GetSystem<UIRichContentSystem>();
    sys->onTextXMLParsingError.Disconnect(this);
    sys->onAliasXMLParsingError.Disconnect(this);
    sys->onBeginProcessComponent.Disconnect(this);
    sys->onEndProcessComponent.Disconnect(this);
    sys->onRemoveComponent.Disconnect(this);
}

void RichContentIssuesHandler::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    using namespace DAVA;
    if (property->GetName() == "Name")
    {
        const DocumentData* data = accessor->GetActiveContext()->GetData<DocumentData>();
        DVASSERT(data != nullptr);

        UIControl* control = node->GetControl();

        for (const auto& pair : issues)
        {
            if (pair.first->GetControl() == control)
            {
                ChangePathToControl(pair.second.id, DAVA::UIControlHelpers::GetControlPath(control, MakeFunction(this, &RichContentIssuesHandler::IsControlOutOfScope)));
            }
        }
    }
}

void RichContentIssuesHandler::OnTextXMLParsingError(DAVA::UIRichContentComponent* component, const DAVA::String& errorMessage)
{
    using namespace DAVA;
    auto it = issues.find(component);
    if (it != issues.end())
    {
        it->second.reset = false;
        ChangeMessage(it->second.id, errorMessage);
    }
    else
    {
        Issue issue{ indexGenerator->NextIssueId() };
        DocumentData* documentData = accessor->GetActiveContext()->GetData<DocumentData>();
        DVASSERT(documentData != nullptr);
        AddIssue(issue.id, errorMessage, documentData->GetPackagePath().GetFrameworkPath(), DAVA::UIControlHelpers::GetControlPath(component->GetControl(), MakeFunction(this, &RichContentIssuesHandler::IsControlOutOfScope)), "RichContent/text");
        issues[component] = issue;
    }
}

void RichContentIssuesHandler::OnAliasXMLParsingError(DAVA::UIRichContentAliasesComponent* component, const DAVA::String& aliasName, const DAVA::String& errorMessage)
{
    using namespace DAVA;
    auto it = issues.find(component);
    if (it != issues.end())
    {
        it->second.reset = false;
        ChangeMessage(it->second.id, errorMessage);
    }
    else
    {
        Issue issue{ indexGenerator->NextIssueId() };
        DocumentData* documentData = accessor->GetActiveContext()->GetData<DocumentData>();
        DVASSERT(documentData != nullptr);
        AddIssue(issue.id, errorMessage, documentData->GetPackagePath().GetFrameworkPath(), DAVA::UIControlHelpers::GetControlPath(component->GetControl(), MakeFunction(this, &RichContentIssuesHandler::IsControlOutOfScope)), "RichContentAliases/aliases");
        issues[component] = issue;
    }
}

void RichContentIssuesHandler::OnBeginProcessComponent(DAVA::UIComponent* component)
{
    using namespace DAVA;
    auto it = issues.find(component);
    if (it != issues.end())
    {
        it->second.reset = true;
    }
}

void RichContentIssuesHandler::OnEndProcessComponent(DAVA::UIComponent* component)
{
    using namespace DAVA;
    auto it = issues.find(component);
    if (it != issues.end())
    {
        if (it->second.reset)
        {
            RemoveIssue(it->second.id);
            issues.erase(it);
        }
    }
}

void RichContentIssuesHandler::OnRemoveComponent(DAVA::UIComponent* component)
{
    using namespace DAVA;
    auto it = issues.find(component);
    if (it != issues.end())
    {
        RemoveIssue(it->second.id);
        issues.erase(it);
    }
}
