// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

#include "VoxelMinimal.h"
#include "VoxelNodeStats.h"
#include "VoxelGraphTracker.h"
#include "VoxelTerminalGraph.h"

VOXEL_RUN_ON_STARTUP_GAME()
{
	Voxel::OnRefreshAll.AddLambda([]
	{
#if WITH_EDITOR
		for (IVoxelNodeStatProvider* Provider : GVoxelNodeStatProviders)
		{
			Provider->ClearStats();
		}

		ForEachObjectOfClass_Copy<UVoxelTerminalGraph>([&](UVoxelTerminalGraph& TerminalGraph)
		{
			if (TerminalGraph.IsTemplate())
			{
				return;
			}

			GVoxelGraphTracker->NotifyEdGraphChanged(TerminalGraph.GetEdGraph());
		});
#endif
	});
}