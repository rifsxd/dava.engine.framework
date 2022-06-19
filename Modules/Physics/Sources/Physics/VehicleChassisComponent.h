#pragma once

#include <Entity/Component.h>
#include <Reflection/Reflection.h>
#include <Math/Vector.h>

namespace DAVA
{
class Entity;

class VehicleChassisComponent final : public Component
{
public:
    Vector3 GetCenterOfMassOffset() const;
    void SetCenterOfMassOffset(const Vector3& value);

private:
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    virtual Component* Clone(Entity* toEntity) override;

    DAVA_VIRTUAL_REFLECTION(VehicleChassisComponent, Component);

private:
    Vector3 centerOfMassOffset = Vector3::Zero;
};
}