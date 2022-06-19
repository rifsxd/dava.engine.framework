#include <Debug/DVAssert.h>
#include <FileSystem/File.h>
#include <Logger/Logger.h>
#include <Math/Vector.h>
#include <Render/2D/Sprite.h>
#include <Render/Image/Image.h>
#include <Render/Image/ImageSystem.h>
#include <Render/Texture.h>

#include <spine/spine.h>
#include <spine/extension.h>

void _spAtlasPage_createTexture(spAtlasPage* self, const char* path_)
{
    using namespace DAVA;

    FilePath path(path_);
    // Check file as is
    if (!path.Exists())
    {
        // Try find sprite file
        path = FilePath::CreateWithNewExtension(path, ".txt");
        if (!path.Exists())
        {
            // Try find texture descriptor
            path = FilePath::CreateWithNewExtension(path, ".tex");
            if (!path.Exists())
            {
                Logger::Error("Spine atlas texture file '%s' not found!", path_ != nullptr ? path_ : "nullptr");
                return;
            }
        }
    }

    Texture* texture = nullptr;
    if (path.GetExtension() == ".tex")
    {
        // Try open atlas as Texture
        texture = Texture::PureCreate(path);
    }
    else if (path.GetExtension() == ".txt")
    {
        // Try open atlas as Sprite
        Sprite* s = Sprite::Create(path);
        DVASSERT(s, "Create sprite failure!");
        texture = SafeRetain(s->GetTexture());
        SafeRelease(s);
    }
    else
    {
        // Try open atlas as Image
        Vector<Image*> images;
        ImageSystem::Load(path, images);
        DVASSERT(images.size() > 0 && images[0] != nullptr, "Failed to load image!");
        if (images.size() > 0 && images[0] != nullptr)
        {
            texture = Texture::CreateFromData(images[0], 0);
        }
        for (Image* image : images)
        {
            image->Release();
        }
    }

    DVASSERT(texture, "Failed to create texture!");
    self->rendererObject = texture;
    self->width = texture ? texture->GetWidth() : 0;
    self->height = texture ? texture->GetHeight() : 0;
}

void _spAtlasPage_disposeTexture(spAtlasPage* self)
{
    using namespace DAVA;
    if (self->rendererObject)
    {
        static_cast<Texture*>(self->rendererObject)->Release();
        self->rendererObject = nullptr;
    }
}

char* _spUtil_readFile(const char* path, int* length)
{
    using namespace DAVA;
    File* fp = File::Create(path, File::READ | File::OPEN);
    DVASSERT(fp != nullptr, "Failed to read file!");
    if (fp != nullptr)
    {
        *length = static_cast<int>(fp->GetSize());

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

        char* bytes = MALLOC(char, *length);

#ifdef __clang__
#pragma clang diagnostic pop
#endif

        fp->Read(bytes, *length);
        fp->Release();
        return bytes;
    }
    *length = 0;
    return nullptr;
}