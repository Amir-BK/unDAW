// Fill out your copyright notice in the Description page of Project Settings.


#include "BKDSLibraryFactory.h"
#include "BKDPresetToFusionImporter.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Logging/LogMacros.h"
#include "Misc/CommandLine.h"
#include "Misc/PathViews.h"
#include <XmlNode.h>
#include <AssetToolsModule.h>
#include "Sound/SoundWave.h"
#include <Factories/SoundFactory.h>
#include "Editor/EditorEngine.h"
#include "Editor.h"
#include "Misc/FileHelper.h"
#include "Widgets/Text/STextBlock.h"
#include "Subsystems/ImportSubsystem.h"
#include <BKZippedAudioReader/ZipSoundFactory.h>

static const TCHAR* ZipFileExtension = TEXT(".dslibrary");

UObject* UBKDSLibraryFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	TArray<FString> ZipsToMergeIn;
	FString ExportPath;
	//TArray<UFusionPatch*> newPatches;
	auto SoundWaveFactory = NewObject<UZipSoundFactory>();
	

	FXmlFile MyXMLfile = FXmlFile();

	FZipArchiveReader Reader(PlatformFile.OpenRead(*Filename));
	bool bAllValid = false;
	if (Reader.IsValid())
	{
		bAllValid = true;
		for (const FString& EmbeddedFileName : Reader.GetFileNames())
		{
			UE_LOG(LogTemp, Log, TEXT("Embedded File Name: %s"), *EmbeddedFileName)
			TArray<uint8> Contents;
			if (!Reader.TryReadFile(EmbeddedFileName, Contents))
			{
				bAllValid = false;
				continue;
			}
			bool dumbHack = false;

			if (EmbeddedFileName.Contains(TEXT("wav")))
			{
				const uint8* bufferStart = Contents.GetData();
				const uint8* bufferEnd = &Contents[Contents.Last()];
				//Contents.view

				FString PathString = FPaths::GetPath(*Filename);
				FString Extension = FPaths::GetExtension(PathString);
				SoundWaveFactory->FactoryCreateBinary(USoundWave::StaticClass(), this, FName(EmbeddedFileName),
					Flags, InParent, *Extension, bufferStart, bufferEnd, Warn);
				//FactoryCreateBinary()
				//FFileHelper::
				//newSample->
			}

			if (EmbeddedFileName.Contains(TEXT("dspreset")))
			{
				FString PathString = FPaths::GetPath(*Filename);
				FString Extension = FPaths::GetExtension(PathString);
				GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(
					this, UFusionPatch::StaticClass(), this, FName(*EmbeddedFileName), *Extension);
				//GEditor->GetEditorSubsystem<UImportSubsystem>()->Import
				//UE_LOG(LogTemp,Log, TEXT("No Spaghetti"))
				TArray <FKeyzoneSettings> KeyZoneArray;
				FFusionPatchSettings newSettings;
				
				UFusionPatch* NewFusionPatch = NewObject<UFusionPatch>(InParent, InClass, dumbHack ? *EmbeddedFileName : InName, Flags);
				newPatches.Add(NewFusionPatch);
				TArray<FString> fileToImport;
				fileToImport.Add(EmbeddedFileName);
				//ZipWriter->AddFile(EmbeddedFileName, Contents, FDateTime::Now());
				UAutomatedAssetImportData* newSample = NewObject<UAutomatedAssetImportData>();
				FString contentBuffer;
				FFileHelper::BufferToString(contentBuffer, Contents.GetData(), Contents.Num());
				//UE_LOG(LogTemp, Log, TEXT("Loaded buffer %s"), *contentBuffer)
				//newSample->
				MyXMLfile.LoadFile(contentBuffer, EConstructMethod::ConstructFromBuffer);
				//UE_LOG(LogTemp, Log, TEXT("No Spaghetti %d"), MyXMLfile.IsValid())
				UBKDPresetToFusionImporter::traverseXML(MyXMLfile.GetRootNode(), KeyZoneArray, newSettings);
				NewFusionPatch->UpdateKeyzones(KeyZoneArray);
				NewFusionPatch->UpdateSettings(newSettings);

				GEditor->GetEditorSubsystem<UImportSubsystem>()->
					BroadcastAssetPostImport(this, NewFusionPatch);

				dumbHack = true;
	
			}


		};


		
	};


	
	AdditionalImportedObjects.Append(newPatches);

	if (!newPatches.IsEmpty()) return newPatches[0];

	return nullptr;
};


UBKDSLibraryFactory::UBKDSLibraryFactory()
{
	//Formats.Add(FString(TEXT("sfz;")) + NSLOCTEXT("USFZAssetFactory", "FormatSfz", "SFZ File").ToString());
	
	//DISABLED UNTIL I SORT ZIP UNARCHIVING THE SAMPLES 
	// 
	//Formats.Add(TEXT("dslibrary;Decent Sampler Library Archive"));
	SupportedClass = UFusionPatch::StaticClass();
	bCreateNew = false;
	bEditorImport = true;
	//Octave_Offset = 12;
}
