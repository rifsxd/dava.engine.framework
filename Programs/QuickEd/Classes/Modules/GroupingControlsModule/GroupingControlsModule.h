#pragma once

#include "EditorSystems/SelectionContainer.h"

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <Reflection/Reflection.h>

class ControlNode;

class GroupingControlsModule : public DAVA::ClientModule
{
public:
    GroupingControlsModule();

private:
    // ClientModule
    void PostInit() override;

    void DoGroup();
    void DoUngroup();

    DAVA::Result CanGroupSelection(const SelectedNodes& selectedNodes) const;
    DAVA::Result CanUngroupSelection(const SelectedNodes& selectedNodes) const;

    DAVA::QtConnections connections;
    DAVA::DataWrapper documentDataWrapper;
    DAVA::RefPtr<ControlNode> sampleGroupNode;

    DAVA_VIRTUAL_REFLECTION(GroupingControlsModule, DAVA::ClientModule);
};
