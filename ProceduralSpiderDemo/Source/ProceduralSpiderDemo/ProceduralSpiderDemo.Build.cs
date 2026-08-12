// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProceduralSpiderDemo : ModuleRules
{
	public ProceduralSpiderDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"ProceduralSpiderDemo",
			"ProceduralSpiderDemo/Variant_Horror",
			"ProceduralSpiderDemo/Variant_Horror/UI",
			"ProceduralSpiderDemo/Variant_Shooter",
			"ProceduralSpiderDemo/Variant_Shooter/AI",
			"ProceduralSpiderDemo/Variant_Shooter/UI",
			"ProceduralSpiderDemo/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
