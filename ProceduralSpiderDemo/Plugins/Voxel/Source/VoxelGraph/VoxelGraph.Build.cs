// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

using UnrealBuildTool;

public class VoxelGraph : ModuleRules
{
	public VoxelGraph(ReadOnlyTargetRules Target) : base(Target)
	{
		SetupVoxelModule(this);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"TraceLog",
			"Chaos",
			"Slate",
			"SlateCore",
			"PhysicsCore",
			"Json",
		});

		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange(new string[]
			{
				"MessageLog",
			});
		}
	}
}