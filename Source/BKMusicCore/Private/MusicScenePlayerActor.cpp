// Fill out your copyright notice in the Description page of Project Settings.

#include "MusicScenePlayerActor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundSourceBus.h"
#include "Sound/AudioBus.h"
#include "Metasound.h"
#include "HarmonixMidi/MusicTimeSpan.h"
#include "Materials/MaterialParameterCollection.h"
#include "EditableMidiFile.h"
#include "MetasoundGeneratorHandle.h"

bool AMusicScenePlayerActor::AttachM2VertexToMixerInput(FName MixerAlias, UMetaSoundPatch* Patch, float InVolume, const FOnTriggerExecuted& InDelegate)
{
	//check if the patch implements the unDAW interface

	//FMetasoundFrontendVersion ActionInterface;
	//ActionInterface.Name = FName("Audible Action");
	//ActionInterface.Number = { 0, 1 };

	//bool bSuccess = false;

	//if (Patch->GetConstDocument().Interfaces.Contains(ActionInterface))
	//{
	//	bSuccess = true;
	//}

	////check if mixer alias is valid
	//if (MixerAlias.IsNone())
	//{
	//	UE_LOG(LogTemp, Error, TEXT("Mixer Alias is None"))
	//		return false;
	//}
	//else {
	//	bSuccess &= GetDAWSequencerData()->Mixers.Contains(MixerAlias);
	//}
	//

	return GetDAWSequencerData()->AttachActionPatchToMixer(MixerAlias, Patch, InVolume, InDelegate);
}

UM2ActionVertex* AMusicScenePlayerActor::AttachPatchToActor(UMetaSoundPatch* Patch, AActor* Actor, FName SocketName, float InVolume, class USoundAttenuation* AttenuationSettings)
{
	return nullptr;
}

