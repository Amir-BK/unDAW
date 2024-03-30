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
	virtual void PostInitProperties() override;

	TArray<TRange<int>> NoteFilters;
	FName MidiOutputName;
	int IndexInSceneManager;

	



	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetSceneManager, Category="unDAW")
	TObjectPtr<AMusicScenePlayerActor> SceneManager;

	UFUNCTION(BlueprintSetter, Category = "unDAW")
	void SetSceneManager(AMusicScenePlayerActor* inSceneManager);

	// Implement this event to add extra initializations to the actor that should happen after it already received the relevant scene data
	UFUNCTION(BlueprintImplementableEvent)
	void InitEvent();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;




		
};
