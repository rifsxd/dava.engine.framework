#pragma once

#include "Classes/Modules/BaseEditorModule.h"

#include <TArc/DataProcessing/DataListener.h>
#include <TArc/Utils/QtConnections.h>

#include <Reflection/Reflection.h>

class CanvasDataAdapter;

namespace DAVA
{
class FieldBinder;
}

class CanvasModule : public BaseEditorModule, public DAVA::DataListener
{
public:
    CanvasModule();

private:
    void PostInit() override;
    void CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;
    void DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;
    void OnContextCreated(DAVA::DataContext* context) override;

    void OnRootControlPositionChanged(const DAVA::Vector2& rootControlPos);
    void OnRootControlSizeChanged(const DAVA::Vector2& rootControlSize);
    void OnWorkAreaSizeChanged(const DAVA::Vector2& workAreaSize);

    void OnDataChanged(const DAVA::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

    void CreateData();
    void InitFieldBinder();
    void CreateMenuSeparator();
    void RecreateBgrColorActions();

    DAVA_VIRTUAL_REFLECTION(CanvasModule, DAVA::ClientModule);

    std::unique_ptr<DAVA::FieldBinder> fieldBinder;
    DAVA::QtConnections connections;
    DAVA::DataWrapper wrapper;
};
