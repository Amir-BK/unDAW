// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MetasoundGeneratorHandle.h"
#include "GameFramework/Actor.h"
#include "BK_MusicSceneManagerInterface.h"
#include "Components/AudioComponent.h"
#include "MusicScenePlayerActor.generated.h"

UCLASS()
class BKMUSICCORE_API AMusicScenePlayerActor : public AActor, public IBK_MusicSceneManagerInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMusicScenePlayerActor();

	// DELEGATES!!!

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "unDAW|Transport")
	FOnTransportSeekCommand TransportSeekDelegate;

	UPROPERTY(BlueprintAssignable, Category = "unDAW|Transport")
	FOnPlaybackStateChanged PlaystateDelegate;


	//PROPERTIES

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW|Audio Setup", meta = (ExposeOnSpawn = true))
	TSubclassOf<UMetasoundBuilderHelperBase> BuilderBPInstance;


	UPROPERTY()
	UMetasoundBuilderHelperBase* BuilderHelper;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "unDAW|Music Scene")
	UAudioComponent* PerformanceAudioComponent;

	UPROPERTY()
	UMetasoundGeneratorHandle* GeneratorHandle;	

	UPROPERTY()
	TEnumAsByte<EBKPlayState> PlayState = EBKPlayState::NotReady;

	UPROPERTY()
	float PlaybackCursorPosition = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "unDAW", meta=(DisplayPriority = "0"))
	TObjectPtr<UDAWSequencerData> SessionData;

	//METHODS

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// TODO [$65cfdef41013620009101dda]: implement clock init in MusicSceneActor, register to clock events and route them to subscribers
	UFUNCTION(BlueprintCallable, Category = "BK Music")
	virtual void InitClock(float inBPM);

	UFUNCTION(BlueprintCallable, Category = "BK Music")
	void UpdateWatchers();

	FOnTransportSeekCommand* GetSeekCommandDelegate() override;

	FOnPlaybackStateChanged* GetPlaybackStateDelegate() override;





	UFUNCTION()
	void SendSeekCommand(float InSeek) override;

	//This blueprint implementable event can be used to update native widgets 
	UFUNCTION(BlueprintImplementableEvent, CallInEditor, Category = "BK Music")
	void ReceivedSeekUpdate(float InSeek);

	UFUNCTION(BlueprintImplementableEvent, CallInEditor, Category = "BK Music")
	void BP_Initializations();

	virtual void Entry_Initializations() override;

	// Inherited via IBK_MusicSceneManagerInterface
	UAudioComponent* GetAudioComponent() override;

	// Inherited via IBK_MusicSceneManagerInterface
	const EBKPlayState GetCurrentPlaybackState() override;

	// Inherited via IBK_MusicSceneManagerInterface
	UDAWSequencerData* GetActiveSessionData() override;

	// Inherited via IBK_MusicSceneManagerInterface
	TSubclassOf<UMetasoundBuilderHelperBase> GetBuilderBPClass() override;

	// Inherited via IBK_MusicSceneManagerInterface
	void SetBuilderHelper(UMetasoundBuilderHelperBase* InBuilderHelper) override;
	UMetasoundBuilderHelperBase* GetBuilderHelper() override;
	void SetGeneratorHandle(UMetasoundGeneratorHandle* GeneratorHandle) override;
	UMetasoundGeneratorHandle* GetGeneratorHandle() override;
};
