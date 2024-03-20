// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class unDAWMetaSounds : ModuleRules
{
	public unDAWMetaSounds(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// This next flag is needed as there are Metasound node derivatives 
		// that are implemented in .cpp files only... no header files. 
		IWYUSupport = IWYUSupport.None;

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
	
				"Engine",
				"Core",
				"CoreUObject",
				"MetasoundStandardNodes",
				"HarmonixDsp",
				"HarmonixMidi",
				"Harmonix",
				
				
				
			 
				// ... add other public dependencies that you statically link with here ...
			}


			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"SignalProcessing",
				"MetasoundEngine",
				"MetasoundGraphCore",
				"MetasoundFrontend",
				"MetasoundGenerator",
				"AudioExtensions",
				"HarmonixMetasound"
				


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
