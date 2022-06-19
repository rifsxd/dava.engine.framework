#pragma once

#include <Base/BaseTypes.h>
#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>
#include <TArc/DataProcessing/DataWrapper.h>
#include <TArc/DataProcessing/DataListener.h>

class ModernPropertiesModule : public DAVA::ClientModule
{
    DAVA_VIRTUAL_REFLECTION(ModernPropertiesModule, DAVA::ClientModule);

private:
    void PostInit() override;

    DAVA::QtConnections connections;
};
