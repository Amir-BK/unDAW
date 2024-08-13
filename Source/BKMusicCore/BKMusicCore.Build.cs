// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class BKMusicCore : ModuleRules
{
    private bool bStrictIncludesCheck = true;

    public BKMusicCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false;
        CppStandard = CppStandardVersion.Cpp20;

        PrivateIncludePaths.AddRange(
                        new string[] {
                    Path.Combine(GetModuleDirectory("MetasoundFrontend"), "Private"),
                          }
        );

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

        // This is to emulate engine installation and verify includes during development
        // Gives effect similar to BuildPlugin with -StrictIncludes
        if (bStrictIncludesCheck)
        {
            bUseUnity = false;
            PCHUsage = PCHUsageMode.NoPCHs;
            // Enable additional checks used for Engine modules
            bTreatAsEngineModule = true;
        }

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
                "unDAWMetaSounds","MetasoundGenerator", "MetasoundGraphCore", "MetasoundFrontend", "MetasoundEngine", "WaveTable"
                ,"MIDIDevice"
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
                "SignalProcessing"
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