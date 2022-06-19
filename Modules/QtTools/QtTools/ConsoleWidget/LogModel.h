#pragma once

#include <TArc/Qt/QtByteArray.h>
#include <TArc/Qt/QtSize.h>

#include <Logger/Logger.h>
#include <functional>

#include <QObject>
#include <QAbstractListModel>
#include <QPixmap>
#include <QMutex>

class QTimer;

class LogModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles
    {
        LEVEL_ROLE = Qt::UserRole,
        INTERNAL_DATA_ROLE
    };
    using ConvertFunc = std::function<DAVA::String(const DAVA::String&)>;

    explicit LogModel(QObject* parent = nullptr);
    ~LogModel() = default;
    void SetConvertFunction(ConvertFunc func); //provide mechanism to convert data string to string to be displayed

    const QPixmap& GetIcon(int ll) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    void AddMessage(DAVA::Logger::eLogLevel ll, const QByteArray& text);
    void AddMessageAsync(DAVA::Logger::eLogLevel ll, const QByteArray& msg);

public slots:
    void Clear();

private slots:
    void Sync();

private:
    void CreateIcons();
    void RecalculateRowWidth(const QString& text);
    struct LogItem
    {
        LogItem(DAVA::Logger::eLogLevel ll_ = DAVA::Logger::LEVEL_FRAMEWORK, const QString& text_ = QString(), const QString& data_ = QString());
        DAVA::Logger::eLogLevel ll;
        QString text;
        QString data;
    };
    QVector<LogItem> items;

    QVector<QPixmap> icons;
    ConvertFunc func;

    QVector<LogItem> itemsToAdd;
    QMutex mutex;
    QTimer* syncTimer = nullptr;
    QSize rowSize;
};
