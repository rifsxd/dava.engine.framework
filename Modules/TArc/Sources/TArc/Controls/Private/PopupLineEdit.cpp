#include "TArc/Controls/PopupLineEdit.h"

#include <QtEvents>
#include <QVBoxLayout>

namespace DAVA
{
PopupLineEdit::PopupLineEdit(const LineEdit::Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : QWidget(parent, Qt::Popup)
{
    edit = new LineEdit(params, wrappersProcessor, model, this);
    SetupControl();
}

PopupLineEdit::PopupLineEdit(const LineEdit::Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : QWidget(parent, Qt::Popup)
{
    edit = new LineEdit(params, accessor, model, this);
    SetupControl();
}

PopupLineEdit::~PopupLineEdit()
{
    edit->TearDown();
}

void PopupLineEdit::Show(const QPoint& position)
{
    move(position);
    show();
    edit->ToWidgetCast()->setFocus(Qt::PopupFocusReason);
}

void PopupLineEdit::Show(const QRect& geometry)
{
    resize(geometry.size());
    move(geometry.topLeft());
    show();
    edit->ToWidgetCast()->setFocus(Qt::PopupFocusReason);
}

void PopupLineEdit::ForceUpdate()
{
    edit->ForceUpdate();
}

bool PopupLineEdit::eventFilter(QObject* obj, QEvent* e)
{
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
        if (keyEvent->matches(QKeySequence::Cancel))
        {
            edit->ForceUpdate();
        }
        else if (keyEvent->key() == Qt::Key_Enter ||
                 keyEvent->key() == Qt::Key_Return)
        {
            close();
        }
    }
    if (e->type() == QEvent::FocusOut)
    {
        close();
    }

    return false;
}

void PopupLineEdit::SetupControl()
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(edit->ToWidgetCast());

    edit->ToWidgetCast()->installEventFilter(this);
    installEventFilter(this);
}
} // namespace DAVA