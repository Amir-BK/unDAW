// Fill out your copyright notice in the Description page of Project Settings.

#include "M2SoundGraphStatics.h"
#include "M2SoundGraphData.h"
#include "Vertexes/M2SoundVertex.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"

FName UM2SoundGraphStatics::GetParentPresetNameIfAny(UMetaSoundPatch* Patch)
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;

	//AssetRegistryModule.Get().

	return FName(TEXT("TEST"));
}

UDAWSequencerData* UM2SoundGraphStatics::CreateEmptySequencerData()
{
	auto NewSequence = NewObject<UDAWSequencerData>();

	//NewSequence->M2SoundGraph = NewObject<UM2SoundGraph>(NewSequence, FName(), RF_Transactional);
	//NewSequence->M2SoundGraph->Schema = UM2SoundEdGraphSchema::StaticClass();

	auto NewMidiFile = NewObject<UMidiFile>(NewSequence, FName(), RF_Transactional);

	//init file, might be a bit wasteful to do it here but it's fine for now
	//NewMidiFile->GetSongMaps()->GetBarMap().SetTicksPerQuarterNote(480);

	//add a note on and off events to track 0, pitch 60, velocity 100, at time 0
	auto NoteOnMidiMessage = FMidiMsg::CreateNoteOn(1, 60, 100);
	auto NoteOffMidiMessage = FMidiMsg::CreateNoteOff(1, 60);

	auto NoteOnEvent = FMidiEvent(0, NoteOnMidiMessage);
	auto NoteOffEvent = FMidiEvent(480, NoteOffMidiMessage);

	//another note on and off event for testing
	auto NoteOnMidiMessage2 = FMidiMsg::CreateNoteOn(0, 62, 100);
	auto NoteOffMidiMessage2 = FMidiMsg::CreateNoteOff(0, 62);

	auto NoteOnEvent2 = FMidiEvent(0, NoteOnMidiMessage2);
	auto NoteOffEvent2 = FMidiEvent(480, NoteOffMidiMessage2);

	NewMidiFile->BuildConductorTrack();
	NewMidiFile->GetSongMaps()->GetTempoMap().AddTempoInfoPoint(Harmonix::Midi::Constants::BPMToMidiTempo(140), 0);
	NewMidiFile->GetSongMaps()->GetBarMap().AddTimeSignatureAtBarIncludingCountIn(0, 4, 4, true);

	//auto NewTrack = NewMidiFile->AddTrack(FString::Printf(TEXT("New Track %d"), 0));
	//NewTrack->AddEvent(NoteOnEvent);
	//NewTrack->AddEvent(NoteOffEvent);

	//NewTrack->AddEvent(NoteOnEvent2);
	//NewTrack->AddEvent(NoteOffEvent2);

	//ConductorTrack->AddEvent(DefaultTempoEvent);
	//ConductorTrack->AddEvent(DefaultTimeSigEvent);

	NewMidiFile->SortAllTracks();

	NewSequence->HarmonixMidiFile = NewMidiFile;

	return NewSequence;
}

void UM2SoundGraphStatics::CreateDefaultVertexesFromInputData(UDAWSequencerData* InSequencerData, const int Index)
{
	UE_LOG(LogTemp, Warning, TEXT("CreateDefaultVertexesFromInputVertex"));

	auto DefaultPatchTest = FSoftObjectPath(TEXT("'/unDAW/Patches/System/unDAW_Fusion_Piano.unDAW_Fusion_Piano'"));
	auto& TrackMetadata = InSequencerData->GetTracksDisplayOptions(Index);

	UM2SoundMidiInputVertex* InputVertex = NewObject<UM2SoundMidiInputVertex>(InSequencerData, NAME_None, RF_Transactional);
	InputVertex->SequencerData = InSequencerData;
	InputVertex->TrackPrefix = FString::Printf(TEXT("Tr%d_Ch%d."), TrackMetadata.TrackIndexInParentMidi, TrackMetadata.ChannelIndexInParentMidi);

	InputVertex->TrackId = Index;
	//InputVertex->

	//UM2SoundMidiOutput* NewOutput = NewObject<UM2SoundMidiOutput>(SequencerData->GetOuter(), NAME_None, RF_Transactional);

	UM2SoundAudioOutput* NewAudioOutput = NewObject<UM2SoundAudioOutput>(InSequencerData, NAME_None, RF_Transactional);
	NewAudioOutput->SequencerData = InSequencerData;

	UM2SoundPatch* NewPatch = NewObject<UM2SoundPatch>(InSequencerData, NAME_None, RF_Transactional);
	NewPatch->Patch = CastChecked<UMetaSoundPatch>(DefaultPatchTest.TryLoad());
	NewPatch->SequencerData = InSequencerData;

	//NewPatch->MakeTrackInputConnection(InputVertex);
	//NewAudioOutput->MakeTrackInputConnection(NewPatch);

	//InputVertex->TrackId = Index;
	//InputVertex->Outputs.Add(NewPatch);
	//NewPatch->Outputs.Add(NewOutput);

	//NewOutput->OutputName = FName(SequencerData->GetTracksDisplayOptions(Index).trackName);

	//NewPatch->Outputs.Add(NewAudioOutput);
	//NewPatch->Inputs.Add(InputVertex);
	//NewPatch->MainInput = InputVertex;

	//NewAudioOutput->Inputs.Add(NewPatch);
	//NewAudioOutput->MainInput = NewPatch;
	//InSequencerData->AddVertex(InputVertex);
	//InSequencerData->AddVertex(NewPatch);
	//InSequencerData->AddVertex(NewAudioOutput);
}

TArray<UM2SoundVertex*> UM2SoundGraphStatics::GetAllVertexesInSequencerData(UDAWSequencerData* SequencerData)
{
	TArray<UM2SoundVertex*> Vertexes;

	return Vertexes;
}

UMetaSoundPatch* UM2SoundGraphStatics::GetPatchByName(FString Name)
{
	//get objects of class, compare string, return object
	TArray<UMetaSoundPatch*> Patches;
	GetObjectsOfClass<UMetaSoundPatch>(Patches);

	for (auto Patch : Patches)
	{
		if (Patch->GetName() == Name)
		{
			return Patch;
		}
	}

	return nullptr;
}

void UM2SoundGraphStatics::PopulateAssignableOutputsArray(TArray<FAssignableAudioOutput>& OutAssignableOutputs, const TArray<FMetaSoundBuilderNodeInputHandle> InMixerNodeInputs)
{
	for (SIZE_T i = 0; i < InMixerNodeInputs.Num(); i += 3)
	{
		//the outputs are always sorted "In 0 L", "In 0 R", "Gain 0"
		FAssignableAudioOutput AssignableOutput;
		AssignableOutput.AudioLeftOutputInputHandle = InMixerNodeInputs[i];
		AssignableOutput.AudioRightOutputInputHandle = InMixerNodeInputs[i + 1];
		AssignableOutput.GainParameterInputHandle = InMixerNodeInputs[i + 2];

		AssignableOutput.OutputName = FName(FGuid::NewGuid().ToString());

		OutAssignableOutputs.Add(AssignableOutput);
	}
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

bool UM2SoundGraphStatics::DoesPatchImplementInterface(UMetaSoundPatch* Patch, UClass* InterfaceClass)
{
	//auto interface = unDAW::Metasounds::FunDAWCustomInsertInterface::GetInterface();
	//const FMetasoundFrontendVersion Version{ interface->GetName(), { interface->GetVersion().Major, interface->GetVersion().Minor } };
	//return Patch->GetDocumentChecked().Interfaces.Contains(Version);

	return false;
}