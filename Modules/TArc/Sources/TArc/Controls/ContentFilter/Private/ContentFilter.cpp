#include "TArc/Controls/ContentFilter/ContentFilter.h"
#include "TArc/Controls/ContentFilter/Private/FilterWidget.h"
#include "TArc/Controls/LineEdit.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Controls/QtFlowLayout.h"
#include "TArc/Controls/ReflectedButton.h"
#include "TArc/Controls/ControlProxy.h"
#include "TArc/Controls/Widget.h"
#include "TArc/Controls/Label.h"
#include "TArc/Qt/QtSize.h"
#include "TArc/Qt/QtString.h"
#include "TArc/DataProcessing/AnyQMetaType.h"
#include "TArc/Utils/Utils.h"

#include <Engine/PlatformApiQt.h>
#include <Reflection/ReflectionRegistrator.h>

#include <QMenu>
#include <QPainter>
#include <QInputDialog>
#include <QWidgetAction>
#include <QDebug>
#include <QStyleOptionFrame>

namespace DAVA
{
class FiltersFrame : public QWidget
{
public:
    FiltersFrame(QWidget* p = nullptr)
        : QWidget(p)
    {
    }

protected:
    void paintEvent(QPaintEvent* e) override
    {
        QPainter p(this);

        QStyleOptionFrame panel;
        panel.initFrom(this);
        panel.rect = contentsRect();
        panel.lineWidth = 0;
        panel.midLineWidth = 0;
        panel.state |= QStyle::State_Sunken;
        panel.features = QStyleOptionFrame::None;
        style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &p, this);
    }
};

class MenuItemWidget : public Widget
{
    using TBase = Widget;

public:
    explicit MenuItemWidget(const QString& text_, QWidget* parent = nullptr)
        : TBase(parent)
        , text(text_)
    {
        setMouseTracking(true);
    }

protected:
    void paintEvent(QPaintEvent* e) override
    {
        QPainter p(this);
        QStyleOptionMenuItem opt;
        opt.initFrom(this);
        opt.palette = palette();
        opt.state = QStyle::State_None;

        if (window()->isActiveWindow())
        {
            opt.state |= QStyle::State_Active;
        }
        if (isEnabled())
        {
            opt.state |= QStyle::State_Enabled;
        }

        QPoint cursorPos = QCursor::pos();
        cursorPos = mapFromGlobal(cursorPos);
        if (rect().contains(cursorPos) == true)
        {
            bool selected = true;
            QList<QWidget*> childrenList = findChildren<QWidget*>();
            foreach (QWidget* child, childrenList)
            {
                QPoint childPos = child->mapFrom(this, cursorPos);
                if (child->rect().contains(childPos))
                {
                    selected = false;
                    break;
                }
            }

            if (selected == true)
            {
                opt.state |= QStyle::State_Selected;
            }
        }

        opt.font = parentWidget()->font();
        opt.fontMetrics = QFontMetrics(opt.font);
        opt.menuHasCheckableItems = false;
        opt.checkType = QStyleOptionMenuItem::NotCheckable;
        opt.menuItemType = QStyleOptionMenuItem::Normal;
        opt.text = text;
        opt.tabWidth = 0;
        opt.maxIconWidth = maxIconWidth;
        opt.menuRect = parentWidget()->rect();
        opt.rect = rect();
        QRegion clipRegion(opt.rect);
        p.setClipRegion(clipRegion);
        style()->drawControl(QStyle::CE_MenuItem, &opt, &p, this);
    }

    bool eventFilter(QObject* obj, QEvent* e) override
    {
        if (e->type() == QEvent::Enter || e->type() == QEvent::Leave)
        {
            update();
        }

        return false;
    }

    void mouseMoveEvent(QMouseEvent* e) override
    {
        update();
    }

    void leaveEvent(QEvent* e) override
    {
        update();
    }

    QSize minimumSizeHint() const override
    {
        QSize s = TBase::minimumSizeHint();
        QFontMetrics metrics(parentWidget()->font());
        int32 w = metrics.boundingRect(text).width();
        int rightMargin = 10;
        return s + QSize(w + maxIconWidth + rightMargin, 0);
    }

    const int32 maxIconWidth = 20;
    QString text;
    bool underMouse = false;
};

