#include "Classes/Modules/CanvasModule/CanvasModuleData.h"
#include "Classes/Modules/CanvasModule/EditorControlsView.h"
#include "Classes/Modules/CanvasModule/EditorCanvas.h"
#include "Classes/Modules/CanvasModule/CanvasDataAdapter.h"

DAVA_VIRTUAL_REFLECTION_IMPL(CanvasModuleData)
{
    DAVA::ReflectionRegistrator<CanvasModuleData>::Begin()
    .ConstructorByPointer()
    .End();
}

CanvasModuleData::CanvasModuleData()
{
    canvas.Set(new DAVA::UIControl());
    canvas->SetName("canvas");
}

CanvasModuleData::~CanvasModuleData() = default;

const EditorCanvas* CanvasModuleData::GetEditorCanvas() const
{
    return editorCanvas.get();
}

const EditorControlsView* CanvasModuleData::GetEditorControlsView() const
{
    return controlsView.get();
}
