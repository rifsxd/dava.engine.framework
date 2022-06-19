#include "Modules/FindInProjectModule/FindInProjectModule.h"
#include "Application/QEGlobal.h"
#include "Modules/ProjectModule/ProjectData.h"
#include "UI/Find/Filters/FindFilter.h"
#include "UI/Find/Widgets/FindInProjectDialog.h"
#include "UI/Find/Widgets/FindInDocumentWidget.h"
#include "UI/Find/FindInDocumentController.h"
#include "UI/Find/Filters/AnchorsAndSizePoliciesConflictFilter.h"
#include "UI/Find/Filters/CompositeFilter.h"

#include <TArc/DataProcessing/Common.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/Utils/ModuleCollection.h>

#include <Reflection/Reflection.h>
#include <Reflection/ReflectionRegistrator.h>

namespace FindInProjectDetail
{
class FindInProjectData : public DAVA::TArcDataNode
{
public:
    std::unique_ptr<FindInDocumentController> widgetController;

private:
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(FindInProjectData, DAVA::TArcDataNode)
    {
        DAVA::ReflectionRegistrator<FindInProjectData>::Begin()
        .End();
    }
};
}

DAVA_VIRTUAL_REFLECTION_IMPL(FindInProjectModule)
{
    DAVA::ReflectionRegistrator<FindInProjectModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void FindInProjectModule::PostInit()
{
    using namespace DAVA;

    UI* ui = GetUI();
    ContextAccessor* accessor = GetAccessor();

    FieldDescriptor packageFieldDescr;
    packageFieldDescr.type = ReflectedTypeDB::Get<ProjectData>();
    packageFieldDescr.fieldName = FastName(ProjectData::projectPathPropertyName);

    const auto updater =
    [](const Any& fieldValue) -> Any {
        return !fieldValue.Cast<FilePath>(FilePath()).IsEmpty();
    };

    const QString selectCurrentDocumentActionName = QStringLiteral("Select Current Document in File System");
    {
        QtAction* findInProjectAction = new QtAction(accessor, QObject::tr("Find in Project..."), nullptr);

        KeyBindableActionInfo info;
        info.blockName = "Find";
        info.context = Qt::ApplicationShortcut;
        info.defaultShortcuts.push_back(QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_F));
        MakeActionKeyBindable(findInProjectAction, info);

        findInProjectAction->SetStateUpdationFunction(QtAction::Enabled, packageFieldDescr, updater);

        connections.AddConnection(findInProjectAction, &QAction::triggered, MakeFunction(this, &FindInProjectModule::OnFindInProject));

        ActionPlacementInfo placementInfo(CreateMenuPoint("Find", InsertionParams(InsertionParams::eInsertionMethod::BeforeItem, selectCurrentDocumentActionName)));
        ui->AddAction(DAVA::mainWindowKey, placementInfo, findInProjectAction);
    }

    {
        QtAction* findErrorsAndWarningAction = new QtAction(accessor, QObject::tr("Find Errors And Warnings"), nullptr);

        KeyBindableActionInfo info;
        info.blockName = "Find";
        info.context = Qt::ApplicationShortcut;
        MakeActionKeyBindable(findErrorsAndWarningAction, info);

        findErrorsAndWarningAction->SetStateUpdationFunction(QtAction::Enabled, packageFieldDescr, updater);

        connections.AddConnection(findErrorsAndWarningAction, &QAction::triggered, MakeFunction(this, &FindInProjectModule::OnFindErrorsAndWarnings));

        ActionPlacementInfo placementInfo(CreateMenuPoint("Find", InsertionParams(InsertionParams::eInsertionMethod::BeforeItem, selectCurrentDocumentActionName)));
        ui->AddAction(DAVA::mainWindowKey, placementInfo, findErrorsAndWarningAction);
    }

    FindInProjectDetail::FindInProjectData* data = new FindInProjectDetail::FindInProjectData();
    data->widgetController.reset(new FindInDocumentController(ui, accessor));
    connections.AddConnection(data->widgetController.get(), &FindInDocumentController::FindInDocumentRequest, [this](std::shared_ptr<FindFilter> filter)
                              {
                                  InvokeOperation(QEGlobal::FindInDocument.ID, filter);
                              });

    connections.AddConnection(data->widgetController.get(), &FindInDocumentController::SelectControlRequest, [this](const QString& path, const QString& name)
                              {
                                  InvokeOperation(QEGlobal::SelectControl.ID, path, name);
                              });

    GetAccessor()->GetGlobalContext()->CreateData(std::unique_ptr<DAVA::TArcDataNode>(data));
}

void FindInProjectModule::OnFindInProject()
{
    FindInProjectDialog findInProjectDialog;
    if (findInProjectDialog.exec() == QDialog::Accepted)
    {
        InvokeOperation(QEGlobal::FindInProject.ID, std::shared_ptr<FindFilter>(findInProjectDialog.BuildFindFilter()));
    }
}

void FindInProjectModule::OnFindErrorsAndWarnings()
{
    DAVA::Vector<std::shared_ptr<FindFilter>> errorsAndWarningsFilters;
    errorsAndWarningsFilters.push_back(std::make_shared<AnchorsSizePoliciesConflictFilter>());

    std::shared_ptr<CompositeFilter> filter = std::make_shared<CompositeFilter>(errorsAndWarningsFilters);
    filter->SetCompositionType(CompositeFilter::OR);
    InvokeOperation(QEGlobal::FindInProject.ID, std::static_pointer_cast<FindFilter>(filter));
}

DECL_TARC_MODULE(FindInProjectModule);