ContentFilter::ContentFilter(const Params& params, Reflection model, QWidget* parent)
    : TBase(params, ControlDescriptor(params.fields), params.accessor, model, parent)
{
    SetupControl();
}

void ContentFilter::SetupControl()
{
    Reflection thisModel = Reflection::Create(ReflectedObject(this));
    QtVBoxLayout* mainLayout = new QtVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(2);

    {
        QtHBoxLayout* firstRowLayout = new QtHBoxLayout();
        firstRowLayout->setMargin(0);
        firstRowLayout->setSpacing(2);
        LineEdit::Params lineEditParams(controlParams.accessor, controlParams.ui, controlParams.wndKey);
        lineEditParams.fields[LineEdit::Fields::Clearable].BindConstValue(true);
        lineEditParams.fields[LineEdit::Fields::ImmediateText] = "immediateText";
        lineEditParams.fields[LineEdit::Fields::IsEnabled] = "isEnabled";
        lineEditParams.fields[LineEdit::Fields::PlaceHolder].BindConstValue(String("Type here to search"));
        lineEditParams.fields[LineEdit::Fields::Text] = "filterText";

        firstRowLayout->AddControl(new LineEdit(lineEditParams, controlParams.accessor, thisModel, this));

        QToolButton* toolButton = new QToolButton(this);
        toolButton->setFixedSize(24, 24);
        toolButton->setIcon(DAVA::SharedIcon(":/TArc/Resources/filter.png"));
        toolButton->setPopupMode(QToolButton::InstantPopup);
        toolButton->setAutoRaise(false);
        toolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);

        buttonMenu = new QMenu(toolButton);
        connections.AddConnection(buttonMenu, &QMenu::aboutToShow, MakeFunction(this, &ContentFilter::AddFilterMenuAboutToShow));
        connections.AddConnection(buttonMenu, &QMenu::aboutToHide, MakeFunction(this, &ContentFilter::AddFilterMenuAboutToHide));
        connections.AddConnection(buttonMenu, &QMenu::triggered, MakeFunction(this, &ContentFilter::AddFilterTriggered));
        toolButton->setMenu(buttonMenu);

        firstRowLayout->addWidget(toolButton);
        mainLayout->addLayout(firstRowLayout);
    }

    {
        QtHBoxLayout* secondRowLayout = new QtHBoxLayout();
        secondRowLayout->setMargin(0);
        secondRowLayout->setSpacing(2);

        FiltersFrame* filtersFrame = new FiltersFrame(this);
        filtersFrame->setVisible(false);
        filtersFrame->setObjectName("filtersFrame");
        filtersFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        filtersLayout = new QtFlowLayout(2, 1, 1);
        filtersFrame->setLayout(filtersLayout);

        secondRowLayout->addWidget(filtersFrame);

        ReflectedButton::Params params(controlParams.accessor, controlParams.ui, controlParams.wndKey);
        params.fields[ReflectedButton::Fields::AutoRaise].BindConstValue(false);
        params.fields[ReflectedButton::Fields::Enabled] = "isSaveButtonEnabled";
        params.fields[ReflectedButton::Fields::Icon].BindConstValue(DAVA::SharedIcon(":/TArc/Resources/save.png"));
        params.fields[ReflectedButton::Fields::Tooltip].BindConstValue(QStringLiteral("Save current filters chain"));
        params.fields[ReflectedButton::Fields::Clicked] = "saveFiltesChain";
        params.fields[ReflectedButton::Fields::Visible] = "hasFilters";

        ReflectedButton* saveFilterButton = new ReflectedButton(params, controlParams.accessor, thisModel, this);
        saveFilterButton->ToWidgetCast()->setFixedSize(QSize(24, 24));
        saveFilterButton->ToWidgetCast()->setVisible(false);
        secondRowLayout->AddControl(saveFilterButton);
        mainLayout->addLayout(secondRowLayout);
    }

    setLayout(mainLayout);
}

void ContentFilter::UpdateControl(const ControlDescriptor& descriptor)
{
    if (descriptor.IsChanged(Fields::Enabled) == true)
    {
        setEnabled(GetFieldValue<bool>(Fields::Enabled, true));
    }

    if (descriptor.IsChanged(Fields::SingleFilterDescriptor) == true ||
        descriptor.IsChanged(Fields::FiltersChain) == true)
    {
        RebuildFiltersWidgets();
        QWidget* filtersFrame = findChild<QWidget*>("filtersFrame");
        DVASSERT(filtersFrame != nullptr);
        filtersFrame->setVisible(HasFilters());
    }
}

