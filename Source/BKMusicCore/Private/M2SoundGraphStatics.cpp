// Fill out your copyright notice in the Description page of Project Settings.


#include "M2SoundGraphStatics.h"
#include "SequencerData.h"

void UM2SoundGraphStatics::CreateDefaultVertexesFromInputVertex(UDAWSequencerData* SequencerData, UM2SoundTrackInput* InputVertex, const int Index)
{
	UE_LOG(LogTemp, Warning, TEXT("CreateDefaultVertexesFromInputVertex"));

	UM2SoundMidiOutput* NewOutput = NewObject<UM2SoundMidiOutput>(SequencerData->GetOuter(), NAME_None, RF_Transactional);

	UM2SoundPatch* NewPatch = NewObject<UM2SoundPatch>(SequencerData->GetOuter(), NAME_None, RF_Transactional);

	InputVertex->TrackId = Index;
	InputVertex->Outputs.Add(NewPatch);
	NewPatch->Outputs.Add(NewOutput);

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
