// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LearningC : ModuleRules
{
	public LearningC(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate",
			"Modular_Inventory_System"
		});

        PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"LearningC",
			"LearningC/Variant_Platforming",
			"LearningC/Variant_Platforming/Animation",
			"LearningC/Variant_Combat",
			"LearningC/Variant_Combat/AI",
			"LearningC/Variant_Combat/Animation",
			"LearningC/Variant_Combat/Gameplay",
			"LearningC/Variant_Combat/Interfaces",
			"LearningC/Variant_Combat/UI",
			"LearningC/Variant_SideScrolling",
			"LearningC/Variant_SideScrolling/AI",
			"LearningC/Variant_SideScrolling/Gameplay",
			"LearningC/Variant_SideScrolling/Interfaces",
			"LearningC/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
