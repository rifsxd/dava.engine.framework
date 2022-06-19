#pragma once

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyDefaultEditor.h"

#include <Sound/SoundEvent.h>

class QLineEdit;
class QToolButton;
class QHBoxLayout;

class ModernPropertyFMODEventEditor : public ModernPropertyDefaultEditor
{
    Q_OBJECT

public:
    ModernPropertyFMODEventEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property);
    ~ModernPropertyFMODEventEditor() override;

    void AddToGrid(QGridLayout* layout, int row, int col, int colSpan) override;

protected:
    void ResetProperty() override;
    void OnPropertyChanged() override;

private:
    void OnSelectClicked();
    void OnPlayClicked();
    void OnEditingFinished();
    void OnTextChanged(const QString& text);

    bool IsSoundEventValid(const QString& eventName) const;
    void ClearLine();

    QLineEdit* line = nullptr;
    QToolButton* selectButton = nullptr;
    QToolButton* playButton = nullptr;

    QList<QString> resourceExtensions;
    QString projectResourceDir;
    QString resourceSubDir;
    bool allowAnyExtension = false;

    QHBoxLayout* layout = nullptr;

    DAVA::Vector<DAVA::String> eventNames;
    DAVA::RefPtr<DAVA::SoundEvent> currentPlayingEvent;
};