void ContentFilter::RebuildFiltersWidgets()
{
    Vector<std::pair<FilterWidget*, QLayoutItem*>> filterItems;
    for (int i = 0; i < filtersLayout->count(); ++i)
    {
        QLayoutItem* item = filtersLayout->itemAt(i);
        QWidget* w = item->widget();
        if (w != nullptr)
        {
            FilterWidget* filterWidget = static_cast<FilterWidget*>(w);
            filterItems.emplace_back(filterWidget, item);
        }
    }

    FilterFieldNames names = GetFieldValue<FilterFieldNames>(Fields::SingleFilterDescriptor, FilterFieldNames());

    Reflection field = model.GetField(GetFieldName(Fields::FiltersChain));
    DVASSERT(field.IsValid() == true);
    Vector<Reflection::Field> filters = field.GetFields();

    FilterWidget::Params p(controlParams.accessor, controlParams.ui, controlParams.wndKey);
    p.fields[FilterWidget::Fields::Enabled] = names.enabledRole;
    p.fields[FilterWidget::Fields::Inversed] = names.inversedRole;
    p.fields[FilterWidget::Fields::Title] = names.nameRole;

    size_t widgetIndex = 0;
    for (const Reflection::Field& filter : filters)
    {
        if (widgetIndex < filterItems.size())
        {
            FilterWidget* w = filterItems[widgetIndex].first;
            w->ResetModel(filter.ref);
            w->ForceUpdate();
            widgetIndex++;
        }
        else
        {
            FilterWidget* filterWidget = new FilterWidget(p, processor, filter.ref, this);
            filterWidget->updateRequire.Connect(this, &ContentFilter::OnUpdateFilterWidgets);
            filterWidget->requestRemoving.Connect(Bind(&ContentFilter::OnRemoveFilterFromChain, this, filter.key, static_cast<ControlProxy*>(filterWidget)));
            filterWidgets.insert(filterWidget);
            filtersLayout->addWidget(filterWidget);

            QAction* deleteAllFilters = new QAction("Delete all filters", filterWidget->ToWidgetCast());
            connections.AddConnection(deleteAllFilters, &QAction::triggered, MakeFunction(this, &ContentFilter::ClearFilterChain));
            filterWidget->ToWidgetCast()->addAction(deleteAllFilters);

            QAction* disableAllFilters = new QAction("Disable all filters", filterWidget->ToWidgetCast());
            connections.AddConnection(disableAllFilters, &QAction::triggered, MakeFunction(this, &ContentFilter::DisableAllFilters));
            filterWidget->ToWidgetCast()->addAction(disableAllFilters);
        }
    }

    for (size_t i = widgetIndex; i < filterItems.size(); ++i)
    {
        FilterWidget* w = filterItems[i].first;
        w->TearDown();
        filterWidgets.erase(w);
        filtersLayout->removeItem(filterItems[i].second);
        w->ToWidgetCast()->deleteLater();
    }

    OnUpdateFilterWidgets();
}

void ContentFilter::SetImmediateText(const String& immText)
{
    FastName fieldName = GetFieldName(Fields::ImmediateText);
    if (fieldName.IsValid())
    {
        AnyFn method = model.GetMethod(fieldName.c_str());
        if (method.IsValid())
        {
            method.Invoke(immText);
        }
    }
}

String ContentFilter::GetText() const
{
    return GetFieldValue<String>(Fields::FilterText, "");
}

void ContentFilter::SetText(const String& text)
{
    FastName fieldName = GetFieldName(Fields::FilterText);
    if (fieldName.IsValid())
    {
        wrapper.SetFieldValue(fieldName, text);
    }
}

bool ContentFilter::IsEnabled() const
{
    return GetFieldValue<bool>(Fields::Enabled, true);
}

bool ContentFilter::IsSaveButtonEnabled() const
{
    return HasFilters() == true && IsEnabled() == true;
}

bool ContentFilter::HasFilters() const
{
    return filtersLayout->count() > 0;
}

