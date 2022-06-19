#include "Classes/Modules/PackageModule/Private/UIViewerDialog.h"

#include "Classes/Modules/ProjectModule/ProjectData.h"

#include <TArc/Controls/CheckBox.h>
#include <TArc/Controls/ComboBox.h>
#include <TArc/Controls/Label.h>
#include <TArc/Controls/QtBoxLayouts.h>
#include <TArc/DataProcessing/PropertiesHolder.h>
#include <TArc/Qt/QtString.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>

#include <Reflection/ReflectedType.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Reflection/Reflection.h>

#include <Debug/DVAssert.h>

#include <QPushButton>

namespace UIViewerDialogDetails
{
const DAVA::String PROPERTIES_KEY = "UIViewerDialog_properties";
const DAVA::String GEOMETRY_KEY = "UIViewerDialog_geometry";
}

struct UIViewerDialog::RunData : public DAVA::ReflectionBase
{
    DAVA::int32 selectedDeviceIndex = 0;
    DAVA::Vector<DAVA::String> allDevices;

    DAVA::int32 selectedBlankIndex = 0;
    DAVA::Vector<DAVA::String> allBlanks;

    bool flowFlag = false;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(RunData, DAVA::ReflectionBase)
    {
        DAVA::ReflectionRegistrator<RunData>::Begin()
        .Field("selectedDeviceIndex", &RunData::selectedDeviceIndex)
        .Field("devicesEnumerator", &RunData::allDevices)
        .Field("selectedBlankIndex", &RunData::selectedBlankIndex)
        .Field("blanksEnumerator", &RunData::allBlanks)
        .Field("flowFlag", &RunData::flowFlag)
        .End();
    }
};

UIViewerDialog::UIViewerDialog(DAVA::ContextAccessor* accessor_, DAVA::UI* ui, QWidget* parent)
    : QDialog(parent)
    , accessor(accessor_)
    , runData(new RunData())

{
    using namespace DAVA;

    ProjectData* projectData = accessor->GetGlobalContext()->GetData<ProjectData>();
    DVASSERT(nullptr != projectData);

    { //init run data
        const DAVA::Vector<ProjectData::Device>& devices = projectData->GetDevices();
        for (const ProjectData::Device& d : devices)
        {
            runData->allDevices.push_back(d.params.at(FastName("name")).Get<String>());
        }

        const DAVA::Vector<ProjectData::Blank>& blanks = projectData->GetBlanks();
        for (const ProjectData::Blank& b : blanks)
        {
            runData->allBlanks.push_back(b.name);
        }
    }

    Reflection reflectedModel = Reflection::Create(runData.get());
    QVBoxLayout* boxLayout = new QVBoxLayout();

    {
        QtHBoxLayout* lineLayout = new QtHBoxLayout();
        lineLayout->addWidget(new QLabel("Select Device: ", this));

        ComboBox::Params params(accessor, ui, DAVA::mainWindowKey);
        params.fields[ComboBox::Fields::Value] = "selectedDeviceIndex";
        params.fields[ComboBox::Fields::Enumerator] = "devicesEnumerator";
        lineLayout->AddControl(new ComboBox(params, accessor, reflectedModel));
        boxLayout->addLayout(lineLayout);
    }

    {
        QtHBoxLayout* lineLayout = new QtHBoxLayout();
        lineLayout->addWidget(new QLabel("Select Blank: ", this));

        ComboBox::Params params(accessor, ui, DAVA::mainWindowKey);
        params.fields[ComboBox::Fields::Value] = "selectedBlankIndex";
        params.fields[ComboBox::Fields::Enumerator] = "blanksEnumerator";
        lineLayout->AddControl(new ComboBox(params, accessor, reflectedModel));
        boxLayout->addLayout(lineLayout);
    }

    {
        QtHBoxLayout* lineLayout = new QtHBoxLayout();
        lineLayout->addWidget(new QLabel("Load as UI flow: ", this));

        CheckBox::Params params(accessor, ui, DAVA::mainWindowKey);
        params.fields[CheckBox::Fields::Checked] = "flowFlag";
        lineLayout->AddControl(new CheckBox(params, accessor, reflectedModel));
        boxLayout->addLayout(lineLayout);
    }

    {
        QPushButton* btnOk = new QPushButton(QStringLiteral("Run UIPreview"), this);
        btnOk->setObjectName(QStringLiteral("btnOk"));
        boxLayout->addWidget(btnOk);

        connect(btnOk, &QPushButton::clicked, this, &UIViewerDialog::accept);
    }

    setLayout(boxLayout);

    DAVA::PropertiesItem propsItem = accessor->CreatePropertiesNode(UIViewerDialogDetails::PROPERTIES_KEY);
    {
        QRect loadedGeometry = propsItem.Get<QRect>(UIViewerDialogDetails::GEOMETRY_KEY);
        if (loadedGeometry.isValid())
        {
            setGeometry(loadedGeometry);
            move(loadedGeometry.topLeft());
        }
    }
}

UIViewerDialog::~UIViewerDialog()
{
    DVASSERT(accessor != nullptr);

    DAVA::PropertiesItem propsItem = accessor->CreatePropertiesNode(UIViewerDialogDetails::PROPERTIES_KEY);
    {
        propsItem.Set(UIViewerDialogDetails::GEOMETRY_KEY, DAVA::Any(geometry()));
    }
}

void UIViewerDialog::SetDeviceIndex(DAVA::int32 index)
{
    runData->selectedDeviceIndex = index;
}

DAVA::int32 UIViewerDialog::GetDeviceIndex() const
{
    return runData->selectedDeviceIndex;
}

void UIViewerDialog::SetBlankIndex(DAVA::int32 index)
{
    runData->selectedBlankIndex = index;
}

DAVA::int32 UIViewerDialog::GetBlankIndex() const
{
    return runData->selectedBlankIndex;
}

void UIViewerDialog::SetFlowFlag(bool value)
{
    runData->flowFlag = value;
}

bool UIViewerDialog::GetFlowFlag() const
{
    return runData->flowFlag;
}