UM2ActionVertex* AMusicScenePlayerActor::SpawnPatchAttached(FMusicTimestamp InTimestamp, UMetaSoundPatch* Patch, UMidiFile* MidiClip, EMidiClockSubdivisionQuantization InQuantizationUnits, USceneComponent* AttachToComponent, FName AttachPointName, FVector Location, FRotator Rotation, EAttachLocation::Type LocationType, bool bStopWhenAttachedToDestroyed, float VolumeMultiplier, float PitchMultiplier, float StartTime, USoundAttenuation* AttenuationSettings, USoundConcurrency* ConcurrencySettings, bool bAutoDestroy)
{
	//so the whole point of this thing is that we need an audio bus and a source bus, the source bus will be attached to the actual actor,
	//the audio bus needs to be assigned to an audio bus output vertex
    
	auto NewSourceBus = NewObject<USoundSourceBus>(this, NAME_None, RF_Transient);
	auto NewAudioBus = NewObject<UAudioBus>(this, NAME_None, RF_Transient);

	NewAudioBus->AudioBusChannels = EAudioBusChannels::Stereo;
	NewSourceBus->NumChannels = 2;

	//REMEMBER TO CHANGE THIS TO THE ACTUAL AUDIO BUS
	NewSourceBus->AudioBus = NewAudioBus;

	auto NewActionVertex = NewObject<UM2ActionVertex>(this, NAME_None, RF_Transient);
	NewActionVertex->SequencerData = GetDAWSequencerData();
	NewActionVertex->Patch = Patch;

	NewActionVertex->SourceBus = NewSourceBus;
	NewActionVertex->AudioBus = NewAudioBus;

	GetDAWSequencerData()->AddTransientVertex(NewActionVertex);

	NewSourceBus->bAutoDeactivateWhenSilent = false;

	
	auto BusTransmitterSoftClassPath = FSoftObjectPath(TEXT("/unDAW/Patches/System/MSP_BusTransmitter.MSP_BusTransmitter"));

	auto BusTransmitterClass = Cast<UMetaSoundPatch>(BusTransmitterSoftClassPath.TryLoad());

	EMetaSoundBuilderResult Result;

	const auto& BuilderContext = GetDAWSequencerData()->BuilderContext;

	auto NewTransientBusOut = BuilderContext->AddNode(BusTransmitterClass, Result);
	NewActionVertex->AdditionalNodes.Add(NewTransientBusOut);

	auto BusInput = BuilderContext->FindNodeInputByName(NewTransientBusOut, FName("Audio Bus"), Result);

	auto BusInputObjectLiteral = GetDAWSequencerData()->MSBuilderSystem->CreateObjectMetaSoundLiteral(NewAudioBus);

	BuilderContext->SetNodeInputDefault(BusInput, BusInputObjectLiteral, Result);
	//now to connect the audio outputs
	using namespace M2Sound::Pins;
	auto AsAudioTrackOutPin = Cast<UM2AudioTrackPin>(NewActionVertex->OutputM2SoundPins[AutoDiscovery::AudioTrack]);

	auto BusInputAudioLeft = BuilderContext->FindNodeInputByName(NewTransientBusOut, FName("unDAW Insert.Audio In L"), Result);
	auto BusInputAudioRight = BuilderContext->FindNodeInputByName(NewTransientBusOut, FName("unDAW Insert.Audio In R"), Result);

	BuilderContext->ConnectNodes(AsAudioTrackOutPin->AudioStreamL->GetHandle<FMetaSoundBuilderNodeOutputHandle>(), BusInputAudioLeft, Result);
	BuilderContext->ConnectNodes(AsAudioTrackOutPin->AudioStreamR->GetHandle<FMetaSoundBuilderNodeOutputHandle>(), BusInputAudioRight, Result);

	//finally create the audio component which should start streaming...
	GetDAWSequencerData()->ConnectTransientVertexToMidiClock(NewActionVertex);
	//if midi clip is not a nullptr, we assume it was already shifted and cropped, it can be assigned to the interface midi asset input
	if (MidiClip != nullptr)
	{
		//NewActionVertex->InputM2SoundPins[FName("unDAW.Midi Asset")]->GetHandle<FMetaSoundBuilderNodeInputHandle>();		//NewActionVertex->MidiAsset = MidiClip;
		//auto MidiAssetInput = BuilderContext->FindNodeInputByName(NewTransientBusOut, FName("unDAW.Midi Asset"), Result);
		//auto MidiAssetObjectLiteral = GetDAWSequencerData()->MSBuilderSystem->CreateObjectMetaSoundLiteral(ShiftAndCropMidiAsset(MidiClip, FMusicalTimeSpan(), InTimestamp, InQuantizationUnits));
		auto MidiAssetObjectLiteral = GetDAWSequencerData()->MSBuilderSystem->CreateObjectMetaSoundLiteral(MidiClip);
		BuilderContext->SetNodeInputDefault(NewActionVertex->InputM2SoundPins[FName("unDAW.Midi Asset")]->GetHandle<FMetaSoundBuilderNodeInputHandle>() , MidiAssetObjectLiteral, Result);
	}

	auto TimestampBarIntInput = NewActionVertex->InputM2SoundPins[FName("unDAW.Action.TimeStampBar")]->GetHandle<FMetaSoundBuilderNodeInputHandle>();
	auto TimestampBeatIntInput = NewActionVertex->InputM2SoundPins[FName("unDAW.Action.TimeStampBeat")]->GetHandle<FMetaSoundBuilderNodeInputHandle>();

	FName DataTypeName;

	auto TimestampBarIntObjectLiteral = GetDAWSequencerData()->MSBuilderSystem->CreateIntMetaSoundLiteral(InTimestamp.Bar, DataTypeName);
	auto TimestampBeatIntObjectLiteral = GetDAWSequencerData()->MSBuilderSystem->CreateIntMetaSoundLiteral(InTimestamp.Beat, DataTypeName);

	BuilderContext->SetNodeInputDefault(TimestampBarIntInput, TimestampBarIntObjectLiteral, Result);
	BuilderContext->SetNodeInputDefault(TimestampBeatIntInput, TimestampBeatIntObjectLiteral, Result);

	NewActionVertex->ExecuteTriggerOnPatch(FName("unDAW Action.OnAdded"));

	NewActionVertex->AudioComponent = UGameplayStatics::SpawnSoundAttached(NewSourceBus, AttachToComponent, AttachPointName, Location, Rotation, LocationType, bStopWhenAttachedToDestroyed, VolumeMultiplier, PitchMultiplier, StartTime, AttenuationSettings, ConcurrencySettings, bAutoDestroy);

	




	return NewActionVertex;
}