void ContentFilter::AddFilterMenuAboutToShow()
{
    Function<void(const std::unique_ptr<AvailableFilterBase>&, QMenu*)> unpackMenu = [&unpackMenu, this](const std::unique_ptr<AvailableFilterBase>& filterBase, QMenu* menu) {
        AvailableFiltersGroup* group = dynamic_cast<AvailableFiltersGroup*>(filterBase.get());
        if (group != nullptr)
        {
            QMenu* subMenu = menu->addMenu(QString(group->name.c_str()));
            for (const std::unique_ptr<AvailableFilterBase>& subFilter : group->filters)
            {
                unpackMenu(subFilter, subMenu);
            }
            return;
        }

        AvailableFilter* filter = dynamic_cast<AvailableFilter*>(filterBase.get());
        if (filter != nullptr)
        {
            QString filterTitle(filter->filterName.c_str());
            if (filter->userDefined == false)
            {
                QAction* action = menu->addAction(filterTitle);
                action->setProperty("filterKey", QVariant::fromValue(filter->key));
            }
            else
            {
                Reflection thisModel = Reflection::Create(ReflectedObject(this));
                MenuItemWidget* containerWidget = new MenuItemWidget(filterTitle, menu);
                QHBoxLayout* layout = new QHBoxLayout();
                layout->setSpacing(1);
                layout->setContentsMargins(0, 0, 0, 0);
                layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
                containerWidget->SetLayout(layout);

                {
                    ReflectedButton::Params p(controlParams.accessor, controlParams.ui, controlParams.wndKey);
                    p.fields[ReflectedButton::Fields::AutoRaise].BindConstValue(false);
                    p.fields[ReflectedButton::Fields::Clicked] = "renameAvailableFilter";
                    p.fields[ReflectedButton::Fields::BindedArgument].BindConstValue(filter->key);
                    p.fields[ReflectedButton::Fields::Icon].BindConstValue(DAVA::SharedIcon(":/TArc/ContentFilter/Private/Resources/rename.png"));
                    p.fields[ReflectedButton::Fields::IconSize].BindConstValue(QSize(16, 16));
                    ReflectedButton* button = new ReflectedButton(p, p.accessor, thisModel, containerWidget->ToWidgetCast());
                    QWidget* buttonWidget = button->ToWidgetCast();
                    buttonWidget->installEventFilter(containerWidget->ToWidgetCast());
                    containerWidget->AddControl(button);
                }

                {
                    DelayedToolButton::Params p(controlParams.accessor, controlParams.ui, controlParams.wndKey);
                    p.fields[DelayedToolButton::Fields::AutoRaise].BindConstValue(false);
                    p.fields[DelayedToolButton::Fields::Clicked] = "removeAvailableFilter";
                    p.fields[DelayedToolButton::Fields::BindedArgument].BindConstValue(filter->key);
                    p.fields[DelayedToolButton::Fields::Icon].BindConstValue(DAVA::SharedIcon(":/TArc/ContentFilter/Private/Resources/delete.png"));
                    p.fields[DelayedToolButton::Fields::IconSize].BindConstValue(QSize(16, 16));
                    p.fields[DelayedToolButton::Fields::DelayMs].BindConstValue(500);
                    p.fields[DelayedToolButton::Fields::ProgressBarColor].BindConstValue(QColor(Qt::red));
                    DelayedToolButton* button = new DelayedToolButton(p, p.accessor, thisModel, containerWidget->ToWidgetCast());
                    QWidget* buttonWidget = button->ToWidgetCast();
                    buttonWidget->installEventFilter(containerWidget->ToWidgetCast());
                    containerWidget->AddControl(button);
                }

                QWidgetAction* action = new QWidgetAction(menu);
                action->setProperty("filterKey", QVariant::fromValue(filter->key));
                action->setDefaultWidget(containerWidget->ToWidgetCast());
                menu->addAction(action);
            }

            return;
        }

        if (dynamic_cast<SeparatorTag*>(filterBase.get()) != nullptr)
        {
            menu->addSeparator();
            return;
        }

        DVASSERT(false);
    };

    FastName avFiltersFieldName = GetFieldName(Fields::AvailableFilters);
    DVASSERT(avFiltersFieldName.IsValid() == true);

    AnyFn avFiltersFn = model.GetMethod(avFiltersFieldName.c_str());
    DVASSERT(avFiltersFn.IsValid() == true);
    DVASSERT(avFiltersFn.GetInvokeParams().retType == Type::Instance<AvailableFilterBase*>());

    AvailableFilterBase* base = avFiltersFn.Invoke().Get<AvailableFilterBase*>();
    AvailableFiltersGroup* group = dynamic_cast<AvailableFiltersGroup*>(base);
    DVASSERT(group != nullptr);

    for (const std::unique_ptr<AvailableFilterBase>& subBase : group->filters)
    {
        unpackMenu(subBase, buttonMenu);
    }
}

