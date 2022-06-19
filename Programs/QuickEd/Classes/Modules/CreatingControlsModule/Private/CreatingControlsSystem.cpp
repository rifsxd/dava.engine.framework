#include "Classes/Modules/CreatingControlsModule/CreatingControlsSystem.h"
#include "Classes/Modules/DocumentsModule/DocumentData.h"
#include "Classes/Modules/ProjectModule/ProjectData.h"

#include "Classes/Model/PackageHierarchy/PackageControlsNode.h"
#include "Classes/Model/PackageHierarchy/PackageBaseNode.h"
#include "Classes/Model/ControlProperties/RootProperty.h"

#include "Classes/UI/CommandExecutor.h"
#include "Classes/Utils/ControlPlacementUtils.h"

#include <Functional/Functional.h>
#include <Engine/PlatformApiQt.h>
#include <Engine/Qt/RenderWidget.h>
#include <UI/UIEvent.h>

namespace CreatingControlsSystemDetails
{
using namespace DAVA;

bool IsEscPressed(UIEvent* input)
{
    return input->device == eInputDevices::KEYBOARD && input->key == eInputElements::KB_ESCAPE;
}
bool IsLMBPressed(UIEvent* input)
{
    return (input->device == eInputDevices::MOUSE && input->phase == UIEvent::Phase::ENDED && input->mouseButton == eMouseButtons::LEFT);
}
}

CreatingControlsSystem::CreatingControlsSystem(DAVA::ContextAccessor* accessor, DAVA::UI* ui)
    : BaseEditorSystem(accessor)
    , ui(ui)
{
    documentDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<DocumentData>());
    BindFields();
}

void CreatingControlsSystem::BindFields()
{
    using namespace DAVA;

    fieldBinder.reset(new FieldBinder(accessor));
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<ProjectData>();
        fieldDescr.fieldName = FastName(ProjectData::projectPathPropertyName);
        fieldBinder->BindField(fieldDescr, DAVA::MakeFunction(this, &CreatingControlsSystem::OnProjectPathChanged));
    }
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
        fieldDescr.fieldName = FastName(DocumentData::packagePropertyName);
        fieldBinder->BindField(fieldDescr, DAVA::MakeFunction(this, &CreatingControlsSystem::OnPackageChanged));
    }
}

void CreatingControlsSystem::OnPackageChanged(const DAVA::Any& package)
{
    CancelCreateByClick();
}

void CreatingControlsSystem::OnProjectPathChanged(const DAVA::Any& projectPath)
{
    CancelCreateByClick();
}

eDragState CreatingControlsSystem::RequireNewState(DAVA::UIEvent* currentInput, eInputSource /*eInputSource*/)
{
    using namespace CreatingControlsSystemDetails;

    if (controlYamlString.empty())
    {
        return eDragState::NoDrag;
    }
    else if (IsEscPressed(currentInput))
    {
        isEscPressed = true;
        return eDragState::NoDrag;
    }
    else if (IsLMBPressed(currentInput))
    {
        isLMBPressed = true;
        return eDragState::NoDrag;
    }
    else
    {
        return eDragState::AddingControl;
    }
}

bool CreatingControlsSystem::CanProcessInput(DAVA::UIEvent* currentInput, eInputSource /*inputSource*/) const
{
    return (isEscPressed || isLMBPressed);
}

void CreatingControlsSystem::ProcessInput(DAVA::UIEvent* currentInput, eInputSource /*inputSource*/)
{
    using namespace DAVA;

    if (isLMBPressed)
    {
        isLMBPressed = false;
        AddControlAtPoint(currentInput->point);
    }
    else if (isEscPressed)
    {
        isEscPressed = false;
        CancelCreateByClick();
    }
}

eSystems CreatingControlsSystem::GetOrder() const
{
    return eSystems::CREATING_CONTROLS;
}

void CreatingControlsSystem::SetCreateByClick(const DAVA::String& _controlYamlString)
{
    controlYamlString = _controlYamlString;
    if (controlYamlString.empty() == false)
    {
        DAVA::PlatformApi::Qt::GetRenderWidget()->setFocus();
    }
}

void CreatingControlsSystem::AddControlAtPoint(const DAVA::Vector2& point)
{
    using namespace DAVA;

    DataContext* active = accessor->GetActiveContext();
    DVASSERT(active != nullptr);
    DocumentData* docData = active->GetData<DocumentData>();
    if (docData != nullptr)
    {
        PackageNode* package = docData->GetPackageNode();
        DVASSERT(package != nullptr);

        uint32 destIndex = 0;
        PackageBaseNode* destNode = GetSystemsManager()->GetControlNodeAtPoint(point);
        if (destNode == nullptr)
        {
            destNode = DynamicTypeCheck<PackageBaseNode*>(package->GetPackageControlsNode());
            destIndex = GetSystemsManager()->GetIndexOfNearestRootControl(point);
        }
        else
        {
            destIndex = destNode->GetCount();
        }

        ControlsContainerNode* destControlContainer = dynamic_cast<ControlsContainerNode*>(destNode);
        if (destControlContainer != nullptr)
        {
            docData->BeginBatch("Copy control from library");
            CommandExecutor executor(accessor, ui);

            DAVA::Set<PackageBaseNode*> newNodes = executor.Paste(docData->GetPackageNode(), destNode, destIndex, controlYamlString);
            if (newNodes.size() == 1)
            {
                ControlNode* newNode = dynamic_cast<ControlNode*>(*(newNodes.begin()));

                if (destNode != package->GetPackageControlsNode())
                {
                    ControlNode* destControl = dynamic_cast<ControlNode*>(destNode);
                    if (destControl != nullptr)
                    {
                        ControlPlacementUtils::SetAbsoulutePosToControlNode(package, newNode, destControl, point);
                        AbstractProperty* postionProperty = newNode->GetRootProperty()->FindPropertyByName("position");
                        AbstractProperty* sizeProperty = newNode->GetRootProperty()->FindPropertyByName("size");
                        newNode->GetRootProperty()->SetProperty(postionProperty, Any(newNode->GetControl()->GetPosition()));
                        newNode->GetRootProperty()->SetProperty(sizeProperty, Any(newNode->GetControl()->GetSize()));
                    }
                }

                SelectedNodes newSelection = { newNode };
                documentDataWrapper.SetFieldValue(DocumentData::selectionPropertyName, newSelection);
            }

            docData->EndBatch();
        }
    }

    CancelCreateByClick();
}

void CreatingControlsSystem::CancelCreateByClick()
{
    controlYamlString.clear();
}
