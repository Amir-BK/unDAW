// Fill out your copyright notice in the Description page of Project Settings.


#include "MetasoundBuilderHelperBase.h"
#include "Engine/Engine.h"
#include "IAudioParameterInterfaceRegistry.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"


void UMetasoundBuilderHelperBase::InitBuilderHelper(FString BuilderName, UAudioComponent* InAuditionComponent)
{
	MSBuilderSystem = GEngine->GetEngineSubsystem<UMetaSoundBuilderSubsystem>();
	AuditionComponentRef = InAuditionComponent;


	//ParentWorld = World;
	//FMetaSoundBuilderNodeOutputHandle OnPlayOutputNode;
	FMetaSoundBuilderNodeInputHandle OnFinished;

	EMetaSoundBuilderResult BuildResult;

	//OutputFormat = SourceOutputFormat;
	//SessionData->MasterOptions.OutputFormat
	CurrentBuilder = MSBuilderSystem->CreateSourceBuilder(FName(BuilderName), OnPlayOutputNode, OnFinished, AudioOuts, BuildResult, SessionData->MasterOptions.OutputFormat, false);
	CurrentBuilder->AddInterface(FName(TEXT("unDAW Session Renderer")), BuildResult);
	CreateMidiPlayerBlock();
	CreateInputsFromMidiTracks();
	CreateMixerNodesSpaghettiBlock();

#ifdef WITH_TESTS
	CreateTestWavPlayerBlock();
#endif //WITH_TESTS

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

#ifdef WITH_TESTS
void UMetasoundBuilderHelperBase::CreateTestWavPlayerBlock()
{
	EMetaSoundBuilderResult BuildResult;
	FSoftObjectPath WavPlayerAssetRef(TEXT("/Script/MetasoundEngine.MetaSoundPatch'/unDAW/BKSystems/MetaSoundBuilderHelperBP_V1/InternalPatches/BK_WavPlayer.BK_WavPlayer'"));
	TScriptInterface<IMetaSoundDocumentInterface> WavPlayerDocument = WavPlayerAssetRef.TryLoad();
	auto WavPlayerNode = CurrentBuilder->AddNodeByClassName(FMetasoundFrontendClassName(TEXT("UE"), TEXT("Wave Player"), TEXT("Stereo")), BuildResult);


	FMetaSoundBuilderNodeInputHandle WavFileInput = CurrentBuilder->FindNodeInputByName(WavPlayerNode, FName(TEXT("Wave Asset")), BuildResult);
	if (SwitchOnBuildResult(BuildResult))
	{
		FSoftObjectPath WavFileAssetRef(TEXT("/Game/onclassical_demo_demicheli_geminiani_pieces_allegro-in-f-major_small-version.onclassical_demo_demicheli_geminiani_pieces_allegro-in-f-major_small-version"));
		auto WavInput = CurrentBuilder->AddGraphInputNode(TEXT("Wave Asset"), TEXT("WaveAsset"), MSBuilderSystem->CreateObjectMetaSoundLiteral(WavFileAssetRef.TryLoad()), BuildResult);
		//CurrentBuilder->SetNodeInputDefault(WavFileInput, MSBuilderSystem->CreateObjectMetaSoundLiteral(WavFileAssetRef.TryLoad()), BuildResult);
		UE_LOG(LogTemp, Log, TEXT("Wav File Input Set! %s"), SwitchOnBuildResult(BuildResult) ? TEXT("yay") : TEXT("nay"))
		FString ResultOut = TEXT("Result: ");
		static const auto AppendToResult = [&ResultOut](const FString& Label, const EMetaSoundBuilderResult& BuildResult) { 
			ResultOut += Label + ":";
			switch (BuildResult)
			{
				case EMetaSoundBuilderResult::Succeeded:
				ResultOut += TEXT("Succeeded\n");
				break;
				case EMetaSoundBuilderResult::Failed:
					ResultOut += TEXT("Failed\n");
					break;


			default:
				ResultOut += TEXT("Unknown");
				break;
			}
				};
		if (SwitchOnBuildResult(BuildResult))
		{
			CurrentBuilder->ConnectNodes(WavInput, WavFileInput, BuildResult);
			AppendToResult(TEXT("Wave Input"), BuildResult);
			auto PlayPin = CurrentBuilder->FindNodeInputByName(WavPlayerNode, FName(TEXT("Play")), BuildResult);
			AppendToResult(TEXT("Find Play Pin"), BuildResult);
			auto AudioLeft = CurrentBuilder->FindNodeOutputByName(WavPlayerNode, FName(TEXT("Out Left")), BuildResult);
			AppendToResult(TEXT("Find Out Left"), BuildResult);
			auto AudioRight = CurrentBuilder->FindNodeOutputByName(WavPlayerNode, FName(TEXT("Out Right")), BuildResult);
			AppendToResult(TEXT("Find Out Right"), BuildResult);
			CurrentBuilder->ConnectNodes(OnPlayOutputNode, PlayPin, BuildResult);
			AppendToResult(TEXT("On Play"), BuildResult);

			auto WavePlayerAudioOuts = CurrentBuilder->FindNodeOutputsByDataType(WavPlayerNode, BuildResult, FName(TEXT("Audio")));


			for (int i = 0; i < AudioOuts.Num(); i++)
			{
				if (WavePlayerAudioOuts.IsValidIndex(i))
				{
					CurrentBuilder->ConnectNodes( WavePlayerAudioOuts[i], AudioOuts[i], BuildResult);
					AppendToResult(FString::Printf(TEXT("Connect Node Output %d"), i), BuildResult);
				}
				else
				{
					UE_LOG(LogTemp, Log, TEXT("Wave Player Audio Out Index %d Not Valid!"), i)
						break;
				}
			}

		}

		UE_LOG(LogTemp, Log, TEXT("Build Results: \n %s"), *ResultOut)

		//CurrentBuilder->SetNodeInputDefault(WavFileInput, MSBuilderSystem->CreateObjectMetaSoundLiteral(WavFileAssetRef.TryLoad()), BuildResult);

	}
	else {
		UE_LOG(LogTemp, Log, TEXT("Wav File Input Not Set! %s"), SwitchOnBuildResult(BuildResult) ? TEXT("yay") : TEXT("nay"))
	}	
}

#endif //WITH_TESTS

void UMetasoundBuilderHelperBase::CreateInputsFromMidiTracks()
{
	// Create the inputs from the midi tracks
	for (const auto& [trackID, trackOptions] : MidiTracks)
	{
		// Create the input node
		//FMetaSoundBuilderNodeInputHandle InputNode = CurrentBuilder->CreateInputNode(FName(*trackOptions.trackName), trackOptions.trackColor, trackOptions.ChannelIndexInParentMidi)
	}



}

void UMetasoundBuilderHelperBase::CreateMixerPatchBlock()
{

	using namespace Metasound::Frontend;
	UMetaSoundPatch* Patch = NewObject<UMetaSoundPatch>(this);
	check(Patch);

	auto Builder = FMetaSoundFrontendDocumentBuilder(Patch);
	Builder.InitDocument();

	// create the mixer patch
	EMetaSoundBuilderResult BuildResult;
	const auto PatchBuilder = MSBuilderSystem->CreatePatchBuilder(FName(TEXT("Mixer Patch")), BuildResult);
	//const auto PB2 = MSBuilderSystem->Trabsuebt
	PatchBuilder->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("AudioMixer")), FName(TEXT("Audio Mixer (Stereo, 8)")))
		, BuildResult);

	if (SwitchOnBuildResult(BuildResult))
	{
		//PatchBuilder->
		FMetaSoundBuilderOptions PatchOptions;
		//PatchOptions. OutputFormat = SessionData->MasterOptions.OutputFormat;
		PatchOptions.bAddToRegistry = false;
		//PatchOptions.Name = FName(TEXT("Mixer Patch"));
		//PatchOptions.bForceUniqueClassName = true;

		FSoftObjectPath MixerPatchAssetRef(TEXT("/Script/MetasoundEngine.MetaSoundPatch'/Game/MixerPatch.MixerPatch'"));
		//PatchBuilder->
		//TScriptInterface<IMetaSoundDocumentInterface> MixerPatch = MixerPatchAssetRef.TryLoad();

		//PatchOptions.ExistingMetaSound = MixerPatch;

		auto MixerPatch = PatchBuilder->Build(Patch, PatchOptions);

		CurrentBuilder->AddNode(MixerPatch, BuildResult);
	}
}

	void UMetasoundBuilderHelperBase::CreateMixerNodesSpaghettiBlock()
	{

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
		//auto midiaudioparameter = FAudioParameter(FName(TEXT("MIDI File")), SessionData->HarmonixMidiFile);
		FMetasoundFrontendLiteral MidiFileLiteral;
		MidiFileLiteral.Set(SessionData->HarmonixMidiFile);
		CurrentBuilder->SetNodeInputDefault(MidiFileInput, MSBuilderSystem->CreateObjectMetaSoundLiteral(SessionData->HarmonixMidiFile), BuildResult);
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
	FSoftObjectPath soundPath(TEXT("/Script/Engine.SoundWave'/Game/onclassical_demo_demicheli_geminiani_pieces_allegro-in-f-major_small-version.onclassical_demo_demicheli_geminiani_pieces_allegro-in-f-major_small-version'"));
	USoundWave* SoundWave = Cast<USoundWave>(soundPath.TryLoad());

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


