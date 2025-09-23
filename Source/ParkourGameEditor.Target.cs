using UnrealBuildTool;
using System.Collections.Generic;

public class ParkourGameEditorTarget : TargetRules
{
	public ParkourGameEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "ParkourGame" } );
	}
}
