// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Vertexes/M2SoundVertex.h"
#include "Sound/SoundSourceBus.h"
#include "Sound/AudioBus.h"
#include "Components/SceneComponent.h"
#include "M2ActionVertex.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class BKMUSICCORE_API UM2ActionVertex : public UM2SoundPatch
{
	GENERATED_BODY()

	public:

		UPROPERTY()
		TObjectPtr<USceneComponent> AttachedSceneComponent = nullptr;

		UPROPERTY()
		TArray<FMetaSoundNodeHandle> AdditionalNodes;

		UPROPERTY()
		FName AttachedSocketName = NAME_None;

		UPROPERTY()
		USoundSourceBus* SourceBus = nullptr;

		UPROPERTY()
		UAudioBus* AudioBus = nullptr;

		//UMidiFile* MidiClip = nullptr;

		UPROPERTY(BlueprintReadOnly, Category = "Musical Gameplay")
		TObjectPtr<UAudioComponent> AudioComponent = nullptr;

		// Executing a trigger on a patch only works if the trigger is not connected to a pin in the metasound graph
		UFUNCTION(BlueprintCallable, Category = "Musical Gameplay")
		void ExecuteTriggerOnPatch(FName TriggerName);

		//This function can be called after the vertex was created, it can be used to connect the pins of the vertex to existing graph aliases
		UFUNCTION(BlueprintCallable, Category = "Musical Gameplay")
		void ConnectPinsToExistingAliases(TMap<FName, FName> InInputPinToGraphAliasMap, TMap<FName, FName> InOutputPinToGraphAliasMap) {};


	
};
