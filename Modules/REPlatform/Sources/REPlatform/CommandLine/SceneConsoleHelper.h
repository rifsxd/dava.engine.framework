#pragma once

#include "FileSystem/FilePath.h"

namespace DAVA
{
class ProgramOptions;

class SceneConsoleHelper
{
public:
    static bool InitializeQualitySystem(const ProgramOptions& options, const FilePath& targetPathname);
    static FilePath CreateQualityPathname(const FilePath& qualityPathname,
                                          const FilePath& targetPathname = "",
                                          const FilePath& resourceFolder = "");
    static void FlushRHI();
};

} //DAVA
