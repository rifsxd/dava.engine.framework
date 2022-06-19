#include "Base/GlobalEnum.h"
#include "FileSystem/KeyedArchive.h"

#include "QtTools/ReloadSprites/DialogReloadSprites.h"
#include "QtTools/ReloadSprites/SpritesPacker.h"
#include "QtTools/ConsoleWidget/LoggerOutputObject.h"

#include <TextureCompression/TextureConverter.h>
#include <TArc/DataProcessing/PropertiesHolder.h>
#include <TArc/Qt/QtByteArray.h>

#include "ui_DialogReloadSprites.h"
#include <QTimer>

DialogReloadSprites::DialogReloadSprites(DAVA::ContextAccessor* accessor_, SpritesPacker* packer, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DialogReloadSprites)
    , spritesPacker(packer)
    , accessor(accessor_)
{
    ui->setupUi(this);

    DAVA::PropertiesItem item = accessor->CreatePropertiesNode("DialogReloadSprites");
    DAVA::eGPUFamily currentGpu = item.Get("currentGpu", DAVA::GPU_ORIGIN);
    DAVA::TextureConverter::eConvertQuality quality = item.Get("quality", DAVA::TextureConverter::ECQ_DEFAULT);
    bool forceRepack = item.Get("forcreRepack", false);
    bool showConsole = item.Get("showConsole", true);
    QByteArray consoleState = item.Get("consoleState", QByteArray());

    DVASSERT(nullptr != spritesPacker);
    qRegisterMetaType<DAVA::eGPUFamily>("DAVA::eGPUFamily");
    qRegisterMetaType<DAVA::TextureConverter::eConvertQuality>("DAVA::TextureConverter::eConvertQuality");

    workerThread.setStackSize(16 * 1024 * 1024);

    LoggerOutputObject* loggerOutput = new LoggerOutputObject(this); //will be removed by dialog
    connect(loggerOutput, &LoggerOutputObject::OutputReady, ui->logWidget, &LogWidget::AddMessage, Qt::DirectConnection);

    ui->pushButton_start->setDisabled(spritesPacker->IsRunning());
    ui->comboBox_targetGPU->setDisabled(spritesPacker->IsRunning());
    ui->comboBox_quality->setDisabled(spritesPacker->IsRunning());
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, ui->pushButton_start, &QWidget::setDisabled);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, ui->comboBox_targetGPU, &QWidget::setDisabled);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, ui->comboBox_quality, &QWidget::setDisabled);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, ui->checkBox_repack, &QWidget::setDisabled);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, this, &DialogReloadSprites::OnRunningChangedQueued, Qt::QueuedConnection);
    connect(spritesPacker, &::SpritesPacker::RunningStateChanged, this, &DialogReloadSprites::OnRunningChangedDirect, Qt::DirectConnection);
    connect(ui->pushButton_cancel, &QPushButton::clicked, this, &DialogReloadSprites::OnStopClicked);
    connect(ui->pushButton_start, &QPushButton::clicked, this, &DialogReloadSprites::OnStartClicked);
    connect(ui->checkBox_showConsole, &QCheckBox::toggled, this, &DialogReloadSprites::OnCheckboxShowConsoleToggled);

    const auto& gpuMap = GlobalEnumMap<DAVA::eGPUFamily>::Instance();
    for (size_t i = 0; i < gpuMap->GetCount(); ++i)
    {
        int value;
        bool ok = gpuMap->GetValue(i, value);
        if (!ok)
        {
            DVASSERT(ok, "wrong enum used to create GPU list");
            break;
        }
        ui->comboBox_targetGPU->addItem(gpuMap->ToString(value), value);
    }
    ui->comboBox_targetGPU->setCurrentText(gpuMap->ToString(currentGpu));

    const auto& qualityMap = GlobalEnumMap<DAVA::TextureConverter::eConvertQuality>::Instance();
    for (size_t i = 0; i < qualityMap->GetCount(); ++i)
    {
        int value;
        bool ok = qualityMap->GetValue(i, value);
        if (!ok)
        {
            DVASSERT(ok, "wrong enum used to create quality list");
            break;
        }
        ui->comboBox_quality->addItem(qualityMap->ToString(value), value);
    }
    ui->comboBox_quality->setCurrentText(qualityMap->ToString(quality));
    ui->checkBox_showConsole->setChecked(showConsole);
    ui->checkBox_repack->setChecked(forceRepack);
    ui->logWidget->Deserialize(consoleState);
}

DialogReloadSprites::~DialogReloadSprites()
{
    if (spritesPacker->IsRunning())
    {
        BlockingStop();
    }

    DAVA::PropertiesItem item = accessor->CreatePropertiesNode("DialogReloadSprites");
    item.Set("currentGpu", static_cast<DAVA::eGPUFamily>(ui->comboBox_targetGPU->currentData().value<int>()));
    item.Set("quality", static_cast<DAVA::TextureConverter::eConvertQuality>(ui->comboBox_quality->currentData().value<int>()));
    item.Set("forcreRepack", ui->checkBox_repack->isChecked());
    item.Set("showConsole", ui->checkBox_showConsole->isChecked());
    item.Set("consoleState", ui->logWidget->Serialize());
}

void DialogReloadSprites::OnStartClicked()
{
    const auto gpuData = ui->comboBox_targetGPU->currentData();
    const auto qualityData = ui->comboBox_quality->currentData();
    if (!gpuData.isValid() || !qualityData.isValid())
    {
        return;
    }
    spritesPacker->moveToThread(&workerThread);
    workerThread.start();
    auto gpuType = static_cast<DAVA::eGPUFamily>(gpuData.toInt());
    auto quality = static_cast<DAVA::TextureConverter::eConvertQuality>(qualityData.toInt());
    QMetaObject::invokeMethod(spritesPacker, "ReloadSprites", Qt::QueuedConnection, Q_ARG(bool, true), Q_ARG(bool, ui->checkBox_repack->isChecked()), Q_ARG(DAVA::eGPUFamily, gpuType), Q_ARG(DAVA::TextureConverter::eConvertQuality, quality));
}

void DialogReloadSprites::OnStopClicked()
{
    if (spritesPacker->IsRunning())
    {
        BlockingStop();
    }
    else
    {
        close();
    }
}

void DialogReloadSprites::OnRunningChangedQueued(bool running)
{
    ui->pushButton_cancel->setText(running ? "Cancel" : "Close");
    if (!running)
    {
        workerThread.quit();
        workerThread.wait();
    }
}

void DialogReloadSprites::OnRunningChangedDirect(bool /*running*/)
{
    spritesPacker->moveToThread(qApp->thread());
}

void DialogReloadSprites::OnCheckboxShowConsoleToggled(bool checked)
{
    ui->logWidget->setVisible(checked);
    if (!checked)
    {
        setFixedHeight(minimumSizeHint().height());
    }
    else
    {
        setMinimumHeight(minimumSizeHint().height());
        setMaximumHeight(QWIDGETSIZE_MAX);
    }
}

void DialogReloadSprites::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event);
    BlockingStop();
}

void DialogReloadSprites::BlockingStop()
{
    this->setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QEventLoop loop;
    connect(&workerThread, &QThread::finished, &loop, &QEventLoop::quit);
    spritesPacker->Cancel();
    if (workerThread.isRunning())
    {
        loop.exec();
    }

    QApplication::restoreOverrideCursor();
    this->setEnabled(true);
}
