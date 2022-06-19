#pragma once

#include <TArc/Core/ClientModule.h>

class StyleSheetInspectorModule : public DAVA::ClientModule
{
    void PostInit() override;

    DAVA_VIRTUAL_REFLECTION(StyleSheetInspectorModule, DAVA::ClientModule);
};
