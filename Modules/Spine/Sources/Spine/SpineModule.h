#pragma once

#include <ModuleManager/IModule.h>

namespace DAVA
{
class Engine;

/** Spine module.
Register UI systems and components for work with Spine.
*/
class SpineModule : public IModule
{
    DAVA_VIRTUAL_REFLECTION(SpineModule, IModule);

public:
    SpineModule(Engine* engine);
    ~SpineModule() override = default;

    void Init() override;
    void Shutdown() override;
};
}