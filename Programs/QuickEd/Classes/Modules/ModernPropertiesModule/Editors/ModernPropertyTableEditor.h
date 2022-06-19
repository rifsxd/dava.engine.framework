#pragma once

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyDefaultEditor.h"

class QTableView;
class QToolButton;
class QStandardItemModel;

class ModernPropertyTableEditor : public ModernPropertyDefaultEditor
{
    Q_OBJECT

public:
    ModernPropertyTableEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property, const QStringList& header);
    ~ModernPropertyTableEditor() override;

    void AddToGrid(QGridLayout* layout, int row, int col, int colSpan) override;

protected:
    void OnPropertyChanged() override;

private:
    void OnDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
    void ChangeDataFromTable();

    void OnAddRow();
    void OnRemoveRow();

    QTableView* table = nullptr;
    QStandardItemModel* tableModel = nullptr;
    QStringList header;
    QVBoxLayout* layout = nullptr;
    QToolButton* addButton = nullptr;
    QToolButton* removeButton = nullptr;
};
