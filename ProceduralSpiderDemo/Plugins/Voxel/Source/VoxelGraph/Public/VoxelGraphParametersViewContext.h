// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelMinimal.h"
#include "VoxelParameterOverridesOwner.h"

class UVoxelGraph;
class FVoxelGraphParametersViewBase;
struct FVoxelParameterValueOverride;

class VOXELGRAPH_API FVoxelGraphParametersViewContext : public TSharedFromThis<FVoxelGraphParametersViewContext>
{
public:
	FVoxelGraphParametersViewContext() = default;

public:
	const FVoxelParameterValueOverride* FindValue(const FGuid& Guid) const;
	FVoxelParameterValueOverride* FindValueOverride(const FGuid& Guid) const;
	const FVoxelParameterValueOverride* FindDefaultValue(const FGuid& Guid) const;

public:
	void ForceEnableOverrides()
	{
		ensure(!bForceEnableMainOverrides);
		bForceEnableMainOverrides = true;
	}

	void DisableMainOverridesOwner()
	{
		ensure(!bSkipMainOverridesOwner);
		bSkipMainOverridesOwner = true;
	}
	void EnableMainOverridesOwner()
	{
		ensure(bSkipMainOverridesOwner);
		bSkipMainOverridesOwner = false;
	}

public:
	FORCEINLINE const FVoxelParameterOverridesOwnerPtr& GetOverridesOwner()
	{
		return MainOverridesOwner;
	}

private:
	FVoxelParameterOverridesOwnerPtr MainOverridesOwner;
	bool bForceEnableMainOverrides = false;
	bool bSkipMainOverridesOwner = false;

	// Highest to lowest priority (graph, then its parent, then parent's parent etc)
	TVoxelInlineArray<TVoxelObjectPtr<const UVoxelGraph>, 1> Graphs;

	friend class IVoxelParameterOverridesOwner;
};