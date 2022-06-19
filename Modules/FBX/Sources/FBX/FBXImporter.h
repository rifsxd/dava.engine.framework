#pragma once

namespace DAVA
{
class FilePath;
class Scene;
class FBXImporter
{
public:
    static Scene* ConstructSceneFromFBX(const FilePath& fbxPath);
    static bool ConvertToSC2(const FilePath& fbxPath, const FilePath& sc2Path);
    static bool ConvertAnimations(const FilePath& fbxPath);
};
};