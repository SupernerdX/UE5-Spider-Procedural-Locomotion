// Copyright Voxel Plugin SAS, 2026. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Voxel : ModuleRules
{
	public Voxel(ReadOnlyTargetRules Target) : base(Target)
	{
		SetupVoxelModule(this);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"VoxelGraph",
			"Chaos",
			"Renderer",
			"Slate",
			"SlateCore",
			"PhysicsCore",
			"Landscape",
			"NavigationSystem",
			"PCG",
			"MeshDescription",
			"StaticMeshDescription",
		});

		PrivateIncludePaths.AddRange(new string[]
		{
			Path.Combine(GetModuleDirectory("Engine"), "Private/"),
			Path.Combine(GetModuleDirectory("Renderer"), "Private"),
			Path.Combine(GetModuleDirectory("Renderer"), "Internal")
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { "NaniteBuilder" });
		}
	}
}