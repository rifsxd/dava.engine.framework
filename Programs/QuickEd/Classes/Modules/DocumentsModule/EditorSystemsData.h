#pragma once

#include <TArc/DataProcessing/TArcDataNode.h>

class EditorSystemsManager;
class ControlNode;

namespace Painting
{
class Painter;
}

class EditorSystemsData : public DAVA::TArcDataNode
{
public:
    EditorSystemsData();
    ~EditorSystemsData() override;

    bool IsHighlightDisabled() const;

    static DAVA::FastName emulationModePropertyName;

private:
    friend class BaseEditorSystem;
    friend class DocumentsModule;

    const EditorSystemsManager* GetSystemsManager() const;

    std::unique_ptr<EditorSystemsManager> systemsManager;
    std::unique_ptr<Painting::Painter> painter;

    bool emulationMode = false;
    bool highlightDisabled = false;

    DAVA_VIRTUAL_REFLECTION(PixelGridData, DAVA::TArcDataNode);
};
