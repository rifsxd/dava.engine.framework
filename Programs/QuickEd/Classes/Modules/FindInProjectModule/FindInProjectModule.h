#pragma once

#include <Base/BaseTypes.h>
#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

class FindResultsWidget;
class FindFilter;

class FindInProjectModule : public DAVA::ClientModule
{
    void PostInit() override;

    void OnFindInProject();
    void OnFindErrorsAndWarnings();

    DAVA::QtConnections connections;

    DAVA_VIRTUAL_REFLECTION(FindInProjectModule, DAVA::ClientModule);
};
