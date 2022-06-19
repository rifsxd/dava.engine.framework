#include "REPlatform/CommandLine/OptionName.h"
#include "REPlatform/CommandLine/SceneConsoleHelper.h"

#include <TArc/Utils/RenderContextGuard.h>

#include <CommandLine/ProgramOptions.h>
#include <Math/Color.h>
#include <Render/Renderer.h>
#include <Render/RenderHelper.h>
#include <Render/RHI/rhi_Public.h>
#include <Scene3D/Systems/QualitySettingsSystem.h>

namespace DAVA
{
namespace SceneConsoleHelperDetail
{
/*
 * Flush implementation
 * temporary (hopefully!) solution to clean-up RHI's objects
 * when there is no run/render loop in the application
 */
DAVA_DEPRECATED(void Flush())
{
    static const rhi::HTexture nullTexture;
    static const rhi::Viewport nullViewport(0, 0, 1, 1);

    rhi::HSyncObject currentFrame = rhi::GetCurrentFrameSyncObject();
    while (!rhi::SyncObjectSignaled(currentFrame))
    {
        Renderer::BeginFrame();
        RenderHelper::CreateClearPass(nullTexture, nullTexture, 0, Color::Clear, nullViewport);
        Renderer::EndFrame();
    }
}
}

FilePath SceneConsoleHelper::CreateQualityPathname(const FilePath& qualityPathname, const FilePath& targetPathname, const FilePath& resourceFolder)
{
    if (qualityPathname.IsEmpty() == false)
    {
        return qualityPathname;
    }

    if (resourceFolder.IsEmpty() == false)
    {
        return resourceFolder + "/quality.yaml";
    }

    String fullPath = targetPathname.GetAbsolutePathname();

    String::size_type pos = fullPath.find("/Data");
    if (pos != String::npos)
    {
        return (fullPath.substr(0, pos) + "/DataSource/quality.yaml");
    }

    return FilePath();
}

bool SceneConsoleHelper::InitializeQualitySystem(const ProgramOptions& options, const FilePath& targetPathname)
{
    FilePath resourceFolder;

    if (options.IsOptionExists(OptionName::ResourceDir))
    {
        resourceFolder = options.GetOption(OptionName::ResourceDir).AsString();
    }

    FilePath qualityPathname = options.GetOption(OptionName::QualityConfig).AsString();
    qualityPathname = CreateQualityPathname(qualityPathname, targetPathname, resourceFolder);
    if (qualityPathname.IsEmpty())
    {
        return false;
    }

    QualitySettingsSystem::Instance()->Load(qualityPathname);
    return true;
}

void SceneConsoleHelper::FlushRHI()
{
    RenderContextGuard guard;
    SceneConsoleHelperDetail::Flush();
}

} //DAVA
