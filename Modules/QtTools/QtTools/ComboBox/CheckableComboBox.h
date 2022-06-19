#pragma once

#include <QComboBox>
#include <QStandardItemModel>

class ComboBoxModel : public QStandardItemModel
{
    Q_OBJECT

public:
    ComboBoxModel(QObject* parent = nullptr);
    Qt::ItemFlags flags(const QModelIndex& index) const override;
};

class CheckableComboBox : public QComboBox
{
    Q_OBJECT

signals:
    void done();

public:
    explicit CheckableComboBox(QWidget* parent = nullptr);
    ~CheckableComboBox();

    QVariantList selectedUserData() const;
    void selectUserData(const QVariantList& data);
signals:
    void selectedUserDataChanged(const QVariantList& data);
private slots:
    void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
    void updateTextHints();

private:
    QModelIndexList checkedIndexes() const;

    bool eventFilter(QObject* obj, QEvent* e) override;
    void paintEvent(QPaintEvent* event) override;

    QString textHint;
};
