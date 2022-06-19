#include "Modules/DisplayFrameModule/DisplayFrameModule.h"
#include "Modules/DisplayFrameModule/DisplaySafeArea.h"
#include "Modules/DisplayFrameModule/DisplayFrameData.h"
#include "Modules/DocumentsModule/DocumentData.h"

#include "Interfaces/EditorSystemsManagerInteface.h"

#include <Engine/Engine.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <UI/UIControlSystem.h>

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Core/FieldBinder.h>

DAVA_VIRTUAL_REFLECTION_IMPL(DisplayFrameModule)
{
    DAVA::ReflectionRegistrator<DisplayFrameModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DisplayFrameModule::DisplayFrameModule()
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(DisplaySafeAreaPreferences);
}

void DisplayFrameModule::PostInit()
{
    std::unique_ptr<DisplayFrameData> data = std::make_unique<DisplayFrameData>();
    data->safeArea = std::make_unique<DisplaySafeArea>(GetAccessor());
    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));

    using namespace DAVA;

    fieldBinder.reset(new FieldBinder(GetAccessor()));

    {
        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<DisplaySafeAreaPreferences>();

        fieldDescr.fieldName = FastName("isVisible");
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &DisplayFrameModule::OnSafeAreaChanged));
        fieldDescr.fieldName = FastName("isEnabled");
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &DisplayFrameModule::OnSafeAreaChanged));
        fieldDescr.fieldName = FastName("leftInset");
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &DisplayFrameModule::OnSafeAreaChanged));
        fieldDescr.fieldName = FastName("topInset");
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &DisplayFrameModule::OnSafeAreaChanged));
        fieldDescr.fieldName = FastName("rightInset");
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &DisplayFrameModule::OnSafeAreaChanged));
        fieldDescr.fieldName = FastName("bottomInset");
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &DisplayFrameModule::OnSafeAreaChanged));
        fieldDescr.fieldName = FastName("leftNotch");
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &DisplayFrameModule::OnSafeAreaChanged));
        fieldDescr.fieldName = FastName("rightNotch");
        fieldBinder->BindField(fieldDescr, MakeFunction(this, &DisplayFrameModule::OnSafeAreaChanged));
    }

    virtualSizeChangedToken = GetEngineContext()->uiControlSystem->vcs->virtualSizeChanged.Connect([&](const Size2i& size) {
        OnSafeAreaChanged(Any());
    });
}

void DisplayFrameModule::CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    DisplayFrameData* data = GetAccessor()->GetGlobalContext()->GetData<DisplayFrameData>();
    DVASSERT(data != nullptr);
    systemsManager->RegisterEditorSystem(data->safeArea.get());
}

void DisplayFrameModule::DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    DisplayFrameData* data = GetAccessor()->GetGlobalContext()->GetData<DisplayFrameData>();
    DVASSERT(data != nullptr);
    systemsManager->UnregisterEditorSystem(data->safeArea.get());
    DAVA::GetEngineContext()->uiControlSystem->vcs->virtualSizeChanged.Disconnect(virtualSizeChangedToken);
}

void DisplayFrameModule::OnSafeAreaChanged(const DAVA::Any& values)
{
    using namespace DAVA;

    DisplaySafeAreaPreferences* prefs = GetAccessor()->GetGlobalContext()->GetData<DisplaySafeAreaPreferences>();
    if (prefs->isEnabled)
    {
        VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;
        GetEngineContext()->uiControlSystem->SetPhysicalSafeAreaInsets(vcs->ConvertVirtualToPhysicalX(prefs->leftInset),
                                                                       vcs->ConvertVirtualToPhysicalY(prefs->topInset),
                                                                       vcs->ConvertVirtualToPhysicalX(prefs->rightInset),
                                                                       vcs->ConvertVirtualToPhysicalY(prefs->bottomInset),
                                                                       prefs->isLeftNotch,
                                                                       prefs->isRightNotch);
    }
    else
    {
        GetEngineContext()->uiControlSystem->SetPhysicalSafeAreaInsets(0, 0, 0, 0, false, false);
    }

    DataContext* activeContext = GetAccessor()->GetActiveContext();
    if (activeContext)
    {
        DocumentData* documentData = activeContext->GetData<DocumentData>();
        SortedControlNodeSet rootControls = documentData->GetDisplayedRootControls();
        for (ControlNode* node : rootControls)
        {
            node->GetControl()->SetLayoutDirty();
        }
    }
}

DECL_TARC_MODULE(DisplayFrameModule);
