#pragma once

#include "Modules/IssueNavigatorModule/IssueHandler.h"
#include "Utils/PackageListenerProxy.h"

#include <Base/BaseTypes.h>

namespace DAVA
{
class ContextAccessor;
class UI;
class UIControl;
class UIComponent;
class UIRichContentComponent;
class UIRichContentAliasesComponent;
}

class IndexGenerator;

class RichContentIssuesHandler final : public IssueHandler, PackageListener
{
public:
    RichContentIssuesHandler(DAVA::ContextAccessor* accessor, DAVA::int32 sectionId, IndexGenerator* indexGenerator);
    ~RichContentIssuesHandler() override;

    // PackageListener
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;

private:
    void OnTextXMLParsingError(DAVA::UIRichContentComponent* component, const DAVA::String& errorMessage);
    void OnAliasXMLParsingError(DAVA::UIRichContentAliasesComponent* component, const DAVA::String& aliasName, const DAVA::String& errorMessage);
    void OnBeginProcessComponent(DAVA::UIComponent* component);
    void OnEndProcessComponent(DAVA::UIComponent* component);
    void OnRemoveComponent(DAVA::UIComponent* component);

    struct Issue
    {
        DAVA::int32 id = 0;
        bool reset = false;
    };

    DAVA::UnorderedMap<DAVA::UIComponent*, Issue> issues;

    IndexGenerator* indexGenerator = nullptr;
    PackageListenerProxy packageListenerProxy;
};
