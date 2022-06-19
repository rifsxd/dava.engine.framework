#pragma once

#include <TArc/Core/ConsoleModule.h>

#include <Base/BaseTypes.h>
#include <Base/Result.h>
#include <CommandLine/ProgramOptions.h>
#include <FileSystem/FilePath.h>

namespace DAVA
{
namespace Metas
{
struct CommandName
{
    CommandName(const String& commandName_)
        : commandName(commandName_)
    {
    }
    String commandName;
};
}

namespace M
{
using CommandName = Meta<Metas::CommandName>;
}

class CommandLineModule : public ConsoleModule
{
public:
    CommandLineModule(const Vector<String>& commandLine, const String& moduleName);

    int GetExitCode() const override;

protected:
    void PostInit() override;
    eFrameResult OnFrame() override;
    void BeforeDestroyed() override;

    virtual bool PostInitInternal();
    virtual void ShowHelpInternal();

    virtual eFrameResult OnFrameInternal();
    virtual void BeforeDestroyedInternal();

    Vector<String> commandLine;
    ProgramOptions options;

    bool isInitialized = false;
    Result result = Result::RESULT_SUCCESS;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(CommandLineModule, ConsoleModule)
    {
    }
};

inline int CommandLineModule::GetExitCode() const
{
    return (result.type == Result::RESULT_SUCCESS ? 0 : -1);
}
} // namespace DAVA