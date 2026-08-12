// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "K2Node_GetVoxelGraphParameter.h"
#include "Graphs/VoxelHeightGraph.h"
#include "Graphs/VoxelVolumeGraph.h"
#include "Graphs/VoxelParameterBlueprintLibrary.h"
#include "Sculpt/Height/VoxelHeightSculptGraph.h"
#include "Sculpt/Volume/VoxelVolumeSculptGraph.h"

UK2Node_GetVoxelHeightGraphParameter::UK2Node_GetVoxelHeightGraphParameter()
{
	FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(UVoxelParameterBlueprintLibrary, K2_GetVoxelHeightGraphParameter),
		UVoxelParameterBlueprintLibrary::StaticClass());
}

bool UK2Node_GetVoxelHeightGraphParameter::IsPinWildcard(const UEdGraphPin& Pin) const
{
	return Pin.PinName == "Value";
}

UEdGraphPin* UK2Node_GetVoxelHeightGraphParameter::GetParameterNamePin() const
{
	return FindPin(TEXT("Name"));
}

UClass* UK2Node_GetVoxelHeightGraphParameter::GetGraphClassType() const
{
	return UVoxelHeightGraph::StaticClass();
}

FName UK2Node_GetVoxelHeightGraphParameter::GetGraphPinName() const
{
	return "Graph";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UK2Node_GetVoxelVolumeGraphParameter::UK2Node_GetVoxelVolumeGraphParameter()
{
	FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(UVoxelParameterBlueprintLibrary, K2_GetVoxelVolumeGraphParameter),
		UVoxelParameterBlueprintLibrary::StaticClass());
}

bool UK2Node_GetVoxelVolumeGraphParameter::IsPinWildcard(const UEdGraphPin& Pin) const
{
	return Pin.PinName == "Value";
}

UEdGraphPin* UK2Node_GetVoxelVolumeGraphParameter::GetParameterNamePin() const
{
	return FindPin(TEXT("Name"));
}

UClass* UK2Node_GetVoxelVolumeGraphParameter::GetGraphClassType() const
{
	return UVoxelVolumeGraph::StaticClass();
}

FName UK2Node_GetVoxelVolumeGraphParameter::GetGraphPinName() const
{
	return "Graph";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UK2Node_GetVoxelHeightSculptGraphParameter::UK2Node_GetVoxelHeightSculptGraphParameter()
{
	FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(UVoxelParameterBlueprintLibrary, K2_GetVoxelHeightSculptGraphParameter),
		UVoxelParameterBlueprintLibrary::StaticClass());
}

bool UK2Node_GetVoxelHeightSculptGraphParameter::IsPinWildcard(const UEdGraphPin& Pin) const
{
	return Pin.PinName == "Value";
}

UEdGraphPin* UK2Node_GetVoxelHeightSculptGraphParameter::GetParameterNamePin() const
{
	return FindPin(TEXT("Name"));
}

UClass* UK2Node_GetVoxelHeightSculptGraphParameter::GetGraphClassType() const
{
	return UVoxelHeightSculptGraph::StaticClass();
}

FName UK2Node_GetVoxelHeightSculptGraphParameter::GetGraphPinName() const
{
	return "Graph";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UK2Node_GetVoxelVolumeSculptGraphParameter::UK2Node_GetVoxelVolumeSculptGraphParameter()
{
	FunctionReference.SetExternalMember(
		GET_FUNCTION_NAME_CHECKED(UVoxelParameterBlueprintLibrary, K2_GetVoxelVolumeSculptGraphParameter),
		UVoxelParameterBlueprintLibrary::StaticClass());
}

bool UK2Node_GetVoxelVolumeSculptGraphParameter::IsPinWildcard(const UEdGraphPin& Pin) const
{
	return Pin.PinName == "Value";
}

UEdGraphPin* UK2Node_GetVoxelVolumeSculptGraphParameter::GetParameterNamePin() const
{
	return FindPin(TEXT("Name"));
}

UClass* UK2Node_GetVoxelVolumeSculptGraphParameter::GetGraphClassType() const
{
	return UVoxelVolumeSculptGraph::StaticClass();
}

FName UK2Node_GetVoxelVolumeSculptGraphParameter::GetGraphPinName() const
{
	return "Graph";
}