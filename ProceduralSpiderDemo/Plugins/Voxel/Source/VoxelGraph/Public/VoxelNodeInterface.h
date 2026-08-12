// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

struct FVoxelNode;
struct FVoxelGraphMergedNodeRef;

struct VOXELGRAPH_API IVoxelNodeInterface
{
public:
	IVoxelNodeInterface() = default;
	virtual ~IVoxelNodeInterface() = default;

	virtual const FVoxelGraphMergedNodeRef& GetNodeRef() const = 0;

public:
	static void RaiseBufferErrorStatic(const FVoxelGraphMergedNodeRef& Node);

	void RaiseBufferError() const
	{
		RaiseBufferErrorStatic(GetNodeRef());
	}
};

template<typename T>
requires std::derived_from<T, IVoxelNodeInterface>
struct TVoxelMessageTokenFactory<T>
{
	static TSharedRef<FVoxelMessageToken> CreateToken(const T& Node)
	{
		return Node.GetNodeRef().CreateMessageToken();
	}
};