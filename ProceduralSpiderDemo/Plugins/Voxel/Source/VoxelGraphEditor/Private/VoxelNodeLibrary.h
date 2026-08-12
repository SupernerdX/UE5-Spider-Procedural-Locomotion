// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "VoxelNode.h"

struct FVoxelNodeLibrary
{
public:
	FVoxelNodeLibrary();

public:
	TConstVoxelArrayView<TSharedRef<const FVoxelNode>> GetNodes();

	TSharedPtr<const FVoxelNode> FindMakeNode(const FVoxelPinType& Type);
	TSharedPtr<const FVoxelNode> FindBreakNode(const FVoxelPinType& Type);
	TSharedPtr<const FVoxelNode> FindCastNode(const FVoxelPinType& From, const FVoxelPinType& To) const;

	TSharedPtr<const FVoxelNode> FindNode(const UScriptStruct* Struct) const;
	TSharedPtr<const FVoxelNode> FindNode(const UFunction* Function) const;

public:
	template<typename T>
	TSharedPtr<const T> GetNodeInstance()
	{
		return StaticCastSharedPtr<const T>(this->FindNode(T::StaticStruct()));
	}

private:
	TVoxelArray<TSharedRef<const FVoxelNode>> Nodes;
	TVoxelMap<FVoxelPinType, TSharedPtr<const FVoxelNode>> TypeToMakeNode;
	TVoxelMap<FVoxelPinType, TSharedPtr<const FVoxelNode>> TypeToBreakNode;
	TVoxelMap<TPair<FVoxelPinType, FVoxelPinType>, TSharedPtr<const FVoxelNode>> FromTypeAndToTypeToCastNode;
	TVoxelMap<const UScriptStruct*, TSharedPtr<const FVoxelNode>> StructToNode;
	TVoxelMap<const UFunction*, TSharedPtr<const FVoxelNode>> FunctionToNode;
};
extern FVoxelNodeLibrary* GVoxelNodeLibrary;