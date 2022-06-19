#pragma once

#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"

#define FBXSDK_SHARED //requested only for dynamic linking

#include <fbxsdk.h>

namespace DAVA
{
namespace FBXImporterDetails
{
Matrix4 ToMatrix4(const FbxAMatrix& fbxMatrix);
Vector3 ToVector3(const FbxVector4& fbxVector);
Vector2 ToVector2(const FbxVector2& fbxVector);

FastName GenerateNodeUID(const FbxNode* node);
void ClearNodeUIDCache();

const char* GetFBXTexturePath(const FbxProperty& textureProperty);

FbxAMatrix GetGeometricTransform(const FbxNode* fbxNode);
Matrix4 EvaluateNodeTransform(FbxNode* fbxNode, const FbxTime& time = FBXSDK_TIME_INFINITE);

template <class Type>
Type GetFbxMeshLayerElementValue(const FbxLayerElementTemplate<Type>* element, int32 controlPointIndex, int32 polygonIndex, int32 vertexIndex)
{
    DVASSERT(element != nullptr);

    FbxLayerElement::EMappingMode mappingMode = element->GetMappingMode();
    FbxLayerElement::EReferenceMode referenceMode = element->GetReferenceMode();

    int32 elementIndex = -1;
    if (mappingMode == FbxGeometryElement::eByControlPoint)
    {
        elementIndex = (referenceMode == FbxGeometryElement::eDirect) ? controlPointIndex : element->GetIndexArray().GetAt(controlPointIndex);
    }
    else if (mappingMode == FbxGeometryElement::eByPolygonVertex)
    {
        int32 index = polygonIndex * 3 + vertexIndex;
        elementIndex = (referenceMode == FbxGeometryElement::eDirect) ? index : element->GetIndexArray().GetAt(index);
    }
    else if (mappingMode == FbxGeometryElement::eByPolygon)
    {
        elementIndex = (referenceMode == FbxGeometryElement::eDirect) ? polygonIndex : element->GetIndexArray().GetAt(polygonIndex);
    }

    return element->GetDirectArray().GetAt(elementIndex);
}
};
};