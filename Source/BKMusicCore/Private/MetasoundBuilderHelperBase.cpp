// Fill out your copyright notice in the Description page of Project Settings.


#include "MetasoundBuilderHelperBase.h"
#include "Engine/Engine.h"
#include "IAudioParameterInterfaceRegistry.h"

#include "Interfaces/unDAWMetasoundInterfaces.h"


void UMetasoundBuilderHelperBase::InitBuilderHelper(FString BuilderName)
{
	UE_LOG(LogTemp,Log, TEXT("Test"))

	MSBuilderSystem = GEngine->GetEngineSubsystem<UMetaSoundBuilderSubsystem>();

	//FMetaSoundBuilderNodeOutputHandle OnPlayOutputNode;
	FMetaSoundBuilderNodeInputHandle OnFinished;
	TArray<FMetaSoundBuilderNodeInputHandle> AudioOuts;
	EMetaSoundBuilderResult BuildResult;

	//OutputFormat = SourceOutputFormat;
	//SessionData->MasterOptions.OutputFormat
	CurrentBuilder = MSBuilderSystem->CreateSourceBuilder(FName(BuilderName), OnPlayOutputNode, OnFinished, AudioOuts, BuildResult, SessionData->MasterOptions.OutputFormat, false);
	CurrentBuilder->AddInterface(FName(TEXT("unDAW Session Renderer")), BuildResult);
	CreateMidiPlayerBlock();

	PerformBpInitialization();

}

TArray<UMetaSoundSource*> UMetasoundBuilderHelperBase::GetAllMetasoundSourcesWithInstrumentInterface()
{
	auto AllObjectsArray = TArray<UMetaSoundSource*>();
	auto OnlyImplementingArray = TArray<UMetaSoundSource*>();
	

	GetObjectsOfClass<UMetaSoundSource>(AllObjectsArray);
	for (const auto& source : AllObjectsArray)
	{
		if (source->ImplementsParameterInterface(unDAW::Metasounds::FunDAWInstrumentRendererInterface::GetInterface())) OnlyImplementingArray.Add(source);
	}
	
	return OnlyImplementingArray;
}

TArray<UMetaSoundPatch*> UMetasoundBuilderHelperBase::GetAllMetasoundPatchesWithInstrumentInterface()
{
	auto AllObjectsArray = TArray<UMetaSoundPatch*>();
	auto OnlyImplementingArray = TArray<UMetaSoundPatch*>();
	GetObjectsOfClass<UMetaSoundPatch>(AllObjectsArray);
	auto interface = unDAW::Metasounds::FunDAWInstrumentRendererInterface::GetInterface();

	const FMetasoundFrontendVersion Version{ interface->GetName(), { interface->GetVersion().Major, interface->GetVersion().Minor } };

	for (const auto& patch : AllObjectsArray)
	{
		if (patch->GetDocumentChecked().Interfaces.Contains(Version)) OnlyImplementingArray.Add(patch);
	}
	return OnlyImplementingArray;
}

TArray<UMetaSoundPatch*> UMetasoundBuilderHelperBase::GetAllMetasoundPatchesWithInsertInterface()
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

void UMetasoundBuilderHelperBase::AuditionAC(UAudioComponent* AudioComponent)
{

}

bool SwitchOnBuildResult(EMetaSoundBuilderResult BuildResult)
{
	switch (BuildResult)
	{
	case EMetaSoundBuilderResult::Succeeded:
		return true;
		break;
	case EMetaSoundBuilderResult::Failed:
		return false;
		break;

		default:
			return false;
	}
}

void UMetasoundBuilderHelperBase::CreateInputsFromMidiTracks()
{
	// Create the inputs from the midi tracks
	for (const auto& [trackID, trackOptions] : MidiTracks)
	{
		// Create the input node
		//FMetaSoundBuilderNodeInputHandle InputNode = CurrentBuilder->CreateInputNode(FName(*trackOptions.trackName), trackOptions.trackColor, trackOptions.ChannelIndexInParentMidi)
	}
}

bool UMetasoundBuilderHelperBase::CreateMidiPlayerBlock()
{
		// Create the midi player block
	EMetaSoundBuilderResult BuildResult;
	FSoftObjectPath MidiPlayerAssetRef(TEXT("/Script/MetasoundEngine.MetaSoundPatch'/unDAW/BKSystems/MetaSoundBuilderHelperBP_V1/InternalPatches/BK_MetaClockManager.BK_MetaClockManager'"));
	TScriptInterface<IMetaSoundDocumentInterface> MidiPlayerDocument = MidiPlayerAssetRef.TryLoad();
	MidiPlayerNode = CurrentBuilder->AddNode(MidiPlayerDocument, BuildResult);

	if (SwitchOnBuildResult(BuildResult))
	{
		CurrentBuilder->ConnectNodeInputsToMatchingGraphInterfaceInputs(MidiPlayerNode, BuildResult);
	}

	if (SwitchOnBuildResult(BuildResult))
	{
		CurrentBuilder->ConnectNodeOutputsToMatchingGraphInterfaceOutputs(MidiPlayerNode, BuildResult);
	}

	FMetaSoundBuilderNodeInputHandle MidiFileInput = CurrentBuilder->FindNodeInputByName(MidiPlayerNode, FName(TEXT("MIDI File")), BuildResult);
	if (SwitchOnBuildResult(BuildResult))
	{
		auto midiaudioparameter = FAudioParameter(FName(TEXT("MIDI File")), SessionData->HarmonixMidiFile);
			//FMetasoundFrontendLiteral MidiFileLiteral = FMetasoundFrontendLiteral(static_cast<UObject*>(SessionData->HarmonixMidiFile));
		CurrentBuilder->SetNodeInputDefault(MidiFileInput, midiaudioparameter, BuildResult);
		UE_LOG(LogTemp, Log, TEXT("Midi File Input Set! %s"), SwitchOnBuildResult(BuildResult) ? TEXT("yay") : TEXT("nay"))
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Midi File Input Not Set! %s"), SwitchOnBuildResult(BuildResult) ? TEXT("yay") : TEXT("nay"))
	}

	return SwitchOnBuildResult(BuildResult);

	//FMetaSoundBuilderNodeInputHandle MidiPlayerNode = CurrentBuilder->CreateMidiPlayerNode(FName(TEXT("Midi Player")), FLinearColor::Green, 0);
}

void UMetasoundBuilderHelperBase::CreateAndAuditionPreviewAudioComponent()
{
	// Create the audio component
	AuditionComponentRef = NewObject<UAudioComponent>(this);
	//PreviewAudioComponent->RegisterComponent();
	//PreviewAudioComponent->SetSound(CurrentBuilder->GetSound());
	FOnCreateAuditionGeneratorHandleDelegate OnCreateAuditionGeneratorHandle;
	OnCreateAuditionGeneratorHandle.BindUFunction(this, TEXT("OnMetaSoundGeneratorHandleCreated"));
	CurrentBuilder->Audition(this, AuditionComponentRef, OnCreateAuditionGeneratorHandle, true);
	//PreviewAudioComponent->Play();
}

void UMetasoundBuilderHelperBase::OnMetaSoundGeneratorHandleCreated(UMetasoundGeneratorHandle* Handle)
{
	UE_LOG(LogTemp, Log, TEXT("Handle Created!"))
	GeneratorHandle = Handle;
}


