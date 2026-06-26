using UnrealBuildTool;

public class TowerDefenseIslander : ModuleRules
{
	public TowerDefenseIslander(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivatePCHHeaderFile = "TowerDefenseIslander.h";

		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"Paper2D",
			"UMG",
			"Slate",
			"SlateCore",
			"GameplayTags",
			"AIModule",
			"NavigationSystem",
			"GameplayTasks",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"GameplayDebugger",
		});
	}
}
