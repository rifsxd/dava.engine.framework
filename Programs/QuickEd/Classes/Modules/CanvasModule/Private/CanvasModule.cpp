#include "Classes/Modules/CanvasModule/CanvasModule.h"
#include "Classes/Modules/CanvasModule/CanvasModuleData.h"
#include "Classes/Modules/CanvasModule/EditorCanvas.h"
#include "Classes/Modules/CanvasModule/EditorControlsView.h"
#include "Classes/Modules/CanvasModule/CanvasData.h"
#include "Classes/Modules/CanvasModule/CanvasDataAdapter.h"

#include "Classes/UI/Preview/PreviewWidgetSettings.h"
#include "Classes/UI/Preview/Data/CentralWidgetData.h"

#include "Classes/Interfaces/EditorSystemsManagerInteface.h"

#include <TArc/Core/FieldBinder.h>
#include <TArc/DataProcessing/Common.h>
#include <TArc/Qt/QtIcon.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/QtAction.h>

#include <UI/Input/UIInputSystem.h>
#include <UI/UIControlSystem.h>

#include <QList>
#include <QString>
#include <QMenu>

DAVA_VIRTUAL_REFLECTION_IMPL(CanvasModule)
{
    DAVA::ReflectionRegistrator<CanvasModule>::Begin()
    .ConstructorByPointer()
    .End();
}

CanvasModule::CanvasModule()
{
}

void CanvasModule::PostInit()
{
    CreateData();
    InitFieldBinder();
    CreateMenuSeparator();
    RecreateBgrColorActions();

    wrapper = GetAccessor()->CreateWrapper(DAVA::ReflectedTypeDB::Get<PreviewWidgetSettings>());
    wrapper.SetListener(this);
}

void CanvasModule::CreateData()
{
    DAVA::ContextAccessor* accessor = GetAccessor();

    std::unique_ptr<CanvasModuleData> data = std::make_unique<CanvasModuleData>();
    data->editorCanvas = std::make_unique<EditorCanvas>(accessor);
    data->controlsView = std::make_unique<EditorControlsView>(data->canvas.Get(), accessor);
    data->canvasDataAdapter = std::make_unique<CanvasDataAdapter>(accessor);

    data->controlsView->workAreaSizeChanged.Connect(this, &CanvasModule::OnWorkAreaSizeChanged);
    data->controlsView->rootControlSizeChanged.Connect(this, &CanvasModule::OnRootControlSizeChanged);
    data->controlsView->rootControlPositionChanged.Connect(this, &CanvasModule::OnRootControlPositionChanged);

    accessor->GetGlobalContext()->CreateData(std::move(data));
}

void CanvasModule::InitFieldBinder()
{
    using namespace DAVA;

    Function<void(const Any&)> tryCentralize = [this](const Any&) {
        CanvasModuleData* canvasModuleData = GetAccessor()->GetGlobalContext()->GetData<CanvasModuleData>();
        canvasModuleData->canvasDataAdapter->TryCentralizeScene();
    };

    fieldBinder.reset(new FieldBinder(GetAccessor()));
    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<CentralWidgetData>();
        fieldDescr.fieldName = FastName(CentralWidgetData::viewSizePropertyName);
        fieldBinder->BindField(fieldDescr, tryCentralize);
    }
}

void CanvasModule::CreateMenuSeparator()
{
    using namespace DAVA;
    QAction* separator = new QAction(nullptr);
    separator->setObjectName("bgrMenuSeparator");
    separator->setSeparator(true);
    ActionPlacementInfo placementInfo;
    placementInfo.AddPlacementPoint(CreateMenuPoint(MenuItems::menuView, { InsertionParams::eInsertionMethod::AfterItem, "Dock" }));
    GetUI()->AddAction(DAVA::mainWindowKey, placementInfo, separator);
}

