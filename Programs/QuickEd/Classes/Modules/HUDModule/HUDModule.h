#pragma once

#include "Classes/Modules/BaseEditorModule.h"

class ControlNode;
class PackageBaseNode;
namespace DAVA
{
struct Rect;
}

class HUDModule : public BaseEditorModule
{
private:
    void PostInit() override;
    void CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;
    void DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;

    void OnHighlightChanged(ControlNode* node);

    void OnSelectionByRectStarted();
    void OnSelectionByRectFinished();
    void OnSelectedRectChanged(const DAVA::Rect& rect);

    DAVA::DataWrapper documentDataWrapper;
    DAVA::Set<PackageBaseNode*> selectionByRectCache;

    DAVA_VIRTUAL_REFLECTION(HUDModule, DAVA::ClientModule);
};
