/*
	Copyright (C) 2005-2007 Feeling Software Inc.
	Portions of the code are:
	Copyright (C) 2005-2007 Sony Computer Entertainment America
	
	MIT License: http://www.opensource.org/licenses/mit-license.php
*/

#include "StdAfx.h"
#include "FCDParameterAnimatable.h"
#if !defined(__APPLE__) && !defined(LINUX)
#include "FCDParameterAnimatable.hpp"
#endif
#include "FCDAnimated.h"
#include <FUtils/FUParameterizable.h>
#include <FMath/FMAngleAxis.h>
#include <FMath/FMLookAt.h>
#include <FMath/FMSkew.h>

//
// FCDParameterAnimatable
//

FCDParameterAnimatable::FCDParameterAnimatable(FUParameterizable* _parent)
    : parent(_parent)
{
}

FCDParameterAnimatable::~FCDParameterAnimatable()
{
    parent = NULL;
}

FCDAnimated* FCDParameterAnimatable::GetAnimated()
{
    if (animated == NULL)
        animated = CreateAnimated();
    return animated;
}
const FCDAnimated* FCDParameterAnimatable::GetAnimated() const
{
    if (animated == NULL)
    {
        FCDParameterAnimatable* _this = const_cast<FCDParameterAnimatable*>(this);
        _this->animated = _this->CreateAnimated();
    }
    return animated;
}

bool FCDParameterAnimatable::IsAnimated() const
{
    return animated != NULL && animated->HasCurve();
}

FCDAnimated* FCDParameterAnimatable::CreateAnimated()
{
    // Implemented below in template specializations of FCDParameterAnimatableT.
    return NULL;
}

FCDParameterAnimatable& FCDParameterAnimatable::operator=(FCDParameterAnimatable& UNUSED(parameter))
{
    // This empty copy operator is necessary to avoid the default copy operator
    // generated by the compiler which copies the parent pointer.
    return *this;
}

//
// FCDParameterAnimatableT
//

template <>
FCDAnimated* FCDParameterAnimatableFloat::CreateAnimated()
{
    float* values[1] = { &value };
    return new FCDAnimated((FCDObject*)GetParent(), 1, FCDAnimatedStandardQualifiers::EMPTY, values);
}

template <>
FCDAnimated* FCDParameterAnimatableVector2::CreateAnimated()
{
    float* values[2] = { &value.x, &value.y };
    return new FCDAnimated((FCDObject*)GetParent(), 2, FCDAnimatedStandardQualifiers::XYZW, values);
}

template <>
FCDAnimated* FCDParameterAnimatableVector3::CreateAnimated()
{
    float* values[3] = { &value.x, &value.y, &value.z };
    return new FCDAnimated((FCDObject*)GetParent(), 3, FCDAnimatedStandardQualifiers::XYZW, values);
}

template <>
FCDAnimated* FCDParameterAnimatableColor3::CreateAnimated()
{
    float* values[3] = { &value.x, &value.y, &value.z };
    return new FCDAnimated((FCDObject*)GetParent(), 3, FCDAnimatedStandardQualifiers::RGBA, values);
}

template <>
FCDAnimated* FCDParameterAnimatableVector4::CreateAnimated()
{
    float* values[4] = { &value.x, &value.y, &value.z, &value.w };
    return new FCDAnimated((FCDObject*)GetParent(), 4, FCDAnimatedStandardQualifiers::XYZW, values);
}

template <>
FCDAnimated* FCDParameterAnimatableColor4::CreateAnimated()
{
    float* values[4] = { &value.x, &value.y, &value.z, &value.w };
    return new FCDAnimated((FCDObject*)GetParent(), 4, FCDAnimatedStandardQualifiers::RGBA, values);
}

template <>
FCDAnimated* FCDParameterAnimatableMatrix44::CreateAnimated()
{
    float* values[16] =
    {
      &value[0][0], &value[1][0], &value[2][0], &value[3][0],
      &value[0][1], &value[1][1], &value[2][1], &value[3][1],
      &value[0][2], &value[1][2], &value[2][2], &value[3][2],
      &value[0][3], &value[1][3], &value[2][3], &value[3][3]
    };
    return new FCDAnimated((FCDObject*)GetParent(), 16, FCDAnimatedStandardQualifiers::MATRIX, values);
}

template <>
FCDAnimated* FCDParameterAnimatableAngleAxis::CreateAnimated()
{
    float* values[4] = { &value.axis.x, &value.axis.y, &value.axis.z, &value.angle };
    return new FCDAnimated((FCDObject*)GetParent(), 4, FCDAnimatedStandardQualifiers::ROTATE_AXIS, values);
}

template <>
FCDAnimated* FCDParameterAnimatableLookAt::CreateAnimated()
{
    float* values[9] =
    {
      &value.position.x, &value.position.y, &value.position.z,
      &value.target.x, &value.target.y, &value.target.z,
      &value.up.x, &value.up.y, &value.up.z
    };
    return new FCDAnimated((FCDObject*)GetParent(), 9, FCDAnimatedStandardQualifiers::LOOKAT, values);
}

