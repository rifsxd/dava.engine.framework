#pragma once

#include "TArc/WindowSubSystem/UI.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/Core/FieldBinder.h"

#include <QString>
#include <QDockWidget>

namespace DAVA
{
class DockPanel final : public QDockWidget
{
public:
    struct Params
    {
        ContextAccessor* accessor;
        Map<DockPanelInfo::Fields, FieldDescriptor> descriptors;
    };

    DockPanel(const Params& params, const QString& title, QWidget* parent = nullptr);
    DockPanel(const Params& params, QWidget* parent = nullptr);

private:
    void Setup();
    void OnTitleChanged(const Any& v);
    void OnActiveStateChanged(const Any& v);

    void SetValue(DockPanelInfo::Fields field, const Any& v);

private:
    Map<DockPanelInfo::Fields, FieldDescriptor> descriptors;
    Params params;
    FieldBinder binder;
    bool isActive = false;
};
} // namespace DAVA
