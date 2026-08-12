// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class UVoxelTerminalGraph;
struct FVoxelGraphCompileScope;

extern VOXELGRAPH_API FVoxelGraphCompileScope* GVoxelGraphCompileScope;

struct VOXELGRAPH_API FVoxelGraphCompileScope
{
public:
	const UVoxelTerminalGraph& TerminalGraph;

	explicit FVoxelGraphCompileScope(
		const UVoxelTerminalGraph& TerminalGraph,
		bool bEnableLogging = true);
	~FVoxelGraphCompileScope();

	bool HasError() const
	{
		return bHasError;
	}
	const TVoxelArray<TSharedRef<FVoxelMessage>>& GetMessages() const
	{
		return Messages;
	}

private:
	FVoxelGraphCompileScope* PreviousScope = nullptr;
	bool bHasError = false;
	TVoxelArray<TSharedRef<FVoxelMessage>> Messages;
	TUniquePtr<FVoxelScopedMessageConsumer> ScopedMessageConsumer;
};