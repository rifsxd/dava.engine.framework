#pragma once

#include "Modules/IssueNavigatorModule/IssueHandler.h"

#include <UI/DataBinding/UIDataBindingIssueDelegate.h>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
}

class UIControl;
class LayoutFormula;
}

class IndexGenerator;

class DataBindingIssueHandler : public IssueHandler, public DAVA::UIDataBindingIssueDelegate
{
public:
    DataBindingIssueHandler(DAVA::ContextAccessor* accessor, DAVA::int32 sectionId, IndexGenerator* indexGenerator);
    ~DataBindingIssueHandler() override;

    DAVA::int32 GenerateNewId() override;
    void OnIssueAdded(DAVA::int32 id, const DAVA::String& message, const DAVA::UIControl* control, const DAVA::String& propertyName) override;
    void OnIssueChanged(DAVA::int32 id, const DAVA::String& message) override;
    void OnIssueRemoved(DAVA::int32 id) override;

private:
    IndexGenerator* indexGenerator = nullptr;
};
