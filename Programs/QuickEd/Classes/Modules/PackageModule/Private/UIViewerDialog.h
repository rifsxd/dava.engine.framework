#pragma once

#include <Base/BaseTypes.h>

#include <QDialog>

#include <memory>

namespace DAVA
{
class ContextAccessor;
class UI;
}

class QWidget;
class UIViewerDialog : public QDialog
{
public:
    UIViewerDialog(DAVA::ContextAccessor* accessor, DAVA::UI* ui, QWidget* parent = nullptr);
    ~UIViewerDialog();

    void SetDeviceIndex(DAVA::int32 index);
    DAVA::int32 GetDeviceIndex() const;

    void SetBlankIndex(DAVA::int32 index);
    DAVA::int32 GetBlankIndex() const;

    void SetFlowFlag(bool value);
    bool GetFlowFlag() const;

private:
    DAVA::ContextAccessor* accessor = nullptr;

    struct RunData;
    std::unique_ptr<RunData> runData;
};
