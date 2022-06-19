#include "Modules/ModernPropertiesModule/Editors/ModernPropertyTableEditor.h"

#include "Model/ControlProperties/ValueProperty.h"
#include "Utils/QtDavaConvertion.h"

#include <Base/Any.h>

#include <TArc/Utils/Utils.h>

#include <QLabel>
#include <QAction>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QToolButton>

ModernPropertyTableEditor::ModernPropertyTableEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property, const QStringList& header_)
    : ModernPropertyDefaultEditor(context, property)
    , header(header_)
{
    using namespace DAVA;

    table = new QTableView();
    table->setProperty("property", true);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    tableModel = new QStandardItemModel(0, header.size(), GetParentWidget());
    tableModel->setHorizontalHeaderLabels(header);

    connect(tableModel, &QStandardItemModel::dataChanged, this, &ModernPropertyTableEditor::OnDataChanged);

    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(18);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setFixedHeight(18);
    table->horizontalHeader()->setDefaultSectionSize(100);
    table->horizontalHeader()->setHighlightSections(false);

    addButton = new QToolButton(GetParentWidget());
    addButton->setProperty("property", true);
    addButton->setText("+");
    connect(addButton, &QToolButton::clicked, this, &ModernPropertyTableEditor::OnAddRow);

    removeButton = new QToolButton(GetParentWidget());
    removeButton->setProperty("property", true);
    removeButton->setText("-");
    connect(removeButton, &QToolButton::clicked, this, &ModernPropertyTableEditor::OnRemoveRow);

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(propertyName);
    hLayout->addStretch();
    hLayout->addWidget(addButton);
    hLayout->addWidget(removeButton);

    layout = new QVBoxLayout();
    layout->addLayout(hLayout);
    layout->addWidget(table);

    OnPropertyChanged();
}

ModernPropertyTableEditor::~ModernPropertyTableEditor()
{
    delete table;
}

void ModernPropertyTableEditor::AddToGrid(QGridLayout* grid, int row, int col, int colSpan)
{
    grid->addLayout(layout, row, col, 1, colSpan);
}

void ModernPropertyTableEditor::OnPropertyChanged()
{
    ModernPropertyDefaultEditor::OnPropertyChanged();

    {
        QSignalBlocker signalBlocker(tableModel);
        table->setModel(nullptr);

        QString stringValue = property->GetValue().Cast<QString>();

        QStringList rows = stringValue.split(";");

        if (rows.size() < tableModel->rowCount())
        {
            tableModel->removeRows(rows.size(), tableModel->rowCount() - rows.size());
        }
        else
        {
            for (int rowNumber = tableModel->rowCount(); rowNumber < rows.size(); rowNumber++)
            {
                QList<QStandardItem*> items;
                for (int i = 0; i < header.size(); i++)
                {
                    QStandardItem* item = new QStandardItem();
                    item->setData("");
                    items.push_back(item);
                }
                tableModel->appendRow(items);
            }
        }

        for (int rowNumber = 0; rowNumber < rows.size(); rowNumber++)
        {
            QStringList row = rows[rowNumber].split(",");
            for (int i = 0; i < header.size(); i++)
            {
                QString str = "";
                if (row.size() > i)
                {
                    str = row[i].trimmed();
                }

                tableModel->item(rowNumber, i)->setData(str, Qt::DisplayRole);
            }
        }
    }

    table->setModel(tableModel);
    table->setFixedHeight(table->verticalHeader()->length() + table->horizontalHeader()->height() + 2);
    table->setDisabled(property->IsReadOnly());

    ApplyStyleToWidget(table);
}

void ModernPropertyTableEditor::OnDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    ChangeDataFromTable();
}

void ModernPropertyTableEditor::ChangeDataFromTable()
{
    QString values;
    for (int row = 0; row < tableModel->rowCount(); row++)
    {
        if (row > 0)
        {
            values.append(";");
        }

        for (int col = 0; col < header.size(); col++)
        {
            if (col > 0)
            {
                values.append(",");
            }
            values.append(tableModel->data(tableModel->index(row, col), Qt::DisplayRole).toString());
        }
    }

    ChangeProperty(values.toStdString());
}

void ModernPropertyTableEditor::OnAddRow()
{
    QList<QStandardItem*> items;
    for (int i = 0; i < header.size(); i++)
    {
        QStandardItem* item = new QStandardItem();
        item->setData("");
        items.push_back(item);
    }
    tableModel->appendRow(items);
    ChangeDataFromTable();
}

void ModernPropertyTableEditor::OnRemoveRow()
{
    QModelIndex current = table->selectionModel()->currentIndex();
    if (current.isValid())
    {
        tableModel->removeRow(current.row());
    }
    ChangeDataFromTable();
}
