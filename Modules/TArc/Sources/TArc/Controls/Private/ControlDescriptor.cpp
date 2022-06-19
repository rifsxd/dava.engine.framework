#include "TArc/Controls/ControlDescriptor.h"

namespace DAVA
{
DescriptorNode::DescriptorNode()
    : fieldName("")
{
}

DescriptorNode& DescriptorNode::operator=(const char* name)
{
    fieldName = FastName(name);
    return *this;
}

DescriptorNode& DescriptorNode::operator=(const String& name)
{
    fieldName = FastName(name);
    return *this;
}

DescriptorNode& DescriptorNode::operator=(const FastName& name)
{
    fieldName = name;
    return *this;
}

void DescriptorNode::BindConstValue(const DAVA::Any& value_)
{
    value = value_;
}

const DAVA::Any& DescriptorNode::GetValue() const
{
    return value;
}

} // namespace DAVA