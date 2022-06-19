#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Utils/QtDelayedExecutor.h"

#include <Reflection/Reflection.h>

class QAction;
class QMenu;

namespace DAVA
{
class QtFlowLayout;

class FilterWidget;
class ContentFilter : public ControlProxyImpl<QWidget>
{
    using TBase = ControlProxyImpl<QWidget>;

public:
    struct FilterFieldNames
    {
        FastName enabledRole;
        FastName inversedRole;
        FastName nameRole;
    };

    struct AvailableFilterBase
    {
        virtual ~AvailableFilterBase() = default;
    };

    struct AvailableFilter final : AvailableFilterBase
    {
        FastName filterName;
        Any key;
        bool userDefined = false;

        bool operator==(const AvailableFilter& other) const
        {
            return filterName == other.filterName &&
            key == other.key &&
            userDefined == other.userDefined;
        }
    };

    struct SeparatorTag final : AvailableFilterBase
    {
        bool operator==(const SeparatorTag& other) const
        {
            return true;
        }
    };

    struct AvailableFiltersGroup final : AvailableFilterBase
    {
        FastName name;
        Vector<std::unique_ptr<AvailableFilterBase>> filters;

        bool operator==(const AvailableFiltersGroup& other) const
        {
            return name == other.name &&
            filters == other.filters;
        }
    };

    enum class Fields : uint32
    {
        FilterText, // String
        ImmediateText, // Method<void(String)>
        Enabled, // bool
        SingleFilterDescriptor, // FilterFieldNames

        FiltersChain, // Collection<Filter>
        AddFilterToChain, // Method<void(const Any& AvailableFilter::key)>
        RemoveFilterFromChain, // Method<void(FiltersChain[n].key)>

        AvailableFilters, // Method<AvailableFilterBase* ()>
        SaveCurrentFiltersChain, // Method<void(QString filterName)>
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    ContentFilter(const Params& params, Reflection model, QWidget* parent = nullptr);

protected:
    void SetupControl();
    void UpdateControl(const ControlDescriptor& descriptor) override;
    void RebuildFiltersWidgets();

    String GetImmediateText() const;
    void SetImmediateText(const String& immText);

    String GetText() const;
    void SetText(const String& text);

    bool IsEnabled() const;
    bool IsSaveButtonEnabled() const;
    bool HasFilters() const;

    void AddFilterMenuAboutToShow();
    void AddFilterMenuAboutToHide();
    void AddFilterTriggered(QAction* triggeredAction);

    void SaveFiltersChain();

    void OnUpdateFilterWidgets();
    void OnRemoveFilterFromChain(const Any& filterKey, ControlProxy* filterControl);
    void OnRemoveAvailableFilter(const Any& filterKey);
    void OnRenameAvailableFilter(const Any& filterKey);

    void TearDown() override;

    void ClearFilterChain();
    void DisableAllFilters();

private:
    QtConnections connections;
    QtDelayedExecutor executor;
    QMenu* buttonMenu = nullptr;
    QtFlowLayout* filtersLayout = nullptr;
    Set<FilterWidget*> filterWidgets;
    DataWrappersProcessor processor;

    DAVA_REFLECTION(ContentFilter);
};

template <>
bool AnyCompare<ContentFilter::AvailableFilter>::IsEqual(const Any& v1, const Any& v2);
extern template struct AnyCompare<ContentFilter::AvailableFilter>;

template <>
bool AnyCompare<ContentFilter::SeparatorTag>::IsEqual(const Any& v1, const Any& v2);
extern template struct AnyCompare<ContentFilter::SeparatorTag>;

template <>
bool AnyCompare<ContentFilter::AvailableFiltersGroup>::IsEqual(const Any& v1, const Any& v2);
extern template struct AnyCompare<ContentFilter::AvailableFiltersGroup>;

} // namespace DAVA
