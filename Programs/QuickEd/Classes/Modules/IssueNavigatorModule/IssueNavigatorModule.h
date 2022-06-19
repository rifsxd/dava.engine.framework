#pragma once

#include "Classes/Modules/IssueNavigatorModule/IssueHelper.h"
#include "Classes/Modules/IssueNavigatorModule/IssueData.h"

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <Base/BaseTypes.h>

class IssueHandler;

class IssueNavigatorModule : public DAVA::ClientModule
{
    DAVA_VIRTUAL_REFLECTION(IssueNavigatorModule, DAVA::ClientModule);

private:
    void PostInit() override;
    void OnWindowClosed(const DAVA::WindowKey& key) override;
    void OnContextCreated(DAVA::DataContext* context) override;
    void OnContextWillBeChanged(DAVA::DataContext* current, DAVA::DataContext* newOne) override;

    void OnIssueAvitvated(DAVA::int32 index);

    const DAVA::Vector<IssueData::Issue>& GetValues() const;
    DAVA::Any GetCurrentValue() const;
    void SetCurrentValue(const DAVA::Any& currentValue);
    void OnContextDeleted(DAVA::DataContext* context) override;

    struct HeaderDescription
    {
        int message;
        int pathToControl;
        int packagePath;
        int propertyName;
        DAVA_REFLECTION(HeaderDescription);
    };
    HeaderDescription headerDescription;
    DAVA::QtConnections connections;

    DAVA::int32 current = 0;
};
