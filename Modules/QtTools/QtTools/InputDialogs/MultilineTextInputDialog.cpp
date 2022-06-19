#include "QtTools/InputDialogs/MultilineTextInputDialog.h"

#include <Reflection/ReflectionRegistrator.h>

#include <QEvent>
#include <QShowEvent>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QApplication>

bool MultilineTextInputDialog::eventFilter(QObject* obj, QEvent* event)
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
                accept();
                return true;
            }
        }
    }
    return false;
}

QString MultilineTextInputDialog::GetMultiLineText(DAVA::UI* ui, const QString& title, const QString& label, const QString& text /*= QString()*/, bool* ok /*= Q_NULLPTR*/, Qt::WindowFlags flags /*= Qt::WindowFlags()*/, Qt::InputMethodHints inputMethodHints /*= Qt::ImhNone*/)
{
    MultilineTextInputDialog dialog(nullptr, flags);

    dialog.setOptions(QInputDialog::UsePlainTextEditForTextInput);
    dialog.setWindowTitle(title);
    dialog.setLabelText(label);
    dialog.setTextValue(text);
    dialog.setInputMethodHints(inputMethodHints);

    for (QObject* child : dialog.children())
    {
        child->installEventFilter(&dialog);
    }

    int ret = ui->ShowModalDialog(DAVA::mainWindowKey, &dialog);

    bool isAccepted = (ret == QDialog::Accepted);
    if (ok)
        *ok = isAccepted;
    if (isAccepted)
    {
        return dialog.textValue();
    }
    else
    {
        return QString();
    }
}

MultilineTextInputDialog::MultilineTextInputDialog(QWidget* parent /*= nullptr*/, Qt::WindowFlags flags /*= Qt::WindowFlags()*/)
    : QInputDialog(parent, flags)
{
}
