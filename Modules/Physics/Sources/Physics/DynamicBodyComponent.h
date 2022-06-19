#pragma once

#include "Physics/PhysicsComponent.h"

namespace DAVA
{
class DynamicBodyComponent : public PhysicsComponent
{
public:
    Component* Clone(Entity* toEntity) override;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    float32 GetLinearDamping() const;
    void SetLinearDamping(float32 damping);

    float32 GetAngularDamping() const;
    void SetAngularDamping(float32 damping);

    float32 GetMaxAngularVelocity() const;
    void SetMaxAngularVelocity(float32 velocity);

    uint32 GetMinPositionIters() const;
    void SetMinPositionIters(uint32 minPositionIters);

    uint32 GetMinVelocityIters() const;
    void SetMinVelocityIters(uint32 minPositionIters);

    enum eLockFlags
    {
        NoLock = 0,
        LinearX = 0x1,
        LinearY = 0x2,
        LinearZ = 0x4,
        AngularX = 0x8,
        AngularY = 0x10,
        AngularZ = 0x20
    };

    eLockFlags GetLockFlags() const;
    void SetLockFlags(eLockFlags lockFlags);

    bool IsCCDEnabled() const;
    void SetCCDEnabled(bool isCCDEnabled);

protected:
#if defined(__DAVAENGINE_DEBUG__)
    void ValidateActorType() const override;
#endif

    void UpdateLocalProperties() override;

private:
    float32 linearDamping = 0.05f;
    float32 angularDamping = 0.05f;

    float32 maxAngularVelocity = 7.0f;
    eLockFlags lockFlags = NoLock;

    uint32 minPositionIters = 4;
    uint32 minVelocityIters = 1;
    bool enableCCD = false;

    DAVA_VIRTUAL_REFLECTION(DynamicBodyComponent, PhysicsComponent);
};
} // namespace DAVA
