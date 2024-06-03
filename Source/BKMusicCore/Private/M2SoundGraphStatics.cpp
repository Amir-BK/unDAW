// Fill out your copyright notice in the Description page of Project Settings.


#include "M2SoundGraphStatics.h"
#include "SequencerData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"

void UM2SoundGraphStatics::CreateDefaultVertexesFromInputVertex(UDAWSequencerData* SequencerData, UM2SoundTrackInput* InputVertex, const int Index)
{
	UE_LOG(LogTemp, Warning, TEXT("CreateDefaultVertexesFromInputVertex"));

	auto DefaultPatchTest = FSoftObjectPath(TEXT("'/unDAW/Patches/System/unDAW_Fusion_Instrument.unDAW_Fusion_Instrument'"));

	UM2SoundMidiOutput* NewOutput = NewObject<UM2SoundMidiOutput>(SequencerData->GetOuter(), NAME_None, RF_Transactional);

	UM2SoundPatch* NewPatch = NewObject<UM2SoundPatch>(SequencerData->GetOuter(), NAME_None, RF_Transactional);
	NewPatch->Patch = CastChecked<UMetaSoundPatch>(DefaultPatchTest.TryLoad());

	InputVertex->TrackId = Index;
	InputVertex->Outputs.Add(NewPatch);
	NewPatch->Outputs.Add(NewOutput);

	NewOutput->OutputName = FName(SequencerData->GetTracksDisplayOptions(Index).trackName);

	SequencerData->Outputs.Add(FName(*FString::Printf(TEXT("Track %d"), Index)), NewOutput);
	SequencerData->Patches.Add(FName(*FString::Printf(TEXT("Track %d"), Index)), NewPatch);
}

TArray<UM2SoundVertex*> UM2SoundGraphStatics::GetAllVertexesInSequencerData(UDAWSequencerData* SequencerData)
{
	TArray<UM2SoundVertex*> Vertexes;

	for (auto& Output : SequencerData->Outputs)
	{
		Vertexes.Add(Output.Value);
	}

	for (auto& Patch : SequencerData->Patches)
	{
		Vertexes.Add(Patch.Value);
	}

	for (auto& Input : SequencerData->TrackInputs)
	{
		Vertexes.Add(Input.Value);
	}

	return Vertexes;

}

TArray<UMetaSoundPatch*> UM2SoundGraphStatics::GetAllPatchesImplementingInstrumetInterface()
{
	TArray<UMetaSoundPatch*> Patches;

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	auto interface = unDAW::Metasounds::FunDAWInstrumentRendererInterface::GetInterface();

	const FMetasoundFrontendVersion Version{ interface->GetName(), { interface->GetVersion().Major, interface->GetVersion().Minor } };
	TArray<FAssetData> AssetData;
	AssetRegistryModule.Get().GetAssetsByClass(UMetaSoundPatch::StaticClass()->GetClassPathName(), AssetData);

	for (int i = 0; i < AssetData.Num(); i++)
	{
		auto Patch = Cast<UMetaSoundPatch>(AssetData[i].GetAsset());
		if (Patch->GetDocumentChecked().Interfaces.Contains(Version))
		{
			Patches.Add(Patch);
		}
	}

	return Patches;

}

TArray<UMetaSoundPatch*> UM2SoundGraphStatics::GetAllMetasoundPatchesWithInsertInterface()
{
	auto AllObjectsArray = TArray<UMetaSoundPatch*>();
	auto OnlyImplementingArray = TArray<UMetaSoundPatch*>();
	GetObjectsOfClass<UMetaSoundPatch>(AllObjectsArray);
	auto interface = unDAW::Metasounds::FunDAWCustomInsertInterface::GetInterface();

	const FMetasoundFrontendVersion Version{ interface->GetName(), { interface->GetVersion().Major, interface->GetVersion().Minor } };

	for (const auto& patch : AllObjectsArray)
	{
		if (patch->GetDocumentChecked().Interfaces.Contains(Version)) OnlyImplementingArray.Add(patch);
	}
	return OnlyImplementingArray;
}
