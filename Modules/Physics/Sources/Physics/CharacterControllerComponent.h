#pragma once

#include <Entity/Component.h>

#include <Reflection/Reflection.h>

namespace physx
{
class PxController;
}

namespace DAVA
{
/**
    Class responsible for controlling a character.
    This component should only be attached to root entities (i.e. entities on a highest level of hierarchy).
*/
class CharacterControllerComponent : public Component
{
public:
    /** Enum describing different movement scehemes. */
    enum MovementMode
    {
        /** Gravity applies to a character, any displacement along up axis is ignored. */
        Walking,

        /** Gravity does not affect a character, free movement in any direction. */
        Flying
    };

    /** Set character's movement mode. */
    void SetMovementMode(MovementMode newMode);

    /** Get character's movement mode. */
    MovementMode GetMovementMode() const;

    /** Move a character for specified `displacement`. */
    void Move(const Vector3& displacement);

    /** Teleport a character to specified `worldPosition`. */
    void Teleport(const Vector3& worldPosition);

    /** Return `true` if object is touching the ground, `false` otherwise. */
    bool IsGrounded() const;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

protected:
    void ScheduleUpdate();

    virtual void CopyFieldsToComponent(CharacterControllerComponent* dest);

protected:
    bool geometryChanged = false;

private:
    friend class PhysicsSystem;

    physx::PxController* controller = nullptr;

    MovementMode mode = MovementMode::Walking;

    bool grounded = false;

    Vector3 totalDisplacement = Vector3::Zero;

    bool teleported = false;
    Vector3 teleportDestination = Vector3::Zero;

    DAVA_VIRTUAL_REFLECTION(CharacterControllerComponent, Component);
};
} // namespace DAVA