// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MusicScenePlayerActor.h"
#include "SequencerData.h"
#include "DAWListenerComponent.generated.h"


class UBKListnerComponentConfigWidget;

UCLASS( ClassGroup=(unDAW), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable )
class BKMUSICCORE_API UDAWListenerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDAWListenerComponent();

protected:

	friend class UBKListnerComponentConfigWidget;
	friend class AMusicScenePlayerActor;

	// Called when the game starts
	virtual void BeginPlay() override;



	TObjectPtr<AMusicScenePlayerActor> SceneManager;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;




		
};
