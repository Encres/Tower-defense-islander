using UnrealBuildTool;
using System.Collections.Generic;

public class TowerDefenseIslanderTarget : TargetRules
{
	public TowerDefenseIslanderTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5Latest;
		ExtraModuleNames.Add("TowerDefenseIslander");
	}
}
