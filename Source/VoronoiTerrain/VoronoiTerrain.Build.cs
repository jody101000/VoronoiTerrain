// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VoronoiTerrain : ModuleRules
{
	public VoronoiTerrain(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnableExceptions = true;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "RenderCore", "RHI", "Niagara" });
		
		PrivateDependencyModuleNames.AddRange(new string[] {"ProceduralMeshComponent", "GeometryFramework","GeometryScriptingCore", "DynamicMesh", "GeometryCore", "Niagara" });
	}
}