UEditableMidiFile* AMusicScenePlayerActor::ShiftAndCropMidiAsset(UMidiFile* InMidiClip, FMusicalTimeSpan InTimeSpan, FMusicTimestamp InTimestamp, EMidiClockSubdivisionQuantization InQuantizationUnits)
{
	//so we want to take the midi data and for now
	//EMidiClockSubdivisionQuantization OffsetUnits = EMidiClockSubdivisionQuantization::Bar;

	//calculate the tick
	auto OffsetTicks = InMidiClip->GetSongMaps()->CalculateMidiTick(InTimestamp, InQuantizationUnits);
	
	//create new editable copy of the midi clip


	UEditableMidiFile* EditableMidiClip = NewObject<UEditableMidiFile>(this, NAME_None, RF_Transient);
	EditableMidiClip->LoadFromHarmonixMidiFileAndApplyModifiers(InMidiClip, nullptr, OffsetTicks);
	
	MidiClips.Add(EditableMidiClip);
	//auto UniqueID = FGuid::NewGuid().ToString() + TEXT(".mid");
	//EditableMidiClip->SaveStdMidiFile(FPaths::ProjectContentDir() / UniqueID);
	//EditableMidiClip->SaveStdMidiFile(FPaths::ProjectContentDir() + InMidiClip->GetName() + FString::FromInt(rand()) +  TEXT(".mid"));

	return EditableMidiClip;
}

// Sets default values
AMusicScenePlayerActor::AMusicScenePlayerActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("Scene Audio Component"));

	//RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Attachment Root"));

	//Audio->AutoAttachParent = RootComponent;
}

void AMusicScenePlayerActor::DAWSequencePlayStateChange(EBKPlayState NewState)
{
	UE_LOG(LogTemp, Log, TEXT("DAW Sequence Play State Changed to %s"), *UEnum::GetValueAsString(NewState));

	if (!bHarmonixInitialized) return;

	switch (NewState)
	{
	case EBKPlayState::TransportPlaying:
		VideoSyncedMidiClock->Start();
		AudioSyncedMidiClock->Start();
		break;

	case EBKPlayState::ReadyToPlay:
		VideoSyncedMidiClock->Stop();
		AudioSyncedMidiClock->Start();
		break;

	case EBKPlayState::TransportPaused:
		VideoSyncedMidiClock->Pause();
		AudioSyncedMidiClock->Start();
		break;
	}

	PlayState = NewState;
}

//hmmm

inline UDAWSequencerData* AMusicScenePlayerActor::GetDAWSequencerData() const {
	return SessionData;
}

// Called when the game starts or when spawned
void AMusicScenePlayerActor::BeginPlay()
{
	Super::BeginPlay();
	bHarmonixInitialized = false;

	if (!GetDAWSequencerData())
	{
		UE_LOG(LogTemp, Error, TEXT("No DAW Sequencer Data set on Music Scene Player Actor"))
			return;
	}

	CurrentTimestamp = CurrentTimestamp.CreateLambda([this]() { return GetDAWSequencerData()->CurrentTimestampData; });

	//we need to do this because 'create sound 2d' is not reliable without a wav file, this one actually play but this adds a point of weakness to the system
	//in the form of the wav file.
	auto PrimingSound = FSoftObjectPath(TEXT("/unDAW/BKSystems/Core/PrimingAudioDontMove/1kSineTonePing.1kSineTonePing")).TryLoad();
	auto AsWavAsset = Cast<USoundWave>(PrimingSound);

	auto AudioComponent = UGameplayStatics::CreateSound2D(this, AsWavAsset, 1.0f, 1.0f, 0.0f, nullptr, true, false);
	GetDAWSequencerData()->OnBuilderReady.AddDynamic(this, &AMusicScenePlayerActor::PerformanceMetasoundGeneratorCreated);
	GetDAWSequencerData()->AuditionBuilder(AudioComponent, true, MasterAudioBus);
	GetDAWSequencerData()->OnPlaybackStateChanged.AddDynamic(this, &AMusicScenePlayerActor::DAWSequencePlayStateChange);
	//AuditionComponent = AudioComponent

	if (bAutoPlay) GetDAWSequencerData()->SendTransportCommand(EBKTransportCommands::Play);
}

