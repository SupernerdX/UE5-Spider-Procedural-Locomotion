// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"

class UVoxelGraph;
class FVoxelParameterView;
class FVoxelGraphParametersViewContext;

class VOXELGRAPH_API FVoxelGraphParametersView
{
public:
	FORCEINLINE FVoxelGraphParametersViewContext& GetContext() const
	{
		return *Context;
	}
	FORCEINLINE TConstVoxelArrayView<FVoxelParameterView*> GetChildren() const
	{
		return Children;
	}
	FORCEINLINE FVoxelParameterView* FindByGuid(const FGuid& Guid) const
	{
		return GuidToParameterView.FindRef(Guid);
	}
	FORCEINLINE FVoxelParameterView* FindByName(const FName Name) const
	{
		return NameToParameterView.FindRef(Name);
	}

	UE_NONCOPYABLE(FVoxelGraphParametersView);

public:
	static TVoxelArray<TVoxelArray<FVoxelParameterView*>> GetCommonChildren(TConstVoxelArrayView<TSharedPtr<FVoxelGraphParametersView>> ParametersViews);

private:
	const TSharedRef<FVoxelGraphParametersViewContext> Context;

	TVoxelArray<FVoxelParameterView> ChildrenRefs;
	TVoxelArray<FVoxelParameterView*> Children;
	TVoxelMap<FGuid, FVoxelParameterView*> GuidToParameterView;
	TVoxelMap<FName, FVoxelParameterView*> NameToParameterView;

	FVoxelGraphParametersView();

	friend class IVoxelParameterOverridesOwner;
};