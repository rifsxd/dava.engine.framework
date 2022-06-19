#include "Modules/RenderOptionsModule/RenderOptionsModule.h"

#include <Engine/EngineContext.h>
#include <Render/RenderOptions.h>
#include <Render/Renderer.h>

#include <TArc/Controls/CheckBox.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/TArcDataNode.h>
#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/QtAction.h>

#include <Render/RenderOptions.h>

#include <QAction>
#include <QDialog>
#include <QScrollArea>
#include <QWidget>
#include <QVBoxLayout>

namespace RenderOptionsDetails
{
namespace Properties
{
static const DAVA::String GlobalName = "RenderOptionsDialogProperties";
static const DAVA::String Geometry = "Geometry";
}

class OptionWrapper : public DAVA::ReflectionBase
{
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(OptionWrapper, DAVA::ReflectionBase)
    {
        using namespace DAVA;
        ReflectionRegistrator<OptionWrapper>::Begin()
        .Field("title", &OptionWrapper::GetTitle, nullptr)
        .Field("enabled", &OptionWrapper::GetEnabled, &OptionWrapper::SetEnabled)
        .End();
    }

public:
    OptionWrapper(DAVA::RenderOptions::RenderOption option)
        : option(option)
    {
    }

    DAVA::String GetTitle() const
    {
        return DAVA::String(DAVA::Renderer::GetOptions()->GetOptionName(option).c_str());
    }

    bool GetEnabled() const
    {
        return DAVA::Renderer::GetOptions()->IsOptionEnabled(option);
    }

    void SetEnabled(bool value)
    {
        DAVA::Renderer::GetOptions()->SetOption(option, value);
    }

private:
    DAVA::RenderOptions::RenderOption option;
};

class RenderOptionsData : public DAVA::TArcDataNode
{
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(RenderOptionsData, DAVA::TArcDataNode)
    {
        DAVA::ReflectionRegistrator<RenderOptionsData>::Begin()
        .Field("isEnabled", &RenderOptionsData::IsEnabled, nullptr)
        .End();
    }

public:
    bool IsEnabled() const
    {
        return DAVA::Renderer::IsInitialized();
    }

    DAVA::Vector<std::shared_ptr<OptionWrapper>> optionsRefs;
    std::unique_ptr<QDialog> optionsDialog;
};
}

const QString RenderOptionsModule::renderOptionsMenuItemName = QString("Render Options");

void RenderOptionsModule::PostInit()
{
    using namespace DAVA;

    std::unique_ptr<RenderOptionsDetails::RenderOptionsData> data(new RenderOptionsDetails::RenderOptionsData());
    for (uint32 option = 0; option < RenderOptions::OPTIONS_COUNT; ++option)
    {
        data->optionsRefs.emplace_back(std::make_shared<RenderOptionsDetails::OptionWrapper>(static_cast<RenderOptions::RenderOption>(option)));
    }

    ContextAccessor* accessor = GetAccessor();
    UI* ui = GetUI();

    // Menu item

    {
        QtAction* action = new QtAction(accessor, QIcon(":/Icons/render_options.png"), renderOptionsMenuItemName);

        FieldDescriptor fieldIsEnabled;
        fieldIsEnabled.type = ReflectedTypeDB::Get<RenderOptionsDetails::RenderOptionsData>();
        fieldIsEnabled.fieldName = FastName("isEnabled");
        action->SetStateUpdationFunction(QtAction::Enabled, fieldIsEnabled, [](const Any& fieldValue) -> Any {
            return fieldValue.Cast<bool>(false);
        });

        connections.AddConnection(action, &QAction::triggered, MakeFunction(this, &RenderOptionsModule::ShowRenderOptionsDialog));

        ActionPlacementInfo placementInfo;
        placementInfo.AddPlacementPoint(CreateMenuPoint(QString("Tools"), { InsertionParams::eInsertionMethod::AfterItem }));

        ui->AddAction(DAVA::mainWindowKey, placementInfo, action);
    }

    // Dialog

    {
        PropertiesItem properties = accessor->CreatePropertiesNode(RenderOptionsDetails::Properties::GlobalName);
        QRect dialogRect = properties.Get<QRect>(RenderOptionsDetails::Properties::Geometry);

        data->optionsDialog = std::make_unique<QDialog>();
        data->optionsDialog->setWindowTitle("Render Options");

        if (dialogRect.isValid())
        {
            data->optionsDialog->setGeometry(dialogRect);
            data->optionsDialog->move(dialogRect.topLeft());
        }
        else
        {
            data->optionsDialog->resize(400, 300);
        }

        QScrollArea* scrollArea = new QScrollArea();
        scrollArea->setWidgetResizable(true);
        scrollArea->setWidget(new QWidget());

        QVBoxLayout* layout = new QVBoxLayout(data->optionsDialog.get());
        layout->setMargin(5);
        layout->addWidget(scrollArea);

        QGridLayout* scrollLayout = new QGridLayout(scrollArea->widget());
        scrollLayout->setSpacing(5);

        uint32 index = 0;
        for (std::shared_ptr<RenderOptionsDetails::OptionWrapper>& option : data->optionsRefs)
        {
            CheckBox::Params params(accessor, ui, DAVA::mainWindowKey);
            params.fields[CheckBox::Fields::Checked] = "enabled";
            params.fields[CheckBox::Fields::TextHint] = "title";
            CheckBox* cb = new CheckBox(params, accessor, DAVA::Reflection::Create(DAVA::ReflectedObject(option.get())), data->optionsDialog.get());
            scrollLayout->addWidget(cb->ToWidgetCast(), index / 2, index & 1);
            index++;
        }
    }

    // Store data node

    {
        DataContext* globalContext = accessor->GetGlobalContext();
        globalContext->CreateData(std::move(data));
    }
}

void RenderOptionsModule::ShowRenderOptionsDialog()
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    if (accessor)
    {
        DataContext* globalContext = accessor->GetGlobalContext();
        RenderOptionsDetails::RenderOptionsData* data = globalContext->GetData<RenderOptionsDetails::RenderOptionsData>();
        DVASSERT(data);
        DVASSERT(data->optionsDialog);

        data->optionsDialog->exec();

        PropertiesItem properties = accessor->CreatePropertiesNode(RenderOptionsDetails::Properties::GlobalName);
        properties.Set(RenderOptionsDetails::Properties::Geometry, data->optionsDialog->geometry());
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(RenderOptionsModule)
{
    DAVA::ReflectionRegistrator<RenderOptionsModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_TARC_MODULE(RenderOptionsModule);
