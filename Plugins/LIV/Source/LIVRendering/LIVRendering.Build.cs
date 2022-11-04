// Copyright 2021 LIV Inc. - MIT License

using System.Collections.Generic;
using UnrealBuildTool;

public class LIVRendering : ModuleRules
{
	public LIVRendering(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
				
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"RHI",
				"RenderCore", 
                "Renderer"
            }
		);

		PrivateIncludePaths.AddRange(
			new string[]
            {
                EngineDirectory + "/Source/Runtime/Renderer/Private",
                EngineDirectory + "/Source/Runtime/Renderer/Private/PostProcess"
			}
        );
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Projects",
				"Slate",
				"SlateCore",
			}
		);
	}
}
