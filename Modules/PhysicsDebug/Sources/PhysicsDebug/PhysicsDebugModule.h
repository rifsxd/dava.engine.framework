#pragma once

#include <ModuleManager/IModule.h>
#include <Math/Vector.h>

namespace physx
{
class PxFoundation;
class PxPvd;
class PxPvdTransport;
} // namespace physx

namespace DAVA
{
class PhysicsDebugModule : public IModule
{
public:
    PhysicsDebugModule(Engine* engine);
    ~PhysicsDebugModule();
    void Init() override;
    void Shutdown() override;

private:
    physx::PxPvd* CreatePvd(physx::PxFoundation* foundation);
    void ReleasePvd();

    physx::PxPvd* pvd = nullptr;
    physx::PxPvdTransport* transport = nullptr;

    DAVA_VIRTUAL_REFLECTION(PhysicsDebugModule, IModule);
};
}