template <>
FCDAnimated* FCDParameterAnimatableSkew::CreateAnimated()
{
    float* values[7] =
    {
      &value.rotateAxis.x, &value.rotateAxis.y, &value.rotateAxis.z,
      &value.aroundAxis.x, &value.aroundAxis.y, &value.aroundAxis.z,
      &value.angle
    };
    return new FCDAnimated((FCDObject*)GetParent(), 7, FCDAnimatedStandardQualifiers::LOOKAT, values);
}

//
// FCDParameterListAnimatable
//
FCDParameterListAnimatable::FCDParameterListAnimatable(FUParameterizable* _parent)
    : parent(_parent)
{
}

FCDParameterListAnimatable::~FCDParameterListAnimatable()
{
    parent = NULL;
}

const FCDAnimated* FCDParameterListAnimatable::GetAnimated(size_t index) const
{
    return const_cast<const FCDAnimated*>(const_cast<FCDParameterListAnimatable*>(this)->GetAnimated(index));
}
FCDAnimated* FCDParameterListAnimatable::GetAnimated(size_t index)
{
    // Check for existing
    size_t mid = BinarySearch(index);
    if (mid < animateds.size() && animateds[mid]->GetArrayElement() == (int32)index)
    {
        return animateds[mid];
    }

    // Create new animated value.
    FCDAnimated* animated = CreateAnimated(index);
    animated->SetArrayElement((int32)index);
    animateds.insert(mid, animated);
    return animated;
}

bool FCDParameterListAnimatable::IsAnimated(size_t index) const
{
    size_t animatedCount = animateds.size();
    if (index == ~(size_t)0)
    {
        for (size_t i = 0; i < animatedCount; ++i)
        {
            if (animateds[i]->HasCurve())
                return true;
        }
        return false;
    }
    else
    {
        size_t i = BinarySearch(index);
        return i < animatedCount && animateds[i]->GetArrayElement() == (int32)index && animateds[i]->HasCurve();
    }
}

void FCDParameterListAnimatable::OnInsertion(size_t offset, size_t count)
{
    // Push forward all the array element indices starting at "offset" by "count".
    for (size_t i = BinarySearch(offset); i < animateds.size(); ++i)
    {
        int32 arrayElement = animateds[i]->GetArrayElement();
        FUAssert(arrayElement >= (int32)offset, continue);
        animateds[i]->SetArrayElement(arrayElement + (int32)count);
    }
}

void FCDParameterListAnimatable::OnRemoval(size_t offset, size_t count)
{
    // Release all the animated helpers for the removed values.
    for (size_t i = BinarySearch(offset); i < animateds.size();)
    {
        int32 arrayElement = animateds[i]->GetArrayElement();
        FUAssert(arrayElement >= (int32)offset, continue);
        if (arrayElement >= (int32)(offset + count))
            break;
        animateds[i]->Release(); // automatic removal from this array..
    }

    // Push back all the array element indices starting at "offset + count" by "count".
    for (size_t i = BinarySearch(offset + count); i < animateds.size(); ++i)
    {
        int32 arrayElement = animateds[i]->GetArrayElement();
        FUAssert(arrayElement >= (int32)(offset + count), continue);
        animateds[i]->SetArrayElement(arrayElement - (int32)count);
    }
}

size_t FCDParameterListAnimatable::BinarySearch(size_t arrayElementIndex) const
{
    // Binary search..
    size_t start = 0, end = animateds.size();
    size_t mid;
    for (mid = (end + start) / 2; start < end; mid = (end + start) / 2)
    {
        int32 arrayElement = animateds[mid]->GetArrayElement();
        if (arrayElement < (int32)arrayElementIndex)
            start = mid + 1;
        else if (arrayElement > (int32)arrayElementIndex)
            end = mid;
        else
            break;
    }
    return mid;
}

FCDAnimated* FCDParameterListAnimatable::CreateAnimated(size_t UNUSED(index))
{
    // Implemented by the specialized template functions below.
    return NULL;
}

//
// FCDParameterListAnimatableT
//

template <>
FCDAnimated* FCDParameterListAnimatableFloat::CreateAnimated(size_t index)
{
    float* _values[1] = { &values[index] };
    return new FCDAnimated((FCDObject*)GetParent(), 1, FCDAnimatedStandardQualifiers::EMPTY, _values);
}

template <>
FCDAnimated* FCDParameterListAnimatableVector2::CreateAnimated(size_t index)
{
    float* _values[2] = { &values[index].x, &values[index].y };
    return new FCDAnimated((FCDObject*)GetParent(), 2, FCDAnimatedStandardQualifiers::XYZW, _values);
}

template <>
FCDAnimated* FCDParameterListAnimatableVector3::CreateAnimated(size_t index)
{
    float* _values[3] = { &values[index].x, &values[index].y, &values[index].z };
    return new FCDAnimated((FCDObject*)GetParent(), 3, FCDAnimatedStandardQualifiers::XYZW, _values);
}

