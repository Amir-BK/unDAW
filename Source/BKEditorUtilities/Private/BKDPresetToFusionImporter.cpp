// Fill out your copyright notice in the Description page of Project Settings.


#include "BKDPresetToFusionImporter.h"
//#include "XmlParser.h"
#include "XmlFile.h"
#include "FileUtilities/ZipArchiveReader.h"
#include <XmlNode.h>
#include <AssetToolsModule.h>
//#include "HarmonixDspEditor/Private/FusionPatchImportOptions.h"



UObject* UBKDPresetToFusionImporter::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	
	
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
	newSettings = FFusionPatchSettings();
	FString PathString = FPaths::GetPath(*Filename);
	UE_LOG(LogTemp, Log, TEXT("Path String:  %s"), *PathString)
	const FString& path = Filename;
	FString UnrealSampleSavePath = TEXT("/Game/Imported/SFZSamples/");
	FPaths::MakePlatformFilename(UnrealSampleSavePath);

	FXmlFile MyXMLfile = FXmlFile();

	FName newName = FName(InName.ToString().Append(TEXT("_DSPRESET")));

	UFusionPatch* NewFusionPatch = NewObject<UFusionPatch>(InParent, InClass, InName, Flags);
	
	MyXMLfile.LoadFile(path);
	//MyXMLfile.

	traverseXML(MyXMLfile.GetRootNode(), KeyZoneArray, newSettings);

	int NumFilesFailedToImport = 0;

	FScopedSlowTask ParseDSPresetRegionsTask(KeyZoneArray.Num(), FText::FromString(TEXT("Processing DSPreset file and importing samples")));

	ParseDSPresetRegionsTask.MakeDialog();


	for (auto& keyzone : KeyZoneArray) {
		ParseDSPresetRegionsTask.EnterProgressFrame();
		FString sampleToImportPath = keyzone.SamplePath;
		sampleToImportPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(PathString + "/" + sampleToImportPath));
		FPaths::MakeStandardFilename(sampleToImportPath);
		FPaths::NormalizeFilename(sampleToImportPath);
		
	
		FString ResultPath = FPaths::Combine(
			UnrealSampleSavePath + FPaths::MakeValidFileName(keyzone.SamplePath));
		if (FPaths::FileExists(sampleToImportPath))
		{
			TArray<FString> fileToImport;
			fileToImport.Add(sampleToImportPath);

			UAutomatedAssetImportData* newSample = NewObject<UAutomatedAssetImportData>();

			newSample->DestinationPath = ResultPath;
			newSample->Filenames = fileToImport;
			newSample->bReplaceExisting = true;

			FString Extension = FPaths::GetExtension(ResultPath);
			GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(
				this, USoundWave::StaticClass(), this, FName(*fileToImport[0]), *Extension);

			auto ResultArray = AssetToolsModule.Get().ImportAssetsAutomated(newSample);
			if (!ResultArray.IsEmpty())
			{
				keyzone.SoundWave = Cast<USoundWave>(ResultArray[0]);
				//Region->ObjectPtrWavAsset = TObjectPtr<USoundWave>(Region->WavAsset);
				//Factory->ImportedWavsMap.Add(FName(value), Region->WavAsset);
			}
			else
			{
				NumFilesFailedToImport++;
			}
			GEditor->GetEditorSubsystem<UImportSubsystem>()->
				BroadcastAssetPostImport(this, keyzone.SoundWave);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("No sample cause the path looks like this: %s"), *sampleToImportPath)
			NumFilesFailedToImport++;
		}

		


	}
	
	//UFusionPatchCreateOptions::FArgs Args;
	//bool WasOkayPressed = false;

	//UFusionPatchCreateOptions::GetWithDialog(MoveTemp(Args), WasOkayPressed);

	newSettings.Adsr->Target = EAdsrTarget::Volume;
	newSettings.Adsr->SustainLevel = 1.0f;
	newSettings.Adsr->IsEnabled = true;

	//newSettings.KeyzoneSelectMode = EKeyzoneSelectMode::Layers;



	newSettings.Adsr->Calculate();
	NewFusionPatch->UpdateSettings(newSettings);
	NewFusionPatch->UpdateKeyzones(KeyZoneArray);


	//if(MyXMLfile.GetRotNoode()) 

	
	return NewFusionPatch;
}

