#pragma once

#include <Command/Command.h>

namespace DAVA
{
class RECommandNotificationObject;
class REDependentCommandsHolder;
class Scene;
class PropertiesHolder;
class ContextAccessor;

class EditorSceneSystem
{
    friend class SceneEditor2;

public:
    virtual ~EditorSceneSystem() = default;

    virtual void EnableSystem();
    virtual void DisableSystem();
    bool IsSystemEnabled() const;

    virtual void LoadLocalProperties(DAVA::PropertiesHolder* holder, DAVA::ContextAccessor* accessor)
    {
    }

    virtual void SaveLocalProperties(DAVA::PropertiesHolder* holder)
    {
    }

protected:
    virtual void Draw()
    {
    }
    virtual void AccumulateDependentCommands(REDependentCommandsHolder& holder)
    {
    }
    virtual void ProcessCommand(const RECommandNotificationObject& commandNotification)
    {
    }

    virtual std::unique_ptr<Command> PrepareForSave(bool saveForGame)
    {
        return nullptr;
    }

    class InputLockGuard
    {
    public:
        InputLockGuard(DAVA::Scene* scene_, EditorSceneSystem* system_)
            : scene(scene_)
            , system(system_)
        {
            lockAcquired = system->AcquireInputLock(scene);
        }

        ~InputLockGuard()
        {
            system->ReleaseInputLock(scene);
        }

        bool IsLockAcquired() const
        {
            return lockAcquired;
        }

    private:
        DAVA::Scene* scene;
        EditorSceneSystem* system;
        bool lockAcquired = false;
    };

    bool AcquireInputLock(DAVA::Scene* scene);
    void ReleaseInputLock(DAVA::Scene* scene);

private:
    bool systemIsEnabled = false;
};
} // namespace DAVA
