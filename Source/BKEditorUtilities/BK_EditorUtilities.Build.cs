// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BK_EditorUtilities : ModuleRules
{
	public BK_EditorUtilities(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Json",
				"JsonUtilities",
                "AudioMixer",
                "AppFramework",
                "BKMusicCore",
                "AssetTools",
                "HarmonixMidi",
                "Harmonix", "Blutility", "BKMusicWidgets", "HarmonixDsp",
                "XmlParser", "FileUtilities", "MetasoundFrontend", "MetasoundEngine",
				"BKMusicCore", "ScriptableEditorWidgets", "EditorScriptingUtilities", "AssetTools", 
				"DetailCustomizations", "EditorWidgets", "EditorSubsystem", "AudioExtensions", "unDAWMetaSounds",
				"GraphEditor", "AudioWidgets"
            

                // ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
                "UnrealEd",
				"Engine",
				"Slate",
				"SlateCore",
				"UMG",
                "UMGEditor",
                "Projects",
                "Blutility",
                "InputCore",
                "HarmonixDspEditor",
                "MetasoundEditor"

				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