UBKDPresetToFusionImporter::UBKDPresetToFusionImporter()
{
	//Formats.Add(FString(TEXT("sfz;")) + NSLOCTEXT("USFZAssetFactory", "FormatSfz", "SFZ File").ToString());
	Formats.Add(TEXT("dspreset;Decent Sampler Preset"));
	SupportedClass = UFusionPatch::StaticClass();
	bCreateNew = false;
	bEditorImport = true;
	//Octave_Offset = 12;
}


void UBKDPresetToFusionImporter::traverseXML(const FXmlNode* nodeIn, TArray <FKeyzoneSettings>& workingArray, FFusionPatchSettings& workingSettings)
{
	if (nodeIn != nullptr)
	{
		//UE_LOG(LogTemp, Log, TEXT("Content: %s, Tag: %s"), *nodeIn->GetContent(), *nodeIn->GetTag());

		//parse groups
		if (nodeIn->GetTag().Contains(TEXT("group")))
		{
			//currentGroup = nodeIn;
			for (auto& attribute : nodeIn->GetAttributes())
			{
				//UE_LOG(LogTemp, Log, TEXT("This is a group: %s, Value %s"), *attribute.GetTag(), *attribute.GetValue());
				if (attribute.GetTag().Contains(TEXT("seqMode")))
				{
					UE_LOG(LogTemp, Log, TEXT("seqMode %s"), *attribute.GetValue());
					if (attribute.GetValue().Contains(TEXT("random")))
					{
						workingSettings.KeyzoneSelectMode = EKeyzoneSelectMode::Random;
					}
					continue;
				};

			};
		}


		//parse samples
		if (nodeIn->GetTag().Contains(TEXT("sample")))
		{
			FKeyzoneSettings newKeyzone = FKeyzoneSettings();
			//UE_LOG(LogTemp, Log, TEXT("This is a sample"));
			for (auto& attribute : nodeIn->GetAttributes())
			{
				FString valCopy = attribute.GetValue();
				if (attribute.GetTag().Contains(TEXT("path")))
				{
					UE_LOG(LogTemp, Log, TEXT("Path %s"), *attribute.GetValue());
					newKeyzone.SamplePath = attribute.GetValue();
					continue;
				};

				if (attribute.GetTag().Contains(TEXT("loNote")))
				{
					//UE_LOG(LogTemp, Log, TEXT("Path %s"), *attribute.GetValue());
					newKeyzone.MinNote = FCString::Atoi(*attribute.GetValue());
					continue;
				};

				if (attribute.GetTag().Contains(TEXT("hiNote")))
				{
					newKeyzone.MaxNote = FCString::Atoi(*attribute.GetValue());
					continue;
				};

				if (attribute.GetTag().Contains(TEXT("rootNote")))
				{
					newKeyzone.RootNote = FCString::Atoi(*attribute.GetValue());
					continue;
				};

				if (attribute.GetTag().Contains(TEXT("loVel")))
				{
					newKeyzone.MinVelocity = FCString::Atoi(*attribute.GetValue());
					continue;
				};			
				
				
				
				if (attribute.GetTag().Contains(TEXT("hiVel")))
				{
					newKeyzone.MaxVelocity = FCString::Atoi(*attribute.GetValue());
					continue;
				};

				UE_LOG(LogTemp, Log, TEXT("Attbribute: %s, Value %s"), *attribute.GetTag(), *attribute.GetValue());
			}

			workingArray.Add(newKeyzone);

		}


		for (auto& node : nodeIn->GetChildrenNodes())
		{
			traverseXML(node, workingArray, workingSettings);
			if (node->GetNextNode()) traverseXML(node->GetNextNode(), workingArray, workingSettings);

		}
	}
}

void UBKDPresetToFusionImporter::configureFusionPatchFromDSPresetData(UFusionPatch* inPatch, TArray<FKeyzoneSettings>& workingArray, FFusionPatchSettings& workingSettings)
{

}
