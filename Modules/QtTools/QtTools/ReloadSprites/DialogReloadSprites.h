#pragma once

#include "SpritesPacker.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Utils/QtThread.h>
#include <QDialog>

namespace Ui
{
class DialogReloadSprites;
}

class DialogReloadSprites : public QDialog
{
    Q_OBJECT
public:
    explicit DialogReloadSprites(DAVA::ContextAccessor* accessor, SpritesPacker* packer, QWidget* parent = nullptr);
    ~DialogReloadSprites();

private slots:
    void OnStartClicked();
    void OnStopClicked();
    void OnRunningChangedQueued(bool running); //we can work with widgets only in application thread
    void OnRunningChangedDirect(bool running); //we can move to thead only from current thread
    void OnCheckboxShowConsoleToggled(bool checked);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void BlockingStop();

    std::unique_ptr<Ui::DialogReloadSprites> ui;
    SpritesPacker* spritesPacker;
    DAVA::QtThread workerThread; //we need this thread only for "cancel" button

public:
    DAVA::ContextAccessor* accessor = nullptr;
};