template <>
FCDAnimated* FCDParameterListAnimatableColor3::CreateAnimated(size_t index)
{
    float* _values[3] = { &values[index].x, &values[index].y, &values[index].z };
    return new FCDAnimated((FCDObject*)GetParent(), 3, FCDAnimatedStandardQualifiers::RGBA, _values);
}

template <>
FCDAnimated* FCDParameterListAnimatableVector4::CreateAnimated(size_t index)
{
    float* _values[4] = { &values[index].x, &values[index].y, &values[index].z, &values[index].w };
    return new FCDAnimated((FCDObject*)GetParent(), 4, FCDAnimatedStandardQualifiers::XYZW, _values);
}

template <>
FCDAnimated* FCDParameterListAnimatableColor4::CreateAnimated(size_t index)
{
    float* _values[4] = { &values[index].x, &values[index].y, &values[index].z, &values[index].w };
    return new FCDAnimated((FCDObject*)GetParent(), 4, FCDAnimatedStandardQualifiers::RGBA, _values);
}

//
// Templatization Trick
//
template <class T, int Q>
void TrickLinkerFCDParameterAnimatableT(const T& value)
{
    FCDParameterAnimatableT<T, Q> v1((FUParameterizable *)NULL), v2((FUParameterizable *)NULL, value);
    if (v1 == value)
        v1 = value;
    T bb = (T)v1;
    (void)bb;
    T& bb2 = (T&)v1;
    (void)bb2;
    const T& bb3 = (const T&)v2;
    (void)bb3;
    *v1;
    v1.operator->();
    v1.operator*();
    *const_cast<const FCDParameterAnimatableT<T, Q>&>(v1);
    const_cast<const FCDParameterAnimatableT<T, Q>&>(v1).operator->();
    const_cast<const FCDParameterAnimatableT<T, Q>&>(v1).operator*();
    FCDAnimated* aa = v1.GetAnimated();
    if (v1.IsAnimated())
        ++aa;
}

template <class T, int Q>
void TrickLinkerFCDParameterListAnimatableT(const T& value)
{
    FCDParameterListAnimatableT<T, Q> v1((FUParameterizable*)NULL);
    v1.push_back(value);
    v1 = v1;
    v1.set(0, value);
    v1.clear();
    v1.insert(0, value);
    v1.insert(0, &value, 1);
    v1.insert(0, 5, value);
    v1.erase((size_t)5);
    v1.erase(value);
    v1.erase((size_t)0, (size_t)4);
    v1.push_back(v1.front());
    v1.push_front(v1.back());
    v1.pop_back();
    v1.pop_front();
    v1.resize(4);
    v1.resize(6, value);
    v1.empty();
    v1.size();
    if (v1.contains(value) && v1.empty())
        v1.find(value);
    v1[0];
    v1.at(0);
    FCDAnimated* aa = v1.GetAnimated(0);
    v1.GetDataList();
    if (v1.IsAnimated())
        ++aa;
    const_cast<const FCDParameterListAnimatableT<T, Q>&>(v1).front();
    const_cast<const FCDParameterListAnimatableT<T, Q>&>(v1).back();
    const_cast<const FCDParameterListAnimatableT<T, Q>&>(v1).GetDataList();
}

extern void TrickLinkerFCDParameterAnimatable()
{
    // Primitive animatables
    TrickLinkerFCDParameterAnimatableT<float, 0>(0.03f);
    TrickLinkerFCDParameterAnimatableT<FMVector2, 0>(FMVector2::XAxis);
    TrickLinkerFCDParameterAnimatableT<FMVector3, 0>(FMVector3::YAxis);
    TrickLinkerFCDParameterAnimatableT<FMVector3, 1>(FMVector3::YAxis);
    TrickLinkerFCDParameterAnimatableT<FMVector4, 0>(FMVector4::Zero);
    TrickLinkerFCDParameterAnimatableT<FMVector4, 1>(FMVector4::Zero);
    TrickLinkerFCDParameterAnimatableT<FMMatrix44, 0>(FMMatrix44::Identity);
    TrickLinkerFCDParameterAnimatableT<FMSkew, 0>(FMSkew());
    TrickLinkerFCDParameterAnimatableT<FMLookAt, 0>(FMLookAt());
    TrickLinkerFCDParameterAnimatableT<FMAngleAxis, 0>(FMAngleAxis());

    // Primitive list animatables
    TrickLinkerFCDParameterListAnimatableT<float, 0>(0.03f);
    TrickLinkerFCDParameterListAnimatableT<FMVector2, 0>(FMVector2::XAxis);
    TrickLinkerFCDParameterListAnimatableT<FMVector3, 0>(FMVector3::YAxis);
    TrickLinkerFCDParameterListAnimatableT<FMVector3, 1>(FMVector3::YAxis);
    TrickLinkerFCDParameterListAnimatableT<FMVector4, 0>(FMVector4::Zero);
    TrickLinkerFCDParameterListAnimatableT<FMVector4, 1>(FMVector4::Zero);
}
