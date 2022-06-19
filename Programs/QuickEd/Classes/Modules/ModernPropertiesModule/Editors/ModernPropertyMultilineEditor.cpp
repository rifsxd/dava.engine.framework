#include "Modules/ModernPropertiesModule/Editors/ModernPropertyMultilineEditor.h"

#include "Model/ControlProperties/ValueProperty.h"
#include "Utils/QtDavaConvertion.h"

#include <Base/Any.h>

#include <TArc/Utils/Utils.h>

#include <QApplication>
#include <QKeyEvent>
#include <QPlainTextEdit>

ModernPropertyMultilineEditor::ModernPropertyMultilineEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property)
    : ModernPropertyDefaultEditor(context, property)
{
    using namespace DAVA;

    text = new QPlainTextEdit();
    text->setProperty("property", true);
    text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    text->setFixedHeight(100);
    text->installEventFilter(this);

    OnPropertyChanged();
}

ModernPropertyMultilineEditor::~ModernPropertyMultilineEditor()
{
    delete text;
}

void ModernPropertyMultilineEditor::AddToGrid(QGridLayout* layout, int row, int col, int colSpan)
{
    layout->addWidget(propertyName, row, col);
    layout->addWidget(text, row, col + 1, 1, colSpan);
}

void ModernPropertyMultilineEditor::OnPropertyChanged()
{
    ModernPropertyDefaultEditor::OnPropertyChanged();

    QSignalBlocker blockSignals(text);

    QString stringValue = StringToQString(property->GetValue().Cast<DAVA::String>());
    if (stringValue != text->toPlainText())
    {
        text->setPlainText(stringValue);
    }
    text->setDisabled(property->IsReadOnly());

    ApplyStyleToWidget(text);
}

void ModernPropertyMultilineEditor::OnTextChanged()
{
    QString stringValue = text->toPlainText();
    ChangeProperty(DAVA::Any(QStringToString(stringValue)));
}

bool ModernPropertyMultilineEditor::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();
        Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
        if (key == Qt::Key_Enter || key == Qt::Key_Return)
        {
            if (modifiers & (Qt::CTRL | Qt::ALT))
            {
                QPlainTextEdit* textEdit = qobject_cast<QPlainTextEdit*>(obj);
                if (nullptr != textEdit)
                {
                    //just appending a text do not clear the selection
                    QKeyEvent newKeyEvent(keyEvent->type(),
                                          keyEvent->key(),
                                          //this event will be handeled here, so we can not send an empty modifiers
                                          Qt::KeyboardModifiers(Qt::SHIFT),
                                          keyEvent->text(),
                                          keyEvent->isAutoRepeat(),
                                          keyEvent->count());
                    qApp->sendEvent(textEdit, &newKeyEvent);
                    return true;
                }
            }
            else if (!(modifiers & Qt::SHIFT))
            {
                OnTextChanged();
                return true;
            }
        }
    }
    else if (event->type() == QEvent::FocusOut)
    {
        OnTextChanged();
        return true;
    }
    return false;
}
