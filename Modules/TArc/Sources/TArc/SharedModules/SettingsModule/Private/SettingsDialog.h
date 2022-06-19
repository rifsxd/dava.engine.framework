#pragma once

#include "TArc/WindowSubSystem/UI.h"
#include "TArc/Controls/PropertyPanel/PropertiesView.h"

#include <Reflection/Reflection.h>

#include <QDialog>

class QCloseEvent;
namespace DAVA
{
class ContextAccessor;
class UI;
class SettingsDialog final : public QDialog
{
public:
    struct Params
    {
        ContextAccessor* accessor = nullptr;
        UI* ui = nullptr;
        FieldDescriptor objectsField;
    };
    SettingsDialog(const Params& params, QWidget* parent = 0);

    Signal<> resetSettings;

private:
    void OnResetPressed();

private:
    PropertiesView* view = nullptr;
    std::shared_ptr<PropertiesView::Updater> updater;

    Params params;

    DAVA_REFLECTION(SettingsDialog);
};
} // namespace DAVA
