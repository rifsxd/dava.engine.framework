#pragma once

#include "Base/Result.h"
#include "LogModel.h"

#include <QWidget>
#include <QPointer>

class QTimer;
class LogFilterModel;
class LogModel;

namespace Ui
{
class LogWidget;
};

class LogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LogWidget(QWidget* parent = NULL);
    ~LogWidget();
    void SetConvertFunction(LogModel::ConvertFunc func); //provide mechanism to convert data string to string to be displayed
    QByteArray Serialize() const;
    void Deserialize(const QByteArray& data);

public slots:
    void AddMessage(DAVA::Logger::eLogLevel ll, const QByteArray& msg);

signals:
    void ItemClicked(const QString& data);

private slots:
    void OnCopy();
    void OnBeforeAdded();
    void UpdateScroll();
    void OnItemClicked(const QModelIndex& index);

private:
    void FillFiltersCombo();
    bool eventFilter(QObject* watched, QEvent* event) override;

    QPointer<LogModel> logModel;
    QPointer<LogFilterModel> logFilterModel;
    Ui::LogWidget* ui;
    QTimer* scrollTimer;
};
