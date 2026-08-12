// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class VoxelEditor : ModuleRules
{
    public VoxelEditor(ReadOnlyTargetRules Target) : base(Target)
    {
	    SetupVoxelModule(this);

	    // For FDetailCategoryImpl
	    PrivateIncludePaths.Add(Path.Combine(GetModuleDirectory("PropertyEditor"), "Private"));

	    PrivateDependencyModuleNames.AddRange(new string[]
	    {
		    "Voxel",
		    "VoxelPCG",
		    "VoxelGraph",
		    "VoxelGraphEditor",
		    "EditorWidgets",
		    "InteractiveToolsFramework",
		    "EditorInteractiveToolsFramework",
		    "SceneOutliner",
		    "LevelEditor",
		    "ToolMenus",
		    "AssetTools",
		    "GraphEditor",
		    "KismetWidgets",
		    "BlueprintGraph",
		    "DetailCustomizations",
		    "PCG",
		    "WorkspaceMenuStructure",
		    "HTTP",
		    "Json",
		    "PlacementMode",
		    "ApplicationCore",
		    "StatusBar",
		    "TraceLog",
		    "Projects",
			"ImageCore",
			"ToolWidgets",
		    "SettingsEditor",
		    "EditorSubsystem",
	    });
    }
}