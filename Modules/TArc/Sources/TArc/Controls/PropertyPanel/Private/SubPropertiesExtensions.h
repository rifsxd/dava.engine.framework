#pragma once

#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"

namespace DAVA
{
class SubPropertyValueChildCreator : public ChildCreatorExtension
{
public:
    void ExposeChildren(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const override;

private:
    void ExposeColorChildren(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const;
    void ExposeRectChildren(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const;
    void ExposeAABBox3Children(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const;

    template <typename TVector>
    void ExposeVectorChildren(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const;
};

class SubPropertyEditorCreator : public EditorComponentExtension
{
public:
    std::unique_ptr<BaseComponentValue> GetEditor(const std::shared_ptr<const PropertyNode>& node) const override;
};
} // namespace DAVA
