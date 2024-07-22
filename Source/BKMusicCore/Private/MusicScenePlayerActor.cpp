// Fill out your copyright notice in the Description page of Project Settings.

#include "MusicScenePlayerActor.h"
#include "Kismet/GameplayStatics.h"
#include "Metasound.h"
#include "Materials/MaterialParameterCollection.h"
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
	GetDAWSequencerData()->AuditionBuilder(AudioComponent);
	GetDAWSequencerData()->OnPlaybackStateChanged.AddDynamic(this, &AMusicScenePlayerActor::DAWSequencePlayStateChange);
	//AuditionComponent = AudioComponent

	if (bAutoPlay) GetDAWSequencerData()->SendTransportCommand(EBKTransportCommands::Play);
}

void AMusicScenePlayerActor::PerformanceMetasoundGeneratorCreated()
{
	// we can't play here cause it's not from the game thread? I think?
	UE_LOG(LogTemp, Warning, TEXT("Hello ?"))
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