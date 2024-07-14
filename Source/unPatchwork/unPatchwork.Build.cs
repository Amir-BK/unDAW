// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class unPatchwork : ModuleRules
{
    private bool bStrictIncludesCheck = true;

    public unPatchwork(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false;

        // This is to emulate engine installation and verify includes during development
        // Gives effect similar to BuildPlugin with -StrictIncludes
        if (bStrictIncludesCheck)
        {
            bUseUnity = false;
            PCHUsage = PCHUsageMode.NoPCHs;
            // Enable additional checks used for Engine modules
            bTreatAsEngineModule = true;
        }

        PublicIncludePaths.AddRange(
            new string[] {
				// ... add public include paths required here ...
				//engine_path + "Plugins/Runtime/Metasound/Source/MetasoundEngine/",
				//engine_path + "Plugins/Runtime/Metasound/Source/MetasoundEngine/Private/",
			}
            );

        PrivateIncludePaths.AddRange(
            new string[] {
				// ... add other private include paths required here ...
			}
            );

        //PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "../ThirdParty/Sfizz/"));

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
				//"Json",

                "AudioMixer",
                "DeveloperSettings",
                "MetasoundEngine",
                "MetasoundGraphCore",
                "MetasoundFrontend",
                "AudioExtensions", "HarmonixDsp", "HarmonixMetasound", "Harmonix", "HarmonixMidi",
                "BKMusicCore"

                //probably don't want to depend on music widget, keep the dependency one directional from widgets -> core
				// ... add other public dependencies that you statically link with here ...
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
				//"Projects",
				//"InputCore",
				"SignalProcessing"

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