#pragma once

#include <TArc/Utils/ReflectionHelpers.h>

#include <Base/Any.h>
#include <Base/Type.h>
#include <Base/TypeInheritance.h>
#include <Debug/DVAssert.h>
#include <Math/Transform.h>
#include <Reflection/ReflectedType.h>
#include <Scene3D/Entity.h>

#include <functional>

namespace DAVA
{
class Selectable
{
public:
    enum class TransformPivot : uint32
    {
        ObjectCenter,
        CommonCenter
    };

    enum class TransformType : uint32
    {
        Disabled,
        Translation,
        Rotation,
        Scale
    };

    class TransformProxy
    {
    public:
        virtual ~TransformProxy() = default;
        virtual const Transform& GetWorldTransform(const Any& object) = 0;
        virtual const Transform& GetLocalTransform(const Any& object) = 0;
        virtual void SetLocalTransform(Any& object, const Transform& matrix) = 0;
        virtual bool SupportsTransformType(const Any& object, TransformType transformType) const = 0;
        virtual bool TransformDependsFromObject(const Any& dependant, const Any& dependsOn) const = 0;
    };

    template <typename CLASS, typename PROXY>
    static void AddTransformProxyForClass();
    static void RemoveAllTransformProxies();

public:
    Selectable() = default;
    explicit Selectable(const Any& object);
    Selectable(const Selectable& other);
    Selectable(Selectable&& other);

    Selectable& operator=(const Selectable& other);
    Selectable& operator=(Selectable&& other);

    bool operator==(const Selectable& other) const;
    bool operator!=(const Selectable& other) const;

    // comparing only pointers here, and not using bounding box
    // added for compatibility with sorted containers
    bool operator<(const Selectable& other) const;

    const ReflectedType* GetObjectType() const;

    template <typename T>
    bool CanBeCastedTo() const;

    template <typename T>
    T* Cast() const;

    const Any& GetContainedObject() const;
    Entity* AsEntity() const;

    const AABBox3& GetBoundingBox() const;
    void SetBoundingBox(const AABBox3& box);

    bool SupportsTransformType(TransformType) const;
    const Transform& GetLocalTransform() const;
    const Transform& GetWorldTransform() const;
    void SetLocalTransform(const Transform& transform);

    bool TransformDependsOn(const Selectable&) const;

    bool ContainsObject() const;

private:
    static void AddConcreteProxy(const Type* classInfo, TransformProxy* proxy);
    static TransformProxy* GetTransformProxyForClass(const Any& object);

private:
    Any object;
    AABBox3 boundingBox;
};

template <typename T>
bool Selectable::CanBeCastedTo() const
{
    if (ContainsObject() == false)
    {
        return false;
    }
    DVASSERT(object.GetType()->IsPointer());
    const ReflectedType* t = GetValueReflectedType(object);
    DVASSERT(t != nullptr);
    return TypeInheritance::CanCast(t->GetType()->Pointer(), Type::Instance<T*>());
}

template <typename T>
inline T* Selectable::Cast() const
{
    DVASSERT(ContainsObject() == true);
    DVASSERT(object.GetType()->IsPointer());
    const ReflectedType* t = GetValueReflectedType(object);
    DVASSERT(t != nullptr);
    const Type* objType = t->GetType()->Pointer();
    const Type* castToType = Type::Instance<T*>();
    if (TypeInheritance::CanCast(objType, castToType) == false)
    {
        DVASSERT(false);
        return nullptr;
    }

    void* inPtr = object.Get<void*>();
    void* outPtr = nullptr;
    bool casted = TypeInheritance::Cast(objType, castToType, inPtr, &outPtr);
    DVASSERT(casted == true);
    return reinterpret_cast<T*>(outPtr);
}

inline const Any& Selectable::GetContainedObject() const
{
    return object;
}

inline const AABBox3& Selectable::GetBoundingBox() const
{
    return boundingBox;
}

inline Entity* Selectable::AsEntity() const
{
    if (CanBeCastedTo<Entity>() == false)
    {
        return nullptr;
    }
    return Cast<Entity>();
}

template <typename CLASS, typename PROXY>
inline void Selectable::AddTransformProxyForClass()
{
    static_assert(std::is_base_of<Selectable::TransformProxy, PROXY>::value,
                  "Transform proxy should be derived from Selectable::TransformProxy");
    AddConcreteProxy(Type::Instance<CLASS>(), new PROXY());
}

inline bool Selectable::ContainsObject() const
{
    return object.IsEmpty() == false;
}
} // namespace DAVA

namespace std
{
template <>
struct hash<DAVA::Selectable>
{
    size_t operator()(const DAVA::Selectable& object) const
    {
        const DAVA::Any& obj = object.GetContainedObject();
        DVASSERT(obj.GetType()->IsPointer() == true);
        return reinterpret_cast<size_t>(obj.Get<void*>());
    }
};
} // namespace std
