#include "REPlatform/Scene/Systems/BeastSystem.h"

#include <FileSystem/KeyedArchive.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/CustomPropertiesComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
float32 BeastSystem::DEFAULT_FALLOFFCUTOFF_VALUE = 1000.0f;

BeastSystem::BeastSystem(Scene* scene)
    : SceneSystem(scene)
{
}

void BeastSystem::AddEntity(Entity* entity)
{
    SetDefaultPropertyValues(entity);
}

void BeastSystem::PrepareForRemove()
{
}

void BeastSystem::SetDefaultPropertyValues(Entity* entity)
{
    KeyedArchive* propertyList = GetOrCreateCustomProperties(entity)->GetArchive();

    if (GetLight(entity))
    {
        SetBool(propertyList, "editor.staticlight.enable", true);
        SetInt32(propertyList, "editor.staticlight.shadowsamples", 1);
        SetFloat(propertyList, "editor.intensity", 1.0f);
        SetFloat(propertyList, "editor.staticlight.shadowangle", 0.0f);
        SetFloat(propertyList, "editor.staticlight.shadowradius", 0.0f);
        SetFloat(propertyList, "editor.staticlight.falloffcutoff", DEFAULT_FALLOFFCUTOFF_VALUE);
        SetFloat(propertyList, "editor.staticlight.falloffexponent", 1.0f);
        SetFloat(propertyList, "editor.staticlight.cone.angle", 90.0f);
        SetFloat(propertyList, "editor.staticlight.cone.penumbra.angle", 0.0f);
        SetFloat(propertyList, "editor.staticlight.cone.penumbra.exponent", 1.0f);
    }
}

void BeastSystem::SetBool(KeyedArchive* propertyList, const String& key, bool value)
{
    if (!propertyList->IsKeyExists(key))
    {
        propertyList->SetBool(key, value);
    }
}

void BeastSystem::SetFloat(KeyedArchive* propertyList, const String& key, float32 value)
{
    if (!propertyList->IsKeyExists(key))
    {
        propertyList->SetFloat(key, value);
    }
}

void BeastSystem::SetInt32(KeyedArchive* propertyList, const String& key, int32 value)
{
    if (!propertyList->IsKeyExists(key))
    {
        propertyList->SetInt32(key, value);
    }
}
} // namespace DAVA
