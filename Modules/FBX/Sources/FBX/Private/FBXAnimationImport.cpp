#include "FBXAnimationImport.h"

#include "Animation/AnimationClip.h"
#include "FileSystem/File.h"
#include "Logger/Logger.h"
#include "Utils/CRC32.h"

namespace DAVA
{
namespace FBXAnimationImportDetails
{
//namespace declarations

template <class T>
void WriteToBuffer(Vector<uint8>& buffer, const T* value);
void WriteToBuffer(Vector<uint8>& buffer, const String& string);
void WriteToBuffer(Vector<uint8>& buffer, const void* data, uint32 size);

Set<FbxTime> GetAnimationSamples(FbxPropertyT<FbxDouble3>& fbxProperty, FbxAnimLayer* fbxAnimLayer);
Vector<FBXImporterDetails::FBXAnimationKey> GetAnimationKeys(FbxNode* fbxNode, const Set<FbxTime>& keyTimes, AnimationTrack::eChannelTarget channel);
FBXImporterDetails::FBXNodeAnimationData GetNodeAnimationData(FbxNode* fbxNode, FbxAnimLayer* fbxAnimLayer);
void ProcessNodeAnimationRecursive(FbxNode* fbxNode, FbxAnimLayer* fbxAnimLayer, Vector<FBXImporterDetails::FBXNodeAnimationData>* outNodesAnimations);
};

namespace FBXImporterDetails
{
FBXAnimationChannelData::FBXAnimationChannelData(AnimationTrack::eChannelTarget _channel, const Vector<FBXAnimationKey>& _animationKeys)
    : channel(_channel)
    , animationKeys(_animationKeys)
{
}

Vector<FBXAnimationStackData> ImportAnimations(FbxScene* fbxScene)
{
    Vector<FBXAnimationStackData> result;

    int animationStackCount = fbxScene->GetSrcObjectCount<FbxAnimStack>();
    for (int as = 0; as < fbxScene->GetSrcObjectCount<FbxAnimStack>(); as++)
    {
        FbxAnimStack* animationStack = fbxScene->GetSrcObject<FbxAnimStack>(as);
        fbxScene->SetCurrentAnimationStack(animationStack);

        result.emplace_back();
        result.back().name = animationStack->GetName();

        int animationLayersCount = animationStack->GetMemberCount<FbxAnimLayer>();
        if (animationLayersCount > 0)
        {
            if (animationLayersCount > 1)
            {
                Logger::Warning("[FBXImporter] FBX animation '%s' contains more than one animation layer. Import only first (base) layer", animationStack->GetName());
            }

            FbxAnimLayer* animationLayer = animationStack->GetMember<FbxAnimLayer>(0);
            FBXAnimationImportDetails::ProcessNodeAnimationRecursive(fbxScene->GetRootNode(), animationLayer, &result.back().nodesAnimations);

            //Calculate duration
            float32& minTimeStamp = result.back().minTimeStamp;
            float32& maxTimeStamp = result.back().maxTimeStamp;
            for (auto& nodeAnimation : result.back().nodesAnimations)
            {
                for (auto& channelData : nodeAnimation.animationTrackData)
                {
                    if (!channelData.animationKeys.empty())
                    {
                        minTimeStamp = Min(minTimeStamp, channelData.animationKeys.front().time);
                        maxTimeStamp = Max(maxTimeStamp, channelData.animationKeys.back().time);
                    }
                }
            }
        }
    }

    return result;
}

void SaveAnimation(const FBXAnimationStackData& fbxStackAnimationData, const FilePath& filePath)
{
    using namespace FBXAnimationImportDetails;

    //binary file format described in 'AnimationBinaryFormat.md'
    struct ChannelHeader
    {
        //Track part
        uint8 target;
        uint8 pad0[3];

        //Channel part
        uint32 signature = AnimationChannel::ANIMATION_CHANNEL_DATA_SIGNATURE;
        uint8 dimension = 0;
        uint8 interpolation = 0;
        uint16 compression = 0;
        uint32 keyCount = 0;
    } channelHeader;

    ScopedPtr<File> file(File::Create(filePath, File::CREATE | File::WRITE));
    if (file)
    {
        Vector<uint8> animationData;

        float32 animationDuration = fbxStackAnimationData.maxTimeStamp - fbxStackAnimationData.minTimeStamp;
        WriteToBuffer(animationData, &animationDuration);

        uint32 nodeCount = uint32(fbxStackAnimationData.nodesAnimations.size());
        WriteToBuffer(animationData, &nodeCount);

        //Write nodes data
        for (auto& fbxNodeData : fbxStackAnimationData.nodesAnimations)
        {
            FbxNode* fbxNode = fbxNodeData.fbxNode;

            String nodeUID = GenerateNodeUID(fbxNode).c_str();
            String nodeName = fbxNode->GetName();

            WriteToBuffer(animationData, nodeUID);
            WriteToBuffer(animationData, nodeName);

            //Write Track data
            uint32 signature = AnimationTrack::ANIMATION_TRACK_DATA_SIGNATURE;
            WriteToBuffer(animationData, &signature);

            uint32 channelsCount = 0;
            for (auto& fbxChannelData : fbxNodeData.animationTrackData)
            {
                if (!fbxChannelData.animationKeys.empty())
                    ++channelsCount;
            }

            WriteToBuffer(animationData, &channelsCount);

            for (auto& fbxChannelData : fbxNodeData.animationTrackData)
            {
                if (!fbxChannelData.animationKeys.empty())
                {
                    channelHeader.target = fbxChannelData.channel;
                    channelHeader.keyCount = uint32(fbxChannelData.animationKeys.size());

                    if (channelHeader.target == AnimationTrack::CHANNEL_TARGET_POSITION)
                    {
                        channelHeader.dimension = 3;
                        channelHeader.interpolation = uint8(AnimationChannel::INTERPOLATION_LINEAR);
                    }
                    else if (channelHeader.target == AnimationTrack::CHANNEL_TARGET_ORIENTATION)
                    {
                        channelHeader.dimension = 4;
                        channelHeader.interpolation = uint8(AnimationChannel::INTERPOLATION_SPHERICAL_LINEAR);
                    }
                    else if (channelHeader.target == AnimationTrack::CHANNEL_TARGET_SCALE)
                    {
                        channelHeader.dimension = 1;
                        channelHeader.interpolation = uint8(AnimationChannel::INTERPOLATION_LINEAR);
                    }

                    WriteToBuffer(animationData, &channelHeader);

                    for (uint32 k = 0; k < channelHeader.keyCount; ++k)
                    {
                        const FBXAnimationKey& key = fbxChannelData.animationKeys[k];

                        float32 relativeKeyTime = key.time - fbxStackAnimationData.minTimeStamp;
                        WriteToBuffer(animationData, &relativeKeyTime);

                        if (channelHeader.target == AnimationTrack::CHANNEL_TARGET_POSITION)
                        {
                            Vector3 position = Vector3(key.value.data);
                            WriteToBuffer(animationData, &position);
                        }
                        else if (channelHeader.target == AnimationTrack::CHANNEL_TARGET_ORIENTATION)
                        {
                            Quaternion orientation = Quaternion(key.value.data);
                            orientation.Normalize();
                            WriteToBuffer(animationData, &orientation);
                        }
                        else if (channelHeader.target == AnimationTrack::CHANNEL_TARGET_SCALE)
                        {
                            float32 scale = key.value.x;
                            WriteToBuffer(animationData, &scale);
                        }
                    }
                }
            }
        }

        uint32 markerCount = 0;
        WriteToBuffer(animationData, &markerCount);

        uint32 animationDataSize = uint32(animationData.size());

        AnimationClip::FileHeader header;
        header.signature = AnimationClip::ANIMATION_CLIP_FILE_SIGNATURE;
        header.version = 1;
        header.crc32 = CRC32::ForBuffer(animationData.data(), animationDataSize);
        header.dataSize = animationDataSize;

        file->Write(&header);
        file->Write(animationData.data(), animationDataSize);

        file->Flush();
    }
    else
    {
        Logger::Error("[FBXImporter] Failed to open file for writing: %s", filePath.GetAbsolutePathname().c_str());
    }
}

}; //ns FBXImporterDetails

//////////////////////////////////////////////////////////////////////////
//FBXAnimationImportDetails namespace definitions

namespace FBXAnimationImportDetails
{
template <class T>
void WriteToBuffer(Vector<uint8>& buffer, const T* value)
{
    WriteToBuffer(buffer, value, sizeof(T));
}

void WriteToBuffer(Vector<uint8>& buffer, const void* data, uint32 size)
{
    DVASSERT(data != nullptr && size != 0);

    const uint8* bytes = reinterpret_cast<const uint8*>(data);
    buffer.insert(buffer.end(), bytes, bytes + size);
}

void WriteToBuffer(Vector<uint8>& buffer, const String& string)
{
    uint32 stringBytes = uint32(string.length() + 1);
    WriteToBuffer(buffer, string.c_str(), stringBytes);

    //as we load animation data directly to memory and use it without any processing we have to align strings data
    uint32 stringAlignment = 4 - (stringBytes & 0x3);
    if (stringAlignment > 0 && stringAlignment < 4)
    {
        uint32 pad = 0;
        WriteToBuffer(buffer, &pad, stringAlignment);
    }
}

Set<FbxTime> GetAnimationSamples(FbxPropertyT<FbxDouble3>& fbxProperty, FbxAnimLayer* fbxAnimLayer)
{
    FbxAnimCurve* fbxAnimCurve[3] = {
        fbxProperty.GetCurve(fbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X),
        fbxProperty.GetCurve(fbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y),
        fbxProperty.GetCurve(fbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z),
    };

    Set<FbxTime> samples;
    for (FbxAnimCurve* curve : fbxAnimCurve)
    {
        if (curve)
        {
            int keyCount = curve->KeyGetCount();
            for (int keyIndex = 0; keyIndex < keyCount; keyIndex++)
                samples.insert(curve->KeyGet(keyIndex).GetTime());
        }
    }

    return samples;
}

Vector<FBXImporterDetails::FBXAnimationKey> GetAnimationKeys(FbxNode* fbxNode, const Set<FbxTime>& keyTimes, AnimationTrack::eChannelTarget channel)
{
    using namespace FBXImporterDetails;

    Vector<FBXAnimationKey> animationKeys;

    for (const FbxTime& t : keyTimes)
    {
        animationKeys.emplace_back();
        animationKeys.back().time = float32(t.GetSecondDouble());

        if (channel == AnimationTrack::CHANNEL_TARGET_POSITION)
            animationKeys.back().value = Vector4(EvaluateNodeTransform(fbxNode, t).GetTranslationVector());
        else if (channel == AnimationTrack::CHANNEL_TARGET_ORIENTATION)
            animationKeys.back().value = Vector4(EvaluateNodeTransform(fbxNode, t).GetRotation().data);
        else if (channel == AnimationTrack::CHANNEL_TARGET_SCALE)
            animationKeys.back().value = Vector4(EvaluateNodeTransform(fbxNode, t).GetScaleVector());
    }

    return animationKeys;
}

FBXImporterDetails::FBXNodeAnimationData GetNodeAnimationData(FbxNode* fbxNode, FbxAnimLayer* fbxAnimLayer)
{
    Set<FbxTime> workSamples;

    FBXImporterDetails::FBXNodeAnimationData result;
    result.fbxNode = fbxNode;

    workSamples = GetAnimationSamples(fbxNode->LclTranslation, fbxAnimLayer);
    if (!workSamples.empty())
        result.animationTrackData.emplace_back(AnimationTrack::CHANNEL_TARGET_POSITION, GetAnimationKeys(fbxNode, workSamples, AnimationTrack::CHANNEL_TARGET_POSITION));

    workSamples = GetAnimationSamples(fbxNode->LclRotation, fbxAnimLayer);
    if (!workSamples.empty())
        result.animationTrackData.emplace_back(AnimationTrack::CHANNEL_TARGET_ORIENTATION, GetAnimationKeys(fbxNode, workSamples, AnimationTrack::CHANNEL_TARGET_ORIENTATION));

    workSamples = GetAnimationSamples(fbxNode->LclScaling, fbxAnimLayer);
    if (!workSamples.empty())
        result.animationTrackData.emplace_back(AnimationTrack::CHANNEL_TARGET_SCALE, GetAnimationKeys(fbxNode, workSamples, AnimationTrack::CHANNEL_TARGET_SCALE));

    return result;
}

void ProcessNodeAnimationRecursive(FbxNode* fbxNode, FbxAnimLayer* fbxAnimLayer, Vector<FBXImporterDetails::FBXNodeAnimationData>* outNodesAnimations)
{
    DVASSERT(fbxNode != nullptr);

    FBXImporterDetails::FBXNodeAnimationData nodeAnimationData = GetNodeAnimationData(fbxNode, fbxAnimLayer);
    if (!nodeAnimationData.animationTrackData.empty())
        outNodesAnimations->emplace_back(std::move(nodeAnimationData));

    int childrenCount = fbxNode->GetChildCount();
    for (int child = 0; child < childrenCount; child++)
        ProcessNodeAnimationRecursive(fbxNode->GetChild(child), fbxAnimLayer, outNodesAnimations);
}

}; //ns FBXAnimationImportDetails

}; //ns DAVA