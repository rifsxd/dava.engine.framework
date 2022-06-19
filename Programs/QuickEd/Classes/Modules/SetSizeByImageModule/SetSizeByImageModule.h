#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <Reflection/Reflection.h>

class SetSizeByImageModule : public DAVA::ClientModule
{
    // ClientModule
    void PostInit() override;

    void OnSetSizeFromImage();

    DAVA::QtConnections connections;
    DAVA_VIRTUAL_REFLECTION(SetSizeByImageModule, DAVA::ClientModule);
};
