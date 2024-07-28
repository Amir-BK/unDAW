// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Vertexes/M2SoundVertex.h"
#include "M2ActionVertex.generated.h"

/**
 * 
 */
UCLASS()
class BKMUSICCORE_API UM2ActionVertex : public UM2SoundPatch
{
	GENERATED_BODY()

	public:

		TObjectPtr<AActor> AttachedActor = nullptr;

		TObjectPtr<USceneComponent> AttachedSceneComponent = nullptr;

		FName AttachedSocketName = NAME_None;
	
};
