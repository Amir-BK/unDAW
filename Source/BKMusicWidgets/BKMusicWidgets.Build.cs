// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BKMusicWidgets : ModuleRules
{
    public BKMusicWidgets(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        bUseUnity = false;

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
                "AppFramework", "Boost", "Harmonix", "HarmonixMidi", "HarmonixDsp", "MetasoundFrontend",
                "BKMusicCore", "CommonUI"
            }
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UMG",
                "Projects",
                "InputCore"
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