#include "TextureCompression/TextureCompressionModule.h"
#include "TextureCompression/Private/ImageConverterImpl.h"
#include "TextureCompression/Private/PVRConverter.h"

#include <Engine/Engine.h>
#include <FileSystem/FileSystem.h>
#include <Logger/Logger.h>
#include <Render/Image/ImageConverter.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
TextureCompressionModule::TextureCompressionModule(Engine* engine_)
    : IModule(engine_)
    , engine(engine_)
{
    imageConverter.reset(new ImageConverterImpl());
}

void TextureCompressionModule::Init()
{
    DVASSERT(GetEngineContext()->imageConverter != nullptr);
    GetEngineContext()->imageConverter->SetImplementation(imageConverter.get());
    
#if defined(__DAVAENGINE_MACOS__)
    String pvrToolName = "PVRTexToolCLI";
#elif defined(__DAVAENGINE_WINDOWS__)
    String pvrToolName = "PVRTexToolCLI.exe";
#else //PLATFORMS
#error "Unknown platform"
#endif //PLATFORMS

    FilePath pvrTexPath = "~res:/" + pvrToolName;

    const Vector<String>& cmdLine = engine->GetCommandLine();
    if (GetEngineContext()->fileSystem->Exists(pvrTexPath) == true)
    {
        PVRConverter::Instance()->SetPVRTexTool(pvrTexPath);
    }
    else if (engine->IsConsoleMode() && cmdLine.empty() == false)
    {
        pvrTexPath = FilePath(cmdLine[0]).GetDirectory() + "Data/" + pvrToolName;
        PVRConverter::Instance()->SetPVRTexTool(pvrTexPath);
    }
    else
    {
        Logger::Error("Cannot setup PVRTexTool pathname");
    }
}

void TextureCompressionModule::Shutdown()
{
    DVASSERT(GetEngineContext()->imageConverter != nullptr);
    GetEngineContext()->imageConverter->SetImplementation(nullptr);
}

DAVA_VIRTUAL_REFLECTION_IMPL(TextureCompressionModule)
{
    ReflectionRegistrator<TextureCompressionModule>::Begin()
    .End();
}
}
