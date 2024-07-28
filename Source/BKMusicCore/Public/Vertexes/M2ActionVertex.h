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
		FName AttachedSocketName = NAME_None;

		UPROPERTY()
		USoundSourceBus* SourceBus = nullptr;

		UPROPERTY()
		UAudioBus* AudioBus = nullptr;

		UPROPERTY()
		TObjectPtr<UAudioComponent> AudioComponent = nullptr;


	
};
