#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Base/BaseTypes.h>
#include <FileSystem/FilePath.h>
#include <Math/AABBox3.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
class Landscape;
class LandscapeProxy;
//class LandscapeSetHeightMapCommand : public RECommand
//{
//public:
//    LandscapeSetHeightMapCommand(Entity* landscapeEntity,
//                                 const FilePath& texturePath,
//                                 const AABBox3& newLandscapeBox);
//
//    ~LandscapeSetHeightMapCommand();
//
//    void Undo() override;
//    void Redo() override;
//
//    Entity* GetEntity() const
//    {
//        return landscapeEntity;
//    }
//
//protected:
//    FilePath originalHeightMapPath;
//    FilePath newHeightMapPath;
//    Entity* landscapeEntity;
//    Landscape* landscape;
//    AABBox3 originalLandscapeBox;
//    AABBox3 newLandscapeBox;
//
//    DAVA_VIRTUAL_REFLECTION(LandscapeSetHeightMapCommand, RECommand);
//};
} // namespace DAVA
