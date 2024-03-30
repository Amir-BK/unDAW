// Fill out your copyright notice in the Description page of Project Settings.


#include "MusicScenePlayerActor.h"
#include "Kismet/GameplayStatics.h"
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

// Called when the game starts or when spawned
void AMusicScenePlayerActor::BeginPlay()
{
	Super::BeginPlay();

	
	//PerformanceAudioComponent = UGameplayStatics::CreateSound2D(this, nullptr, 1.0f, 1.0f, 0.0f, nullptr, true, false);
	//Audio->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	if (GetActiveSessionData() && GetActiveSessionData()->PerformanceMetaSound)
	{
		//GetActiveSessionData()->PerformanceMetaSound->OnGeneratorInstanceCreated.Add(this, &AMusicScenePlayerActor::PerformanceMetasoundGeneratorCreated);
		PerformanceAudioComponent = UGameplayStatics::CreateSound2D(this, GetActiveSessionData()->PerformanceMetaSound, 1.0f, 1.0f, 0.0f, nullptr, true);
		//PerformanceAudioComponent->AddToRoot();
		//PerformanceAudioComponent->SetSound();
		UMetasoundGeneratorHandle::CreateMetaSoundGeneratorHandle(PerformanceAudioComponent);
	} 
	
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



void AMusicScenePlayerActor::Entry_Initializations()
{
	BP_Initializations();
}

UAudioComponent* AMusicScenePlayerActor::GetAudioComponent()
{
	
	return PerformanceAudioComponent;
}


UDAWSequencerData* AMusicScenePlayerActor::GetActiveSessionData()
{
	return SessionData;
}

TSubclassOf<UMetasoundBuilderHelperBase> AMusicScenePlayerActor::GetBuilderBPClass()
{
	return BuilderBPInstance;
}

void AMusicScenePlayerActor::SetBuilderHelper(UMetasoundBuilderHelperBase* InBuilderHelper)
{
	BuilderHelper = InBuilderHelper;
}

UMetasoundBuilderHelperBase* AMusicScenePlayerActor::GetBuilderHelper()
{
	return BuilderHelper;
}

void AMusicScenePlayerActor::SetGeneratorHandle(UMetasoundGeneratorHandle* InGeneratorHandle)
{
	GeneratorHandle = TSharedPtr<UMetasoundGeneratorHandle>(InGeneratorHandle);
}

UMetasoundGeneratorHandle* AMusicScenePlayerActor::GetGeneratorHandle()
{
	return GeneratorHandle.Get();
}


void AMusicScenePlayerActor::SendSeekCommand(float InSeek)
{
	PlaybackCursorPosition = InSeek;
	TransportSeekDelegate.Broadcast(InSeek);
	ReceivedSeekUpdate(InSeek);

	IBK_MusicSceneManagerInterface::SendSeekCommand(InSeek);
}


