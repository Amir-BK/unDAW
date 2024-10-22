// Fill out your copyright notice in the Description page of Project Settings.

#include "ListenerComponent/DAWListenerComponent.h"


// Sets default values for this component's properties
UDAWListenerComponent::UDAWListenerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UDAWListenerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (SceneManager)
	{
		UMetaSoundOutputSubsystem* MetaSoundOutputSubsystem = GetWorld()->GetSubsystem<UMetaSoundOutputSubsystem>();
		const auto& AudiotionComponent = SceneManager->GetDAWSequencerData()->AuditionComponent;

		if (MetaSoundOutputSubsystem && AudiotionComponent)


		{
			bool bIsDelegateAlreadyBound = OnMidiStreamOutputReceived.IsBound();

			UE_CLOG(bIsDelegateAlreadyBound, LogTemp, Warning, TEXT("OnMidiStreamOutputReceived is already bound!"));
			OnMidiStreamOutputReceived.BindLambda([this](FName OutputName, const FMetaSoundOutput Value) {
				ReceiveMetaSoundMidiStreamOutput(FName(WatchedOutput), Value);
				});
			bIsWatchingOutput = MetaSoundOutputSubsystem->WatchOutput(AudiotionComponent, FName(WatchedOutput), OnMidiStreamOutputReceived);
			if (bIsWatchingOutput)
			{

			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Could not find MetasoundOutputSubsystem or AudiotionComponent"));
		}
	}




	// ...
}

void UDAWListenerComponent::PostInitProperties()
{
	Super::PostInitProperties();

	if (SceneManager)
	{
		UE_LOG(LogTemp, Log, TEXT("we have a scene manager"))
	}
	else {
		UE_LOG(LogTemp, Log, TEXT("we don't have a scene manager"))
	}
}

void UDAWListenerComponent::SetSceneManager(AMusicScenePlayerActor* inSceneManager)
{
	SceneManager = inSceneManager;

	InitEvent();
}

void UDAWListenerComponent::ReceiveMetaSoundMidiStreamOutput(FName OutputName, const FMetaSoundOutput Value)
{
	FMidiEventInfo MidiEvent;

	const bool MidiEventGetSuccess = Value.Get(MidiEvent);
	if (MidiEventGetSuccess)
	{
		OnMidiEvent.Broadcast(MidiEvent);
	}
}

// Called every frame
void UDAWListenerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

#if WITH_EDITOR
void UDAWListenerComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	//when WatchedOutput is changed get the metadata from the scene manager
	if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UDAWListenerComponent, WatchedOutput))
	{
		if (SceneManager)
		{
			MidiTrackMetadata = *SceneManager->GetDAWSequencerData()->M2TrackMetadata.FindByPredicate([&](const FTrackDisplayOptions& Track) {
				return Track.TrackName == WatchedOutput;
				});
		}
	}
}
#endif
