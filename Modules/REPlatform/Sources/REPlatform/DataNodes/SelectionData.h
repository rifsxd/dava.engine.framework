#pragma once

#include "REPlatform/DataNodes/SelectableGroup.h"

#include <TArc/DataProcessing/TArcDataNode.h>

#include <Math/AABBox3.h>
#include <Reflection/Reflection.h>

#include <memory>

class SelectionModule;
namespace DAVA
{
class Entity;
class SelectionSystem;
class SelectionData : public TArcDataNode
{
public:
    static const char* selectionPropertyName;
    static const char* selectionBoxPropertyName;
    static const char* selectionAllowedPropertyName;

    const SelectableGroup& GetSelection() const;
    SelectableGroup GetMutableSelection() const;
    void SetSelection(SelectableGroup& newSelection);

    const AABBox3& GetSelectionBox() const;

    void CancelSelection();

    //Support old selectionSystem interface
    void ResetSelectionComponentMask();
    void SetSelectionComponentMask(const ComponentMask& mask);
    const ComponentMask& GetSelectionComponentMask() const;

    void SetSelectionAllowed(bool allowed);
    bool IsSelectionAllowed() const;

    bool Lock();
    bool IsLocked() const;
    void Unlock();

    bool IsEntitySelectable(Entity* selectionCandidate) const;
    //end of old interface

private:
    friend class ::SelectionModule;

    std::unique_ptr<SelectionSystem> selectionSystem;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SelectionData, TArcDataNode)
    {
        ReflectionRegistrator<SelectionData>::Begin()
        .Field(selectionPropertyName, &SelectionData::GetSelection, nullptr)
        .Field(selectionBoxPropertyName, &SelectionData::GetSelectionBox, nullptr)
        .Field(selectionAllowedPropertyName, &SelectionData::IsSelectionAllowed, nullptr)
        .End();
    }
};

template <>
bool AnyCompare<SelectableGroup>::IsEqual(const Any& v1, const Any& v2);
extern template struct AnyCompare<SelectableGroup>;

} // namespace DAVA
