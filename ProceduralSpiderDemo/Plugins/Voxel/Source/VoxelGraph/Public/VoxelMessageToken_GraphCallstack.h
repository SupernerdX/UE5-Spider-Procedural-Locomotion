// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelMessage.h"
#include "VoxelGraphNodeRef.h"
#include "Widgets/SCompoundWidget.h"
#include "VoxelMessageToken_GraphCallstack.generated.h"

struct FVoxelGraphCallstack;

class VOXELGRAPH_API FVoxelGraphSharedCallstack : public TSharedFromThis<FVoxelGraphSharedCallstack>
{
public:
	explicit FVoxelGraphSharedCallstack(const FVoxelGraphCallstack& Callstack);

	uint32 GetHash() const;
	FString ToDebugString() const;

	const FVoxelGraphMergedNodeRef& GetNodeRef() const
	{
		return NodeRef;
	}
	const TSharedPtr<const FVoxelGraphSharedCallstack>& GetParent() const
	{
		return Parent;
	}
	FORCEINLINE bool IsDebugNode() const
	{
		return bDebugNode;
	}

private:
	FVoxelGraphMergedNodeRef NodeRef;
	TSharedPtr<const FVoxelGraphSharedCallstack> Parent;
	bool bDebugNode = false;
};

USTRUCT()
struct VOXELGRAPH_API FVoxelMessageToken_GraphCallstack : public FVoxelMessageToken
{
	GENERATED_BODY()
	GENERATED_VIRTUAL_STRUCT_BODY()

public:
	TSharedPtr<const FVoxelGraphSharedCallstack> Callstack;
	TSharedPtr<const FVoxelMessage> Message;

	//~ Begin FVoxelMessageToken Interface
	virtual uint32 GetHash() const override;
	virtual FString ToString() const override;
	virtual TSharedRef<IMessageToken> GetMessageToken() const override;
	//~ End FVoxelMessageToken Interface
};