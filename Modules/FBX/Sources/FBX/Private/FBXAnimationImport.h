#pragma once

#include "Animation/AnimationTrack.h"

#include "FBXUtils.h"

namespace DAVA
{
class FilePath;

namespace FBXImporterDetails
{
struct FBXAnimationKey
{
    float32 time;
    Vector4 value;
};

struct FBXAnimationChannelData
{
    FBXAnimationChannelData() = default;
    FBXAnimationChannelData(AnimationTrack::eChannelTarget _channel, const Vector<FBXAnimationKey>& _animationKeys);

    AnimationTrack::eChannelTarget channel = AnimationTrack::CHANNEL_TARGET_COUNT;
    Vector<FBXAnimationKey> animationKeys;
};

struct FBXNodeAnimationData
{
    FbxNode* fbxNode = nullptr;
    Vector<FBXAnimationChannelData> animationTrackData;
};

struct FBXAnimationStackData
{
    String name;
    float32 minTimeStamp = std::numeric_limits<float32>::infinity();
    float32 maxTimeStamp = -std::numeric_limits<float32>::infinity();
    Vector<FBXNodeAnimationData> nodesAnimations;
};

//////////////////////////////////////////////////////////////////////////

Vector<FBXAnimationStackData> ImportAnimations(FbxScene* fbxScene);
void SaveAnimation(const FBXAnimationStackData& fbxStackAnimationData, const FilePath& filePath);
};
};