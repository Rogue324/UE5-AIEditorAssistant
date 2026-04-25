using UnrealBuildTool;

public class AIEditorAssistant : ModuleRules
{
    public AIEditorAssistant(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "ApplicationCore",
            "DesktopPlatform",
            "DeveloperSettings",
            "EditorStyle",
            "HTTP",
            "ImageWrapper",
            "InputCore",
            "Json",
            "JsonUtilities",
            "Projects",
            "Slate",
            "SlateCore",
            "SoftUEBridge",
            "ToolMenus",
            "UnrealEd"
        });
    }
}
