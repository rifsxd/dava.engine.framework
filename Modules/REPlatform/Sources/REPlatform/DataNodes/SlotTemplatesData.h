#pragma once

#include <TArc/DataProcessing/TArcDataNode.h>

#include <Base/FastName.h>
#include <Base/UnordererMap.h>
#include <FileSystem/FilePath.h>
#include <Math/Color.h>
#include <Math/Vector.h>
#include <Reflection/Reflection.h>

class SlotSupportModule;
namespace DAVA
{
class SlotTemplatesData : public TArcDataNode
{
public:
    struct Template
    {
        FastName name;
        Vector3 boundingBoxSize;
        Vector3 pivot;
    };

    const Template* GetTemplate(FastName name) const;
    Vector<Template> GetTemplates() const;

private:
    friend class ::SlotSupportModule;
    void Clear();
    void ParseConfig(const FilePath& configPath);

private:
    UnorderedMap<FastName, Template> templates;

    DAVA_VIRTUAL_REFLECTION(SlotTemplatesData, TArcDataNode);
};
} // namespace DAVA