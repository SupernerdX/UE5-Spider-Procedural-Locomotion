// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

using UnrealBuildTool;

public class VoxelBlueprint : ModuleRules
{
    public VoxelBlueprint(ReadOnlyTargetRules Target) : base(Target)
    {
	    SetupVoxelModule(this);

	    PublicDependencyModuleNames.AddRange(new string[]
	    {
		    "VoxelCoreEditor",
		    "Voxel",
		    "VoxelEditor",
		    "VoxelGraph",
		    "VoxelGraphEditor",
		    "Slate",
		    "SlateCore",
		    "ToolMenus",
		    "GraphEditor",
		    "BlueprintGraph",
		    "KismetCompiler",
	    });
    }
}