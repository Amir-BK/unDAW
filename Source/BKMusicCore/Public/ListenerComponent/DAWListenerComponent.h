// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HarmonixMetasound/DataTypes/MidiEventInfo.h"
#include "MusicScenePlayerActor.h"
#include "M2SoundGraphData.h"
#include "MetasoundGeneratorHandle.h"
#include "MetasoundOutputSubsystem.h"
#include "DAWListenerComponent.generated.h"

class UBKListnerComponentConfigWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMetasoundMidiNoteEvent, FMidiEventInfo , Event);

UCLASS(ClassGroup = (unDAW), meta = (BlueprintSpawnableComponent), BlueprintType, Blueprintable)
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

	TArray<FString> MidiOutputNames;

	UFUNCTION(BlueprintCallable, Category = "unDAW")
	TArray<FString> GetMidiOutputNames()
	{
		if (SceneManager)
		{
			return SceneManager->GetMidiOutputNames();
		}

		return TArray<FString>{TEXT("None")};
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unDAW", meta = (GetOptions = "GetMidiOutputNames"))
	FString WatchedOutput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetSceneManager, Category = "unDAW", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<AMusicScenePlayerActor> SceneManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "unDAW")
	FTrackDisplayOptions MidiTrackMetadata;

	UFUNCTION(BlueprintSetter, Category = "unDAW")
	void SetSceneManager(AMusicScenePlayerActor* inSceneManager);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "unDAW")
	bool bIsWatchingOutput = false;

	// Implement this event to add extra initializations to the actor that should happen after it already received the relevant scene data
	UFUNCTION(BlueprintImplementableEvent)
	void InitEvent();

	UFUNCTION()
	void ReceiveMetaSoundMidiStreamOutput(FName OutputName, const FMetaSoundOutput Value);

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#if WITH_EDITOR
	// post init edit
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	FOnMetasoundOutputValueChangedNative OnMidiStreamOutputReceived;

	UPROPERTY(BlueprintAssignable, Category = "unDAW")
	FOnMetasoundMidiNoteEvent OnMidiEvent;
};
