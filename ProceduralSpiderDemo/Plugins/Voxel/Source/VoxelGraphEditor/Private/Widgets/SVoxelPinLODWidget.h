// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#pragma once

#include "VoxelEditorMinimal.h"
#include "SLevelOfDetailBranchNode.h"

class VOXELGRAPHEDITOR_API SVoxelPinLODWidget : public SLevelOfDetailBranchNode
{
public:
	//~ Begin SLevelOfDetailBranchNode Interface
	virtual FVector2D ComputeDesiredSize(const float LayoutScaleMultiplier) const override
	{
		if (LastCachedValue == 0)
		{
			DesiredSize = SLevelOfDetailBranchNode::ComputeDesiredSize(LayoutScaleMultiplier);
		}

		return DesiredSize;
	}
	//~ End SLevelOfDetailBranchNode Interface

private:
	mutable FVector2D DesiredSize = FVector2D(20.f);
};