void AMusicScenePlayerActor::PerformanceMetasoundGeneratorCreated()
{
	// we can't play here cause it's not from the game thread? I think?

		InitHarmonixComponents();

	GetDAWSequencerData()->OnBuilderReady.RemoveDynamic(this, &AMusicScenePlayerActor::PerformanceMetasoundGeneratorCreated);
}

void AMusicScenePlayerActor::PerformanceMetasoundGeneratorDestroyed(uint64 GeneratorPointer)
{
	UE_LOG(LogTemp, Log, TEXT("Good bye ?"))
}

// Called every frame
void AMusicScenePlayerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TODO [$65cfdef41013620009101dd9]: implement time keeping and cursor updates vs. the game thread
	// DON'T DO THAT
}

void delegateFunc(FName Output, const FMetaSoundOutput& MetaSoundOutput)
{
	UE_LOG(LogTemp, Log, TEXT("What"))
}

void AMusicScenePlayerActor::InitHarmonixComponents()
{
	auto& AudioComponent = GetDAWSequencerData()->AuditionComponent;

	if (!MaterialParameterCollection)
	{
		UE_LOG(LogTemp, Error, TEXT("No Material Parameter Collection set on Music Scene Player Actor"))
			//return;
	}
	else {
		FCollectionScalarParameter* DurationParam;
		DurationParam = MaterialParameterCollection->ScalarParameters.FindByPredicate([](const FCollectionScalarParameter& Info) {
			return Info.ParameterName == FName("SongDuration");
			});

		if (DurationParam)
		{
			DurationParam->DefaultValue = GetDAWSequencerData()->SequenceDuration;
		}
		//int32 DurationParam =	MaterialParameterCollection->ScalarParameters.IndexOfByPredicate([](const FMaterialParameterInfo& Info) {
		//	return Info.Name == FName("SongDuration");
		//});

		//MaterialParameterCollection->ScalarParameters[DurationParam].DefaultValue = GetDAWSequencerData()->SequenceDuration;
	}

	VideoSyncedMidiClock = NewObject<UMusicClockComponent>(this);
	VideoSyncedMidiClock->MetasoundOutputName = TEXT("unDAW.Midi Clock");
	VideoSyncedMidiClock->RegisterComponent();
	bool bConnectSuccess = VideoSyncedMidiClock->ConnectToMetasoundOnAudioComponent(AudioComponent);

	if (!bConnectSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to connect Video Synced Midi Clock to Audio Component"))
	}

	//VideoSyncedMidiClock->PlayStateEvent.AddUniqueDynamic(this, &AMusicScenePlayerActor::OnMusicClockPlaystate);
	//VideoSyncedMidiClock->Start();
	AudioSyncedMidiClock = NewObject<UMusicClockComponent>(this);
	AudioSyncedMidiClock->MetasoundOutputName = TEXT("unDAW.Midi Clock");
	AudioSyncedMidiClock->RegisterComponent();
	AudioSyncedMidiClock->ConnectToMetasoundOnAudioComponent(AudioComponent);
	VideoSyncedMidiClock->TimebaseForBarAndBeatEvents = ECalibratedMusicTimebase::ExperiencedTime;
	AudioSyncedMidiClock->TimebaseForBarAndBeatEvents = ECalibratedMusicTimebase::ExperiencedTime;

	MusicTempometer = NewObject<UMusicTempometerComponent>(this);
	MusicTempometer->RegisterComponent();
	MusicTempometer->SetMaterialParameterCollection(MaterialParameterCollection);
	MusicTempometer->SetClock(VideoSyncedMidiClock);

	bHarmonixInitialized = true;
}

void AMusicScenePlayerActor::UpdateWatchers()
{
	GeneratorHandle->UpdateWatchers();
}