using UnrealBuildTool;
using System.Collections.Generic;

public class TowerDefenseIslanderEditorTarget : TargetRules
{
	public TowerDefenseIslanderEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5Latest;
		ExtraModuleNames.Add("TowerDefenseIslander");
	}
}
