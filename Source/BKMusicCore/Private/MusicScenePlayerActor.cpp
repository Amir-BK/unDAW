// Fill out your copyright notice in the Description page of Project Settings.


#include "MusicScenePlayerActor.h"

#include "MetasoundGeneratorHandle.h"

// Sets default values
AMusicScenePlayerActor::AMusicScenePlayerActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("Scene Audio Component"));
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Attachment Root"));
	//Audio->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	Audio->AutoAttachParent = RootComponent;

}

// Called when the game starts or when spawned
void AMusicScenePlayerActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMusicScenePlayerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
    // TODO [$65cfdef41013620009101dd9]: implement time keeping and cursor updates vs. the game thread

}

void delegateFunc(FName Output, const FMetaSoundOutput& MetaSoundOutput)
{
	UE_LOG(LogTemp, Log, TEXT("What"))
}

void AMusicScenePlayerActor::InitClock(float inBPM)
{
    //AudioComponent;
	//GeneratorHandle = UMetasoundGeneratorHandle::CreateMetaSoundGeneratorHandle(AudioComponent);
	//GeneratorHandle->WatchOutput(FName("Midi Stream"), delegateFunc);
    
}

void AMusicScenePlayerActor::UpdateWatchers()
{
	GeneratorHandle->UpdateWatchers();
}

FOnTransportSeekCommand* AMusicScenePlayerActor::GetSeekCommandDelegate()
{
	return &TransportSeekDelegate;
}

FOnPlaybackStateChanged* AMusicScenePlayerActor::GetPlaybackStateDelegate()
{
	return &PlaystateDelegate;
}



UAudioComponent* AMusicScenePlayerActor::GetAudioComponent()
{
	return Audio;
}

const EBKPlayState AMusicScenePlayerActor::GetCurrentPlaybackState()
{
	return PlayState;
}

UDAWSequencerData* AMusicScenePlayerActor::GetActiveSessionData()
{
	return SessionData;
}

void AMusicScenePlayerActor::SendTransportCommand(const EBKTransportCommands InCommand)
{
	//TODODOTOD	TODTO
	UE_LOG(LogTemp, Log, TEXT("But do we enter here?"))

	switch (InCommand)
	{
	case Init:
		// create builder
		UE_LOG(LogTemp, Log, TEXT("Received Init"))
		break;
	case Play:

		UE_LOG(LogTemp,Log, TEXT("Received Play"))
		break;
	case Pause:
		break;
	case Stop:
		break;
	case Kill:
		break;
	case TransportBackward:
		break;
	case TransportForward:
		break;
	case NextMarker:
		break;
	case PrevMarker:
		break;
	default:
		break;
	}

	
}

void AMusicScenePlayerActor::SendSeekCommand(float InSeek)
{
	PlaybackCursorPosition = InSeek;
	TransportSeekDelegate.Broadcast(InSeek);
	ReceivedSeekUpdate(InSeek);
}


