#include "QuickEdDoubleValidator.h"

QuickEdDoubleValidator::QuickEdDoubleValidator(QObject* parent)
    : QRegExpValidator(QRegExp("[+-]?\\d*[\\.,]?\\d+"), parent)
{
}
