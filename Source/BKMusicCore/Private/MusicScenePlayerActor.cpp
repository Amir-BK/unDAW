// Fill out your copyright notice in the Description page of Project Settings.


#include "MusicScenePlayerActor.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollection.h"
#include "MetasoundGeneratorHandle.h"

// Sets default values
AMusicScenePlayerActor::AMusicScenePlayerActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("Scene Audio Component"));

	//RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Attachment Root"));
	
	//Audio->AutoAttachParent = RootComponent;

}


//hmmm

inline UDAWSequencerData* AMusicScenePlayerActor::GetDAWSequencerData() const {

		return SessionData;
}

// Called when the game starts or when spawned
void AMusicScenePlayerActor::BeginPlay()
{
	Super::BeginPlay();

	if(!GetDAWSequencerData())
	{
		UE_LOG(LogTemp, Error, TEXT("No DAW Sequencer Data set on Music Scene Player Actor"))
		return;
	}

	//we need to do this because 'create sound 2d' is not reliable without a wav file, this one actually play but this adds a point of weakness to the system
	//in the form of the wav file.
	auto PrimingSound = FSoftObjectPath(TEXT("/unDAW/BKSystems/Core/PrimingAudioDontMove/1kSineTonePing.1kSineTonePing")).TryLoad();
	auto AsWavAsset = Cast<USoundWave>(PrimingSound);
	
	auto AudioComponent = UGameplayStatics::CreateSound2D(this, AsWavAsset, 1.0f, 1.0f, 0.0f, nullptr, true, false);
	GetDAWSequencerData()->AuditionBuilder(AudioComponent);

	if(!MaterialParameterCollection)
	{
		UE_LOG(LogTemp, Error, TEXT("No Material Parameter Collection set on Music Scene Player Actor"))
		//return;
	}
	else {
		//int32 DurationParam =	MaterialParameterCollection->ScalarParameters.IndexOfByPredicate([](const FMaterialParameterInfo& Info) {
		//	return Info.Name == FName("SongDuration");
		//});

		//MaterialParameterCollection->ScalarParameters[DurationParam].DefaultValue = GetDAWSequencerData()->SequenceDuration;

	}

	VideoSyncedMidiClock = NewObject<UMusicClockComponent>(this);
	VideoSyncedMidiClock->RegisterComponent();
	VideoSyncedMidiClock->ConnectToMetasoundOnAudioComponent(AudioComponent);
	VideoSyncedMidiClock->Start();

	MusicTempometer = NewObject<UMusicTempometerComponent>(this);
	MusicTempometer->RegisterComponent();
	MusicTempometer->SetMaterialParameterCollection(MaterialParameterCollection);
	MusicTempometer->SetClock(VideoSyncedMidiClock);


	//MusicTempometer
	
	//Audio->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	//if (GetActiveSessionData() && GetActiveSessionData()->SavedMetaSound)
	//{
	//	//GetActiveSessionData()->PerformanceMetaSound->OnGeneratorInstanceCreated.Add(this, &AMusicScenePlayerActor::PerformanceMetasoundGeneratorCreated);
	//	PerformanceAudioComponent = UGameplayStatics::CreateSound2D(this, GetActiveSessionData()->SavedMetaSound, 1.0f, 1.0f, 0.0f, nullptr, true);
	//	//PerformanceAudioComponent->AddToRoot();
	//	//PerformanceAudioComponent->SetSound();
	//	UMetasoundGeneratorHandle::CreateMetaSoundGeneratorHandle(PerformanceAudioComponent);
	//} 
	
}

void AMusicScenePlayerActor::PerformanceMetasoundGeneratorCreated(TSharedPtr<Metasound::FMetasoundGenerator> GeneratorPointer, ESPMode UserPolicy)
{
	UE_LOG(LogTemp, Log, TEXT("Yes Hello?"))
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
	//AudioComponent;
	//GeneratorHandle = UMetasoundGeneratorHandle::CreateMetaSoundGeneratorHandle(AudioComponent);
	//GeneratorHandle->WatchOutput(FName("Midi Stream"), delegateFunc);
	
}

void AMusicScenePlayerActor::UpdateWatchers()
{
	GeneratorHandle->UpdateWatchers();
}



