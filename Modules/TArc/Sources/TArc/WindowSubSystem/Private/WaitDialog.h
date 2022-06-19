#pragma once

#include "TArc/DataProcessing/DataWrapper.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/Utils/QtConnections.h"

#include <QPointer>
#include <QCursor>

class QDialog;
class QProgressBar;
class QPlainTextEdit;
class QPushButton;

namespace DAVA
{
class WaitDialog : public WaitHandle
{
public:
    WaitDialog(const WaitDialogParams& params, QWidget* parent);
    ~WaitDialog() override;

    void Show();
    void SetMessage(const QString& msg) override;
    void SetRange(uint32 min, uint32 max) override;
    void SetProgressValue(uint32 progress) override;
    bool WasCanceled() const override;

    void Update() override;

    Signal<WaitHandle*> beforeDestroy;

private:
    void CanceledByMouse();

    QPointer<QDialog> dlg;
    QPointer<QProgressBar> progressBar;
    QPointer<QPlainTextEdit> messageLabel;
    QPointer<QPushButton> cancelButton;

    QCursor originalCursor;
    QtConnections connections;

    WaitDialogParams params;
    bool wasCanceled = false;
};
} // namespace DAVA
