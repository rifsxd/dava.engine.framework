#include "Modules/ModernPropertiesModule/Editors/ModernPropertyVectorEditor.h"

#include "Modules/ModernPropertiesModule/Editors/QuickEdDoubleValidator.h"

#include "Model/ControlProperties/ValueProperty.h"
#include <Base/Any.h>
#include <Math/Vector.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QLineEdit>
#include <QTimer>

const char* ModernPropertyVectorEditor::COMPONENT_NAMES[COMPONENTS_MAX] =
{
  "X", "Y", "Z", "W"
};

ModernPropertyVectorEditor::ModernPropertyVectorEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property, eComponents components_)
    : ModernPropertyDefaultEditor(context, property)
    , components(components_)
{
    using namespace DAVA;

    DVASSERT(components == COMPONENTS_XY ||
             components == COMPONENTS_XYZ ||
             components == COMPONENTS_XYZW);

    layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(4);

    for (int32 i = 0; i < components; i++)
    {
        editors[i].caption = new QLabel(COMPONENT_NAMES[i], GetParentWidget());
        editors[i].line = new QLineEdit(GetParentWidget());
        editors[i].line->setProperty("property", true);
        editors[i].line->setValidator(new QuickEdDoubleValidator());
        QObject::connect(editors[i].line, &QLineEdit::editingFinished, this, &ModernPropertyVectorEditor::OnEditingFinished);

        layout->addWidget(editors[i].caption);
        layout->addWidget(editors[i].line);
    }

    OnPropertyChanged();
}

ModernPropertyVectorEditor::~ModernPropertyVectorEditor()
{
    for (DAVA::int32 i = 0; i < components; i++)
    {
        delete editors[i].caption;
        delete editors[i].line;
    }
    delete layout;
}

void ModernPropertyVectorEditor::AddToGrid(QGridLayout* grid, int row, int col, int colSpan)
{
    grid->addWidget(propertyName, row, col);
    grid->addLayout(layout, row, col + 1, 1, colSpan);
}

void ModernPropertyVectorEditor::OnPropertyChanged()
{
    ModernPropertyDefaultEditor::OnPropertyChanged();

    for (DAVA::int32 i = 0; i < components; i++)
    {
        editors[i].caption->setDisabled(property->IsReadOnly());
        editors[i].line->setDisabled(property->IsReadOnly());
    }

    if (!waitTimer)
    {
        waitTimer = true;
        QTimer::singleShot(100, this, &ModernPropertyVectorEditor::ApplyChangedProperties);
    }
}

void ModernPropertyVectorEditor::OnEditingFinished()
{
    bool modified = false;
    for (DAVA::int32 i = 0; i < components; i++)
    {
        if (editors[i].line->isModified())
        {
            modified = true;
            break;
        }
    }

    if (modified)
    {
        DAVA::float32 data[COMPONENTS_MAX];

        for (DAVA::int32 i = 0; i < components; i++)
        {
            DAVA::String str = editors[i].line->text().replace(",", ".").toStdString();
            data[i] = static_cast<DAVA::float32>(std::atof(str.c_str()));
            editors[i].line->setModified(false);
        }

        switch (components)
        {
        case COMPONENTS_XY:
            ChangeProperty(DAVA::Vector2(data));
            break;
        case COMPONENTS_XYZ:
            ChangeProperty(DAVA::Vector3(data));
            break;
        case COMPONENTS_XYZW:
            ChangeProperty(DAVA::Vector4(data));
            break;
        default:
            DVASSERT(false);
        }
    }
}

void ModernPropertyVectorEditor::ApplyChangedProperties()
{
    using namespace DAVA;

    bool modified = false;
    for (int32 i = 0; i < components; i++)
    {
        if (editors[i].line->isModified())
        {
            modified = true;
            break;
        }
    }

    if (!modified)
    {
        Vector4 r;
        switch (components)
        {
        case COMPONENTS_XY:
            r = Vector3(property->GetValue().Cast<DAVA::Vector2>(), 0.0f);
            break;
        case COMPONENTS_XYZ:
            r = property->GetValue().Cast<DAVA::Vector3>();
            break;
        case COMPONENTS_XYZW:
            r = property->GetValue().Cast<DAVA::Vector4>();
            break;
        default:
            DVASSERT(false);
        }

        for (DAVA::int32 i = 0; i < components; i++)
        {
            QSignalBlocker blockSignals(editors[i].line);

            QString newText = QString::asprintf("%g", r.data[i]);
            if (editors[i].line->text() != newText)
            {
                editors[i].line->setText(newText);
            }
            ApplyStyleToWidget(editors[i].caption);
            ApplyStyleToWidget(editors[i].line);
        }
    }

    waitTimer = false;
}
