#pragma once


#include "Model/PackageHierarchy/ControlNode.h"

#include <Base/BaseTypes.h>
#include <Base/Token.h>
#include <QtTools/Updaters/ContinuousUpdater.h>
#include <TArc/Core/ClientModule.h>
#include <TArc/Core/FieldBinder.h>

class PackageBaseNode;
class AbstractProperty;
class ControlNode;
class DataBindingInspectorModel;
class QWidget;

namespace DAVA
{
class UIControl;
class UIComponent;
}

class DataBindingInspectorModule : public DAVA::ClientModule
{
private:
    void PostInit() override;
    void OnWindowClosed(const DAVA::WindowKey& key) override;
    QWidget* InitUI();

    void OnDataModelProcessed(DAVA::UIControl* control, DAVA::UIComponent* component);
    void OnSelectionChanged(const DAVA::Any& selectionValue);
    void OnCurrentControlChanged(const DAVA::Any& val);

    void RefreshData();

    std::unique_ptr<DAVA::FieldBinder> selectionFieldBinder;

    DAVA_VIRTUAL_REFLECTION(DataBindingInspectorModule, DAVA::ClientModule);
};
