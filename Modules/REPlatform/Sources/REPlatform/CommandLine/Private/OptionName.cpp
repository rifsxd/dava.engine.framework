#include "REPlatform/CommandLine/OptionName.h"
#include <Render/GPUFamilyDescriptor.h>

namespace DAVA
{
const String OptionName::Output("-output");
const String OptionName::OutFile("-outfile");
const String OptionName::OutDir("-outdir");

const String OptionName::ResourceDir("-resdir");

const String OptionName::File("-file");
const String OptionName::ProcessFile("-processfile");

const String OptionName::ProcessFileList("-processfilelist");

const String OptionName::Folder("-folder");
const String OptionName::InDir("-indir");
const String OptionName::ProcessDir("-processdir");

const String OptionName::QualityConfig("-qualitycfgpath");

const String OptionName::Split("-split");
const String OptionName::Merge("-merge");
const String OptionName::Save("-save");
const String OptionName::Resave("-resave");
const String OptionName::Build("-build");
const String OptionName::Convert("-convert");
const String OptionName::Create("-create");

const String OptionName::Links("-links");
const String OptionName::Scene("-scene");
const String OptionName::Texture("-texture");
const String OptionName::Yaml("-yaml");

const String OptionName::GPU("-gpu");
const String OptionName::Quality("-quality");
const String OptionName::Force("-f");
const String OptionName::Mipmaps("-m");
const String OptionName::HDTextures("-hd");
const String OptionName::Mode("-mode");

const String OptionName::SaveNormals("-saveNormals");
const String OptionName::CopyConverted("-copyconverted");
const String OptionName::SetCompression("-setcompression");
const String OptionName::SetPreset("-setpreset");
const String OptionName::SavePreset("-savepreset");
const String OptionName::PresetOpt("-preset");
const String OptionName::PresetsList("-presetslist");

const String OptionName::UseAssetCache("-useCache");
const String OptionName::AssetCacheIP("-ip");
const String OptionName::AssetCachePort("-p");
const String OptionName::AssetCacheTimeout("-t");

const String OptionName::Width("-width");
const String OptionName::Height("-height");
const String OptionName::Camera("-camera");

const String OptionName::Validate("-validate");
const String OptionName::Count("-count");

const String OptionName::Tag("-tag");
const String OptionName::TagList("-taglist");

const String OptionName::MakeNameForGPU(eGPUFamily gpuFamily)
{
    return ("-" + GPUFamilyDescriptor::GetGPUName(gpuFamily));
}

} //DAVA
