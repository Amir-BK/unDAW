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

// Called every frame
void UDAWListenerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}