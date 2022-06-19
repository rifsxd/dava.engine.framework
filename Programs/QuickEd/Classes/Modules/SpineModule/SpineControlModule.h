#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Qt/QtIcon.h>

#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class UISpineSystem;
}

class SpineControlModule : public DAVA::ClientModule
{
    DAVA_VIRTUAL_REFLECTION(SpineControlModule, DAVA::ClientModule);

protected:
    void OnContextCreated(DAVA::DataContext* context) override;
    void OnContextDeleted(DAVA::DataContext* context) override;
    void PostInit() override;

private:
    bool IsEnabled() const;
    const QIcon& GetRebuildButtonIcon();
    const QIcon& GetPauseButtonIcon();
    void RebuildAllBoneLinks();
    void PlayPause();
    DAVA::UISpineSystem* GetSpineSystem() const;

    const DAVA::String pauseButtonHint = "Play/Pause Spine animations";
    const DAVA::String rebuildButtonHint = "Rebuild all links Spine bones to controls";
};
