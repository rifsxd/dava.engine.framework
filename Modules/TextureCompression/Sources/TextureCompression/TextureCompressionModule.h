#pragma once

#include <ModuleManager/IModule.h>
#include <Reflection/Reflection.h>

#include <memory>

namespace DAVA
{
class Engine;
class ImageConverter;
class TextureCompressionModule : public IModule
{
public:
    TextureCompressionModule(Engine* engine);

    void Init() override;
    void Shutdown() override;

private:
    std::unique_ptr<ImageConverter> imageConverter;
    Engine* engine = nullptr;

    DAVA_VIRTUAL_REFLECTION(TextureConverterModule, IModule);
};
}
