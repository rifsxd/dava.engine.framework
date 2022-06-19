#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/DataProcessing/DataWrapper.h>
#include <TArc/DataProcessing/DataListener.h>
#include <TArc/Utils/QtConnections.h>
#include <TArc/Utils/QtDelayedExecutor.h>
#include <TArc/Qt/QtString.h>

#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class FieldBinder;
}

class ResourceSelectorModule : public DAVA::ClientModule, public DAVA::DataListener
{
    DAVA_VIRTUAL_REFLECTION(ResourceSelectorModule, DAVA::ClientModule);

public:
    ResourceSelectorModule();

    void OnDataChanged(const DAVA::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

protected:
    void PostInit() override;

private:
    void CreateAction(const QString& actionName, const QString& prevActionName, DAVA::int32 gfxMode);

    void OnGfxSelected(DAVA::int32 gfxMode);
    void OnWindowResized(DAVA::Size2f windowSize);
    void RegisterGfxFolders();
    void OnGraphicsSettingsChanged();
    void ReloadSpritesImpl();

    void RefreshUIControls(const DAVA::Any& package);
    std::unique_ptr<DAVA::FieldBinder> fieldBinder;

    DAVA::DataWrapper projectDataWrapper;
    DAVA::QtConnections connections;

    DAVA::QtDelayedExecutor delayedExecutor;
    DAVA::Vector<QString> gfxActionPlacementName;

    DAVA::int32 resourceIndex = 0;
    bool settingsAreFiltered = false;
};
