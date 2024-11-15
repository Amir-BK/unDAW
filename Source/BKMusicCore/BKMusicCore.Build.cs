// Copyright Epic Games, Inc. All Rights Reserved.

using EpicGames.Core;
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
                "MetasoundGenerator", "MetasoundGraphCore", "MetasoundFrontend", "MetasoundEngine", "WaveTable"
                , "LevelSequence", "MovieScene", "MovieSceneTracks", "AudioWidgets"
			}

            );

		// Win or Macos support MIDI device
		if(Target.Platform == UnrealTargetPlatform.Win64 ||
			Target.Platform == UnrealTargetPlatform.Mac)
		{
			PublicDependencyModuleNames.AddRange(
			new string[]
			{
				//"MIDIDevice",
			}
			);

			PublicDefinitions.Add("WITH_MIDI_DEVICE");

		}

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

	static public void ConfigurePlugins(ModuleRules Rules, ReadOnlyTargetRules Target)
	{
		JsonObject RawObject;
		if (JsonObject.TryRead(Target.ProjectFile, out RawObject))
		{
			JsonObject[] pluginObjects;
			if (RawObject.TryGetObjectArrayField("Plugins", out pluginObjects))
			{
				foreach (JsonObject pluginObject in pluginObjects)
				{
					string pluginName;
					pluginObject.TryGetStringField("Name", out pluginName);

					bool pluginEnabled;
					pluginObject.TryGetBoolField("Enabled", out pluginEnabled);

					if (pluginName == "Chunreal" && pluginEnabled)
					{

							Rules.PublicDefinitions.Add("WITH_CHUNREAL_PLUGIN");
					}
				}
			}
		}
	}
}