// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Modular_Inventory_System : ModuleRules
{
	public Modular_Inventory_System(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);


        PublicDependencyModuleNames.AddRange(new string[] {
		"Core",
		"CoreUObject",
		"Engine",
		"UMG",
		"Slate",
		"SlateCore"
		});
	
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
