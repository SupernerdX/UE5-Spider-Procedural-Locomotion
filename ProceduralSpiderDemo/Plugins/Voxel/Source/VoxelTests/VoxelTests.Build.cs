// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

using UnrealBuildTool;

public class VoxelTests : ModuleRules
{
    public VoxelTests(ReadOnlyTargetRules Target) : base(Target)
    {
	    SetupVoxelModule(this);

	    if (new VoxelConfig(this).DevWorkflow)
	    {
			// Needed by include testing
		    PrivatePCHHeaderFile = "Private/VoxelTestPCH.h";
	    }

	    PrivateDependencyModuleNames.AddRange(new string[]
	    {
		    "NavigationSystem"
	    });
	    PublicDependencyModuleNames.AddRange(new string[]
	    {
		    "Json"
	    });
    }
}