#pragma once

#include <TextureCompression/TextureConverter.h>

#include <Render/TextureDescriptor.h>

namespace DAVA
{
class FileList;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Info: all general code should be moved into Render/TextureDescriptorUtils.h
////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace RETextureDescriptorUtils
{
void ResaveDescriptorsForFolder(const DAVA::FilePath& folder);
void ResaveDescriptor(const DAVA::FilePath& descriptorPath);

void CreateDescriptorsForFolder(const DAVA::FilePath& folder, const DAVA::FilePath& presetPath);
bool CreateOrUpdateDescriptor(const DAVA::FilePath& texturePath, const DAVA::FilePath& presetPath = DAVA::FilePath());

void SetCompressionParamsForFolder(const DAVA::FilePath& folder, const DAVA::Map<DAVA::eGPUFamily, DAVA::TextureDescriptor::Compression>& compressionParams, bool convertionEnabled, bool force, DAVA::TextureConverter::eConvertQuality quality, bool generateMipMaps);
void SetCompressionParams(const DAVA::FilePath& descriptorPath, const DAVA::Map<DAVA::eGPUFamily, DAVA::TextureDescriptor::Compression>& compressionParams, bool convertionEnabled, bool force, DAVA::TextureConverter::eConvertQuality quality, bool generateMipMaps);

void SetPresetForFolder(const DAVA::FilePath& folder, const DAVA::FilePath& presetPath, bool toConvert, DAVA::TextureConverter::eConvertQuality quality);
void SetPreset(const DAVA::FilePath& descriptorPath, const DAVA::FilePath& presetPath, bool toConvert, DAVA::TextureConverter::eConvertQuality quality);

void SavePreset(const DAVA::Vector<DAVA::FilePath>& descriptorPath, const DAVA::Vector<DAVA::FilePath>& presetPath);
} // namespace RETextureDescriptorUtils
} // namespace DAVA
