#include "Modules/ModernPropertiesModule/Editors/ModernPropertyEnumEditor.h"

#include "Model/ControlProperties/ValueProperty.h"
#include "Utils/QtDavaConvertion.h"

#include <Base/Any.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QAction>

ModernPropertyEnumEditor::ModernPropertyEnumEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property)
    : ModernPropertyDefaultEditor(context, property)
{
    using namespace DAVA;

    comboBox = new QComboBox(GetParentWidget());
    comboBox->setProperty("property", true);
    comboBox->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred));
    comboBox->setFocusPolicy(Qt::StrongFocus);
    comboBox->installEventFilter(this);

    const EnumMap* enumMap = property->GetEnumMap();
    DVASSERT(enumMap);

    for (size_t i = 0; i < enumMap->GetCount(); ++i)
    {
        int value = 0;
        if (enumMap->GetValue(i, value))
        {
            QVariant variantValue;
            variantValue.setValue<DAVA::Any>(DAVA::Any(value));
            comboBox->addItem(QObject::tr(enumMap->ToString(value)), variantValue);
        }
    }

    QObject::connect(comboBox, static_cast<void (QComboBox::*)(int index)>(&QComboBox::currentIndexChanged),
                     this, &ModernPropertyEnumEditor::OnCurrentIndexChanged, Qt::QueuedConnection);

    OnPropertyChanged();
}

ModernPropertyEnumEditor::~ModernPropertyEnumEditor()
{
    delete comboBox;
}

void ModernPropertyEnumEditor::AddToGrid(QGridLayout* layout, int row, int col, int colSpan)
{
    layout->addWidget(propertyName, row, col);
    layout->addWidget(comboBox, row, col + 1, 1, colSpan);
}

void ModernPropertyEnumEditor::OnPropertyChanged()
{
    ModernPropertyDefaultEditor::OnPropertyChanged();

    comboBox->setDisabled(property->IsReadOnly());

    QSignalBlocker blockSignals(comboBox);

    const EnumMap* enumMap = property->GetEnumMap();
    DVASSERT(enumMap);

    int index = comboBox->findText(QObject::tr(enumMap->ToString(property->GetValue().Cast<DAVA::int32>())));
    comboBox->setCurrentIndex(index);

    ApplyStyleToWidget(comboBox);
}

void ModernPropertyEnumEditor::OnCurrentIndexChanged(int index)
{
    QVariant data = comboBox->itemData(index);
    ChangeProperty(data.value<DAVA::Any>());
}
