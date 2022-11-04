// Copyright 2021 LIV Inc. - MIT License
using UnrealBuildTool;

public class LIVEditor : ModuleRules
{
    public LIVEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", 
                "AppFramework",
                "EditorStyle", 
                "HTTP"
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Analytics",
                "ComponentVisualizers",
                "CoreUObject",
                "Engine",
                "InputCore",
                "Json",
                "JsonUtilities",
                "LIV",
                "Projects",
                "Slate",
                "SlateCore",
                "SourceControl",
                "RenderCore",
                "RHI",
                "ToolMenus",
                "UnrealEd",
                "WorkspaceMenuStructure", 
                "MainFrame", 
                "GameProjectGeneration"
            }
        );
    }
}