void CanvasModule::RecreateBgrColorActions()
{
    using namespace DAVA;

    CanvasModuleData* data = GetAccessor()->GetGlobalContext()->GetData<CanvasModuleData>();
    std::for_each(data->bgrColorActions.begin(), data->bgrColorActions.end(), [this](const CanvasModuleData::ActionInfo& actionInfo)
                  {
                      GetUI()->RemoveAction(DAVA::mainWindowKey, actionInfo.placement, actionInfo.name);
                  });

    QString menuBgrColor = "Background Color";
    {
        QMenu* backgroundMenu = new QMenu(menuBgrColor, nullptr);
        ActionPlacementInfo placement;
        placement.AddPlacementPoint(CreateMenuPoint(MenuItems::menuView, { InsertionParams::eInsertionMethod::AfterItem, "bgrMenuSeparator" }));
        GetUI()->AddAction(mainWindowKey, placement, backgroundMenu->menuAction());
    }

    FieldDescriptor indexFieldDescr;
    indexFieldDescr.type = ReflectedTypeDB::Get<PreviewWidgetSettings>();
    indexFieldDescr.fieldName = FastName("backgroundColorIndex");

    FieldDescriptor colorsFieldDescr;
    colorsFieldDescr.type = ReflectedTypeDB::Get<PreviewWidgetSettings>();
    colorsFieldDescr.fieldName = DAVA::FastName("backgroundColors");

    ActionPlacementInfo placement(CreateMenuPoint(QList<QString>() << MenuItems::menuView << menuBgrColor));

    PreviewWidgetSettings* settings = GetAccessor()->GetGlobalContext()->GetData<PreviewWidgetSettings>();
    const Vector<Color>& colors = settings->backgroundColors;

    data->bgrColorActions.resize(colors.size());
    for (DAVA::uint32 currentIndex = 0; currentIndex < colors.size(); ++currentIndex)
    {
        QtAction* action = new QtAction(GetAccessor(), QString("Background color %1").arg(currentIndex));
        action->SetStateUpdationFunction(QtAction::Icon, colorsFieldDescr, [currentIndex](const Any& v)
                                         {
                                             const Vector<Color>& colors = v.Cast<Vector<Color>>();
                                             if (currentIndex < colors.size())
                                             {
                                                 Any color = colors[currentIndex];
                                                 return color.Cast<QIcon>(QIcon());
                                             }
                                             else
                                             {
                                                 return QIcon();
                                             }
                                         });

        action->SetStateUpdationFunction(QtAction::Checked, indexFieldDescr, [currentIndex](const Any& v)
                                         {
                                             return v.Cast<DAVA::uint32>(-1) == currentIndex;
                                         });
        connections.AddConnection(action, &QAction::triggered, [this, currentIndex]()
                                  {
                                      PreviewWidgetSettings* settings = GetAccessor()->GetGlobalContext()->GetData<PreviewWidgetSettings>();
                                      settings->backgroundColorIndex = currentIndex;
                                  });

        GetUI()->AddAction(DAVA::mainWindowKey, placement, action);
        CanvasModuleData::ActionInfo& actionInfo = data->bgrColorActions[currentIndex];
        actionInfo.name = action->text();
        actionInfo.placement = placement;
    }
}

void CanvasModule::CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    CanvasModuleData* data = GetAccessor()->GetGlobalContext()->GetData<CanvasModuleData>();
    DVASSERT(data != nullptr);

    EditorCanvas* editorCanvas = data->editorCanvas.get();
    EditorControlsView* controlsView = data->controlsView.get();

    systemsManager->RegisterEditorSystem(editorCanvas);
    systemsManager->RegisterEditorSystem(controlsView);
}

void CanvasModule::DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    CanvasModuleData* data = GetAccessor()->GetGlobalContext()->GetData<CanvasModuleData>();
    DVASSERT(data != nullptr);

    EditorCanvas* editorCanvas = data->editorCanvas.get();
    EditorControlsView* controlsView = data->controlsView.get();

    systemsManager->UnregisterEditorSystem(editorCanvas);
    systemsManager->UnregisterEditorSystem(controlsView);
}

void CanvasModule::OnContextCreated(DAVA::DataContext* context)
{
    std::unique_ptr<CanvasData> canvasData = std::make_unique<CanvasData>();
    context->CreateData(std::move(canvasData));
}

void CanvasModule::OnDataChanged(const DAVA::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    RecreateBgrColorActions();
}

void CanvasModule::OnRootControlPositionChanged(const DAVA::Vector2& rootControlPos)
{
    DAVA::DataContext* activeContext = GetAccessor()->GetActiveContext();
    if (activeContext == nullptr)
    {
        return;
    }

    CanvasData* canvasData = activeContext->GetData<CanvasData>();
    canvasData->rootRelativePosition = rootControlPos;
}

void CanvasModule::OnRootControlSizeChanged(const DAVA::Vector2& rootControlSize)
{
    DAVA::DataContext* activeContext = GetAccessor()->GetActiveContext();
    if (activeContext == nullptr)
    {
        return;
    }

    CanvasData* canvasData = activeContext->GetData<CanvasData>();
    canvasData->rootControlSize = rootControlSize;
}

void CanvasModule::OnWorkAreaSizeChanged(const DAVA::Vector2& workAreaSize)
{
    DAVA::DataContext* activeContext = GetAccessor()->GetActiveContext();
    if (activeContext == nullptr)
    {
        return;
    }

    CanvasData* canvasData = activeContext->GetData<CanvasData>();
    canvasData->workAreaSize = workAreaSize;
    GetAccessor()->GetGlobalContext()->GetData<CanvasModuleData>()->canvasDataAdapter->TryCentralizeScene();
}

DECL_TARC_MODULE(CanvasModule);