void ContentFilter::AddFilterMenuAboutToHide()
{
    executor.DelayedExecute([this]() {
        buttonMenu->clear();
    });
}

void ContentFilter::AddFilterTriggered(QAction* triggeredAction)
{
    QVariant key = triggeredAction->property("filterKey");
    DVASSERT(key.canConvert<Any>() == true);

    FastName addFilterMethodName = GetFieldName(Fields::AddFilterToChain);
    DVASSERT(addFilterMethodName.IsValid() == true);

    AnyFn addFilterFn = model.GetMethod(addFilterMethodName.c_str());
    DVASSERT(addFilterFn.IsValid() == true);
    addFilterFn.Invoke(key.value<Any>());
}

void ContentFilter::SaveFiltersChain()
{
    QString filterName = QInputDialog::getText(this, "Save filter", "Input name for current filter");
    if (filterName.isEmpty() == true)
    {
        return;
    }

    FastName saveMethodName = GetFieldName(Fields::SaveCurrentFiltersChain);
    DVASSERT(saveMethodName.IsValid() == true);

    AnyFn saveFn = model.GetMethod(saveMethodName.c_str());
    DVASSERT(saveFn.IsValid() == true);
    saveFn.Invoke(filterName);
}

void ContentFilter::OnUpdateFilterWidgets()
{
    executor.DelayedExecute([this]() {
        processor.Sync();
    });
}

void ContentFilter::OnRemoveFilterFromChain(const Any& filterKey, ControlProxy* filterControl)
{
    FastName removeMethodName = GetFieldName(Fields::RemoveFilterFromChain);
    DVASSERT(removeMethodName.IsValid() == true);

    AnyFn removeFn = model.GetMethod(removeMethodName.c_str());
    DVASSERT(removeFn.IsValid() == true);
    removeFn.Invoke(filterKey);
    ForceUpdate();
}

void ContentFilter::OnRemoveAvailableFilter(const Any& filterKey)
{
    buttonMenu->close();

    FastName getAvFiltersName = GetFieldName(Fields::AvailableFilters);
    DVASSERT(getAvFiltersName.IsValid() == true);

    AnyFn getFn = model.GetMethod(getAvFiltersName.c_str());
    DVASSERT(getFn.IsValid() == true);

    DVASSERT(getFn.GetInvokeParams().retType == Type::Instance<AvailableFilterBase*>());
    AvailableFilterBase* filterBase = getFn.Invoke().Get<AvailableFilterBase*>();

    Function<bool(AvailableFilterBase*)> scanHierarchy = [&scanHierarchy, &filterKey](AvailableFilterBase* base) -> bool {
        AvailableFilter* filter = dynamic_cast<AvailableFilter*>(base);
        if (filter != nullptr)
        {
            if (filter->key == filterKey && filter->userDefined == true)
            {
                return true;
            }

            return false;
        }

        AvailableFiltersGroup* group = dynamic_cast<AvailableFiltersGroup*>(base);
        if (group != nullptr)
        {
            for (auto iter = group->filters.begin(); iter != group->filters.end(); ++iter)
            {
                bool matched = scanHierarchy(iter->get());
                if (matched)
                {
                    group->filters.erase(iter);
                    return false;
                }
            }
        }

        return false;
    };

    scanHierarchy(filterBase);
}

