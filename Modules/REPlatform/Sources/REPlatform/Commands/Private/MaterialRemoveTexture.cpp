#include "REPlatform/Commands/MaterialRemoveTexture.h"

#include <Base/FastName.h>
#include <Debug/DVAssert.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Material/NMaterial.h>

namespace DAVA
{
MaterialRemoveTexture::MaterialRemoveTexture(const DAVA::FastName& textureSlot_, DAVA::NMaterial* material_)
    : RECommand("Remove invalid texture from material")
{
    DVASSERT(material_ != nullptr);
    DVASSERT(textureSlot_.IsValid());
    DVASSERT(material_->HasLocalTexture(textureSlot_));

    textureSlot = textureSlot_;
    material = DAVA::SafeRetain(material_);
    texture = DAVA::SafeRetain(material->GetLocalTexture(textureSlot));
}

MaterialRemoveTexture::~MaterialRemoveTexture()
{
    DAVA::SafeRelease(texture);
    DAVA::SafeRelease(material);
}

void MaterialRemoveTexture::Undo()
{
    material->AddTexture(textureSlot, texture);
}

void MaterialRemoveTexture::Redo()
{
    material->RemoveTexture(textureSlot);
}

DAVA_VIRTUAL_REFLECTION_IMPL(MaterialRemoveTexture)
{
    ReflectionRegistrator<MaterialRemoveTexture>::Begin()
    .End();
}
} // namespace DAVA
