#include "Classes/Modules/HUDModule/HUDModule.h"
#include "Classes/Modules/HUDModule/HUDModuleData.h"
#include "Classes/Modules/HUDModule/Private/HUDSystem.h"
#include "Classes/Modules/DocumentsModule/DocumentData.h"

#include "Interfaces/EditorSystemsManagerInteface.h"

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/Utils/Utils.h>

DAVA_VIRTUAL_REFLECTION_IMPL(HUDModule)
{
    DAVA::ReflectionRegistrator<HUDModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void HUDModule::PostInit()
{
    documentDataWrapper = GetAccessor()->CreateWrapper(DAVA::ReflectedTypeDB::Get<DocumentData>());

    std::unique_ptr<HUDModuleData> data = std::make_unique<HUDModuleData>();
    data->hudSystem = std::make_unique<HUDSystem>(GetAccessor());
    data->hudSystem->highlightChanged.Connect(this, &HUDModule::OnHighlightChanged);
    data->hudSystem->selectionRectChanged.Connect(this, &HUDModule::OnSelectedRectChanged);
    data->hudSystem->selectionByRectStarted.Connect(this, &HUDModule::OnSelectionByRectStarted);
    data->hudSystem->selectionByRectFinished.Connect(this, &HUDModule::OnSelectionByRectFinished);
    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));
}

void HUDModule::CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    HUDModuleData* data = GetAccessor()->GetGlobalContext()->GetData<HUDModuleData>();
    DVASSERT(data != nullptr);
    HUDSystem* system = data->hudSystem.get();

    systemsManager->RegisterEditorSystem(system);
}

void HUDModule::DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    HUDModuleData* data = GetAccessor()->GetGlobalContext()->GetData<HUDModuleData>();
    DVASSERT(data != nullptr);
    HUDSystem* system = data->hudSystem.get();

    systemsManager->UnregisterEditorSystem(system);
}

void HUDModule::OnHighlightChanged(ControlNode* node)
{
    HUDModuleData* data = GetAccessor()->GetGlobalContext()->GetData<HUDModuleData>();
    DVASSERT(data != nullptr);
    data->highlightedNode = node;
}

void HUDModule::OnSelectionByRectStarted()
{
    using namespace DAVA;

    selectionByRectCache.clear();
    if (IsKeyPressed(DAVA::eModifierKeys::SHIFT))
    {
        DataContext* activeContext = GetAccessor()->GetActiveContext();
        DVASSERT(activeContext != nullptr);
        DocumentData* documentData = activeContext->GetData<DocumentData>();

        selectionByRectCache = documentData->GetSelectedNodes();
    }
}

void HUDModule::OnSelectionByRectFinished()
{
    selectionByRectCache.clear();
}

void HUDModule::OnSelectedRectChanged(const DAVA::Rect& rect)
{
    using namespace DAVA;

    Set<PackageBaseNode*> newSelection = selectionByRectCache;

    DataContext* activeContext = GetAccessor()->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* documentData = activeContext->GetData<DocumentData>();

    for (const PackageBaseNode* node : documentData->GetDisplayedRootControls())
    {
        for (int i = 0, count = node->GetCount(); i < count; ++i)
        {
            PackageBaseNode* child = node->Get(i);
            DAVA::UIControl* control = child->GetControl();
            DVASSERT(nullptr != control);
            if (control->IsVisible() && rect.RectContains(control->GetGeometricData().GetAABBox()))
            {
                if (IsKeyPressed(DAVA::eModifierKeys::SHIFT) && selectionByRectCache.find(child) != selectionByRectCache.end())
                {
                    newSelection.erase(child);
                }
                else
                {
                    newSelection.insert(child);
                }
            }
        }
    }
    DVASSERT(documentDataWrapper.HasData());
    documentDataWrapper.SetFieldValue(DocumentData::selectionPropertyName, newSelection);
}

DECL_TARC_MODULE(HUDModule);
