// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UnrealBlackholio : ModuleRules
{
	public UnrealBlackholio(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "SpacetimeDB" });
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "SpacetimeDB" });
	}
}
