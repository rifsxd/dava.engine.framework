#pragma once

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"

namespace DAVA
{
//command line constants for unification of command line
class OptionName
{
public:
    static const String Output;
    static const String OutFile;
    static const String OutDir;

    static const String ResourceDir;

    static const String File;
    static const String ProcessFile;

    static const String ProcessFileList;

    static const String Folder;
    static const String InDir;
    static const String ProcessDir;

    static const String QualityConfig;

    static const String Split;
    static const String Merge;
    static const String Save;
    static const String Resave;
    static const String Build;
    static const String Convert;
    static const String Create;

    static const String Links;
    static const String Scene;
    static const String Texture;
    static const String Yaml;

    static const String GPU;
    static const String Quality;
    static const String Force;
    static const String Mipmaps;
    static const String HDTextures;
    static const String Mode;

    static const String SaveNormals;
    static const String CopyConverted;
    static const String SetCompression;
    static const String SetPreset;
    static const String SavePreset;
    static const String PresetOpt;
    static const String PresetsList;

    static const String UseAssetCache;
    static const String AssetCacheIP;
    static const String AssetCachePort;
    static const String AssetCacheTimeout;

    static const String Width;
    static const String Height;
    static const String Camera;

    static const String Validate;
    static const String Count;

    static const String Tag;
    static const String TagList;

    static const String MakeNameForGPU(eGPUFamily gpuFamily);
};

} //DAVA
