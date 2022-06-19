#pragma once

#include <QValidator>

class QuickEdDoubleValidator : public QRegExpValidator
{
    Q_OBJECT
public:
    explicit QuickEdDoubleValidator(QObject* parent = Q_NULLPTR);
};
