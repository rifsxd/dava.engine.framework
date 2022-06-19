#pragma once

#include <Base/BaseTypes.h>
#include <Functional/Signal.h>
#include <UI/UISystem.h>

/**
This system is created only to send signal beforeRender
This signal designed to update data-driven views before render
*/
class UpdateViewsSystem final : public DAVA::UISystem
{
public:
    DAVA::Signal<> beforeRender;

private:
    void Process(DAVA::float32 elapsedTime) override;
};
