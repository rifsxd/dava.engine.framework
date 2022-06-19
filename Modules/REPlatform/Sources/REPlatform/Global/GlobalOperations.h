#pragma once

#include <TArc/Core/OperationRegistrator.h>

namespace DAVA
{
DECLARE_OPERATION_ID(OpenLastProjectOperation); // Args - empty
DECLARE_OPERATION_ID(CreateFirstSceneOperation); // Args - empty
DECLARE_OPERATION_ID(OpenSceneOperation); // Args - scenePath: DAVA::FilePath
DECLARE_OPERATION_ID(AddSceneOperation); // Args - scenePath: DAVA::FilePath
DECLARE_OPERATION_ID(SaveCurrentScene); // Args - empty
DECLARE_OPERATION_ID(CloseAllScenesOperation); // Args - need ask user about saving scenes : bool
DECLARE_OPERATION_ID(ReloadAllTextures); // Args - gpu : eGpuFamily
DECLARE_OPERATION_ID(ReloadTextures); // Args - textures : DAVA::Vector<DAVA::Texture*>
DECLARE_OPERATION_ID(ShowMaterial); // Args - NMaterial*
DECLARE_OPERATION_ID(ConvertTaggedTextures); // Args - empty
} // namespace DAVA
