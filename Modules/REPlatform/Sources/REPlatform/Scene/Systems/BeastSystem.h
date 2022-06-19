#pragma once

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>

namespace DAVA
{
class Entity;
class Scene;
class KeyedArchive;

class BeastSystem : public SceneSystem
{
public:
    BeastSystem(Scene* scene);

    void static SetDefaultPropertyValues(Entity* entity);

    void AddEntity(Entity* entity) override;
    void PrepareForRemove() override;

    static float32 DEFAULT_FALLOFFCUTOFF_VALUE;

private:
    static void SetBool(KeyedArchive* propertyList, const String& key, bool value);
    static void SetFloat(KeyedArchive* propertyList, const String& key, float32 value);
    static void SetInt32(KeyedArchive* propertyList, const String& key, int32 value);
};
} // namespace DAVA
