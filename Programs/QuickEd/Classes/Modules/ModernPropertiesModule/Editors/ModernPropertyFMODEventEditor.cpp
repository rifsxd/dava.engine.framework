#include "Modules/ModernPropertiesModule/Editors/ModernPropertyFMODEventEditor.h"

#include "Modules/ProjectModule/ProjectData.h"
#include "Model/ControlProperties/ValueProperty.h"
#include "Utils/QtDavaConvertion.h"

#include <TArc/Utils/Utils.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/PropertiesHolder.h>
#include <QtTools/FileDialogs/FileDialog.h>
#include <QtTools/FMODSoundBrowser/FMODSoundBrowser.h>

#include <Base/Any.h>
#include <Engine/Engine.h>
#include <FileSystem/FileSystem.h>

#include <QAction>
#include <QLineEdit>
#include <QApplication>
#include <QMap>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QFileDialog>

ModernPropertyFMODEventEditor::ModernPropertyFMODEventEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property)
    : ModernPropertyDefaultEditor(context, property)
{
    using namespace DAVA;

    SoundSystem::Instance()->GetAllEventsNames(eventNames);

    line = new QLineEdit(GetParentWidget());
    line->setProperty("property", true);
    QObject::connect(line, &QLineEdit::editingFinished, this, &ModernPropertyFMODEventEditor::OnEditingFinished);
    QObject::connect(line, &QLineEdit::textChanged, this, &ModernPropertyFMODEventEditor::OnTextChanged);

    selectButton = new QToolButton(GetParentWidget());
    selectButton->setProperty("property", true);
    selectButton->setText("...");
    QObject::connect(selectButton, &QToolButton::clicked, this, &ModernPropertyFMODEventEditor::OnSelectClicked);

    playButton = new QToolButton(GetParentWidget());
    playButton->setProperty("property", true);
    playButton->setIcon(QIcon(":/Icons/play.png"));
    QObject::connect(playButton, &QToolButton::clicked, this, &ModernPropertyFMODEventEditor::OnPlayClicked);

    layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(4);
    layout->addWidget(line);
    layout->addWidget(selectButton);
    layout->addWidget(playButton);

    OnPropertyChanged();
}

ModernPropertyFMODEventEditor::~ModernPropertyFMODEventEditor()
{
    delete line;
    delete selectButton;
    delete playButton;
    delete layout;
}

void ModernPropertyFMODEventEditor::AddToGrid(QGridLayout* grid, int row, int col, int colSpan)
{
    grid->addWidget(propertyName, row, col);
    grid->addLayout(layout, row, col + 1, 1, colSpan);
}

void ModernPropertyFMODEventEditor::ResetProperty()
{
    ClearLine();
    ModernPropertyDefaultEditor::ResetProperty();
}

void ModernPropertyFMODEventEditor::OnPropertyChanged()
{
    using namespace DAVA;
    ModernPropertyDefaultEditor::OnPropertyChanged();

    line->setDisabled(property->IsReadOnly());
    selectButton->setDisabled(property->IsReadOnly());
    playButton->setDisabled(property->IsReadOnly());

    QSignalBlocker blockLineSignals(line);
    line->setText(property->GetValue().Cast<QString>());

    ApplyStyleToWidget(line);
}

void ModernPropertyFMODEventEditor::OnSelectClicked()
{
    using namespace DAVA;

    FMODSoundBrowser soundBrowser(GetParentWidget());

    const QString& currentEventName = line->text();
    if (!currentEventName.isEmpty())
    {
        soundBrowser.SetCurrentEvent(currentEventName.toStdString());
    }

    if (soundBrowser.exec() == QDialog::Accepted)
    {
        const String& selectedEventName = soundBrowser.GetSelectSoundEvent();

        ChangeProperty(DAVA::FastName(selectedEventName));
    }
}

void ModernPropertyFMODEventEditor::OnPlayClicked()
{
    using namespace DAVA;

    const QString& currentEventName = line->text();
    if (!currentEventName.isEmpty() && IsSoundEventValid(currentEventName))
    {
        if (currentPlayingEvent != nullptr)
        {
            currentPlayingEvent->Stop();
        }

        RefPtr<SoundEvent> event(SoundSystem::Instance()->CreateSoundEventByID(FastName(currentEventName.toStdString()), FastName()));

        currentPlayingEvent = event;
        currentPlayingEvent->Trigger();
    }
}

void ModernPropertyFMODEventEditor::OnEditingFinished()
{
    if (line->isModified())
    {
        const QString& text = line->text();
        if (text.isEmpty() || IsSoundEventValid(text))
        {
            ChangeProperty(DAVA::FastName(line->text().toStdString()));
        }
    }
}

void ModernPropertyFMODEventEditor::OnTextChanged(const QString& text)
{
    line->setProperty("error", !IsSoundEventValid(text));
    ApplyStyleToWidget(line);
}

bool ModernPropertyFMODEventEditor::IsSoundEventValid(const QString& eventName) const
{
    return std::find(eventNames.begin(), eventNames.end(), eventName.toStdString()) != eventNames.end();
}

void ModernPropertyFMODEventEditor::ClearLine()
{
    line->setText("");
    line->setProperty("error", false);
    ApplyStyleToWidget(line);
}
