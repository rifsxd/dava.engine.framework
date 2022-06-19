#include "LayoutIssueHandler.h"

#include "Modules/IssueNavigatorModule/IssueData.h"
#include "Modules/IssueNavigatorModule/IssueHelper.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Model/ControlProperties/AbstractProperty.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"

#include <Engine/Engine.h>
#include <UI/UIControlSystem.h>
#include <UI/UIControl.h>
#include <UI/UIControlHelpers.h>
#include <UI/Layouts/UILayoutSystem.h>
#include <UI/Layouts/LayoutFormula.h>

#include <TArc/Core/ContextAccessor.h>

LayoutIssueHandler::LayoutIssueHandler(DAVA::ContextAccessor* accessor_, DAVA::int32 sectionId_, IndexGenerator* indexGenerator_)
    : IssueHandler(accessor_, sectionId_)
    , indexGenerator(indexGenerator_)
    , packageListenerProxy(this, accessor_)
{
    accessor->GetEngineContext()->uiControlSystem->GetLayoutSystem()->formulaProcessed.Connect(this, &LayoutIssueHandler::OnFormulaProcessed);
    accessor->GetEngineContext()->uiControlSystem->GetLayoutSystem()->formulaRemoved.Connect(this, &LayoutIssueHandler::OnFormulaRemoved);
}

LayoutIssueHandler::~LayoutIssueHandler()
{
    accessor->GetEngineContext()->uiControlSystem->GetLayoutSystem()->formulaProcessed.Disconnect(this);
    accessor->GetEngineContext()->uiControlSystem->GetLayoutSystem()->formulaRemoved.Disconnect(this);
}

void LayoutIssueHandler::OnFormulaProcessed(DAVA::UIControl* control, DAVA::Vector2::eAxis axis, const DAVA::LayoutFormula* formula)
{
    if (formula->HasError())
    {
        DocumentData* documentData = accessor->GetActiveContext()->GetData<DocumentData>();
        DVASSERT(documentData != nullptr);

        auto it = createdIssues[axis].find(control);
        if (it != createdIssues[axis].end())
        {
            ChangeMessage(it->second, formula->GetErrorMessage());
        }
        else
        {
            DAVA::int32 nextIssueId = indexGenerator->NextIssueId();
            AddIssue(nextIssueId,
                     formula->GetErrorMessage(),
                     documentData->GetPackagePath().GetFrameworkPath(),
                     DAVA::UIControlHelpers::GetControlPath(control, MakeFunction(this, &LayoutIssueHandler::IsControlOutOfScope)),
                     axis == DAVA::Vector2::AXIS_X ? "SizePolicy/horizontalFormula" : "SizePolicy/verticalFormula");

            createdIssues[axis][control] = nextIssueId;
        }
    }
    else
    {
        RemoveIssueForControl(control, axis);
    }
}

void LayoutIssueHandler::OnFormulaRemoved(DAVA::UIControl* control, DAVA::Vector2::eAxis axis, const DAVA::LayoutFormula* formula)
{
    RemoveIssueForControl(control, axis);
}

void LayoutIssueHandler::RemoveIssueForControl(DAVA::UIControl* control, DAVA::Vector2::eAxis axis)
{
    auto it = createdIssues[axis].find(control);
    if (it != createdIssues[axis].end())
    {
        RemoveIssue(it->second);
        createdIssues[axis].erase(it);
    }
}

void LayoutIssueHandler::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    using namespace DAVA;

    if (property->GetName() == "Name")
    {
        const DocumentData* data = accessor->GetActiveContext()->GetData<DocumentData>();
        DVASSERT(data != nullptr);

        UIControl* control = node->GetControl();

        for (auto& axisIssuesMap : createdIssues)
        {
            auto it = axisIssuesMap.find(control);
            if (it != axisIssuesMap.end())
            {
                ChangePathToControl(it->second, DAVA::UIControlHelpers::GetControlPath(control, MakeFunction(this, &LayoutIssueHandler::IsControlOutOfScope)));
            }
        }
    }
}
