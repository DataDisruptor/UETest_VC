// Copyright 2021 LIV Inc. - MIT License
using System.IO;
using UnrealBuildTool;

public class LivSDK : ModuleRules
{
	public LivSDK(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			string Configuration = "Release";
			string LibraryName = "LIV.lib";

            // use "RelWithDebInfo" cmake configuration when using a ue4 debug configuration
			if (Target.Configuration == UnrealTargetConfiguration.Debug || Target.Configuration == UnrealTargetConfiguration.DebugGame)
            {
                // we only distribute release publicly so much check if the configuration exists
                if (Directory.Exists(Path.Combine(ModuleDirectory, "RelWithDebInfo")))
                {
                    Configuration = "RelWithDebInfo";
                    LibraryName = "LIV.lib";
				}
            }

			// Add the import library
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, Configuration, "lib", LibraryName));

			// Add the headers
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, Configuration, "include"));
		}
	}
}
