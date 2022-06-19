#pragma once

namespace DAVA
{
class TextureDescriptor;
class NMaterial;
class KeyedArchive;
class FilePath;

namespace Preset
{
bool SaveArchive(const KeyedArchive* presetArchive, const FilePath& path);
KeyedArchive* LoadArchive(const FilePath& path);

bool ApplyTexturePreset(TextureDescriptor* descriptor, const KeyedArchive* preset);

bool DialogSavePresetForTexture(const TextureDescriptor* descriptor);
bool DialogLoadPresetForTexture(TextureDescriptor* descriptor);

bool DialogSavePresetForMaterial(NMaterial* material);
bool DialogLoadPresetForMaterial(NMaterial* material);
}
} // namespace DAVA