void ContentFilter::OnRenameAvailableFilter(const Any& filterKey)
{
    buttonMenu->close();
    executor.DelayedExecute([this, filterKey]() {
        QString filterName = QInputDialog::getText(this, "Rename saved filter", "Input name for saved filter");
        if (filterName.isEmpty() == true)
        {
            return;
        }

        FastName getAvFiltersName = GetFieldName(Fields::AvailableFilters);
        DVASSERT(getAvFiltersName.IsValid() == true);

        AnyFn getFn = model.GetMethod(getAvFiltersName.c_str());
        DVASSERT(getFn.IsValid() == true);

        DVASSERT(getFn.GetInvokeParams().retType == Type::Instance<AvailableFilterBase*>());
        AvailableFilterBase* filterBase = getFn.Invoke().Get<AvailableFilterBase*>();

        Function<AvailableFilter*(AvailableFilterBase*)> scanHierarchy = [&scanHierarchy, &filterKey](AvailableFilterBase* base) -> AvailableFilter* {
            AvailableFilter* filter = dynamic_cast<AvailableFilter*>(base);
            if (filter != nullptr)
            {
                if (filter->key == filterKey)
                {
                    return filter;
                }

                return nullptr;
            }

            AvailableFiltersGroup* group = dynamic_cast<AvailableFiltersGroup*>(base);
            if (group != nullptr)
            {
                for (std::unique_ptr<AvailableFilterBase>& subBase : group->filters)
                {
                    AvailableFilter* filter = scanHierarchy(subBase.get());
                    if (filter != nullptr)
                    {
                        return filter;
                    }
                }
            }

            return nullptr;
        };

        AvailableFilter* filterToRename = scanHierarchy(filterBase);
        DVASSERT(filterToRename != nullptr);

        filterToRename->filterName = FastName(filterName.toStdString());
    });
}

void ContentFilter::TearDown()
{
    processor.Shoutdown();
    TBase::TearDown();
}

void ContentFilter::ClearFilterChain()
{
    for (ControlProxy* filterWidget : filterWidgets)
    {
        filterWidget->TearDown();
    }
    filterWidgets.clear();

    FastName removeMethodName = GetFieldName(Fields::RemoveFilterFromChain);
    DVASSERT(removeMethodName.IsValid() == true);

    AnyFn removeFn = model.GetMethod(removeMethodName.c_str());
    DVASSERT(removeFn.IsValid() == true);

    Reflection field = model.GetField(GetFieldName(Fields::FiltersChain));
    DVASSERT(field.IsValid() == true);
    while (field.HasFields())
    {
        Vector<Reflection::Field> filters = field.GetFields();
        removeFn.Invoke(filters.back().key);
    }
}

void ContentFilter::DisableAllFilters()
{
    FilterFieldNames names = GetFieldValue<FilterFieldNames>(Fields::SingleFilterDescriptor, FilterFieldNames());

    Reflection field = model.GetField(GetFieldName(Fields::FiltersChain));
    DVASSERT(field.IsValid() == true);
    Vector<Reflection::Field> filters = field.GetFields();

    for (const Reflection::Field& filter : filters)
    {
        filter.ref.GetField(names.enabledRole).SetValue(false);
    }

    OnUpdateFilterWidgets();
}

DAVA_REFLECTION_IMPL(ContentFilter)
{
    ReflectionRegistrator<ContentFilter>::Begin()
    .Method("immediateText", &ContentFilter::SetImmediateText)
    .Field("filterText", &ContentFilter::GetText, &ContentFilter::SetText)
    .Field("isEnabled", &ContentFilter::IsEnabled, nullptr)
    .Field("isSaveButtonEnabled", &ContentFilter::IsSaveButtonEnabled, nullptr)
    .Field("hasFilters", &ContentFilter::HasFilters, nullptr)
    .Method("saveFiltesChain", &ContentFilter::SaveFiltersChain)
    .Method("removeAvailableFilter", &ContentFilter::OnRemoveAvailableFilter)
    .Method("renameAvailableFilter", &ContentFilter::OnRenameAvailableFilter)
    .End();
}

template <>
bool AnyCompare<ContentFilter::AvailableFilter>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<ContentFilter::AvailableFilter>() == v2.Get<ContentFilter::AvailableFilter>();
}

template <>
bool AnyCompare<ContentFilter::SeparatorTag>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<ContentFilter::SeparatorTag>() == v2.Get<ContentFilter::SeparatorTag>();
}

template <>
bool AnyCompare<ContentFilter::AvailableFiltersGroup>::IsEqual(const Any& v1, const Any& v2)
{
    return v1.Get<ContentFilter::AvailableFiltersGroup>() == v2.Get<ContentFilter::AvailableFiltersGroup>();
}

} // namespace DAVA
