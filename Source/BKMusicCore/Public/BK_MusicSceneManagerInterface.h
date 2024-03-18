// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "BK_MusicSceneManagerInterface.generated.h"



UENUM(BlueprintType)
enum EBKTransportCommands : uint8
{
	Init,
	Play,
	Pause,
	Stop,
	Kill,
	TransportBackward,
	TransportForward,
	NextMarker,
	PrevMarker

};

UENUM(BlueprintType)
enum EBKPlayState : uint8
{
	Preparing,
	ReadyToPlay,
	Playing,
	Seeking,
	Paused

};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlaybackStateChanged, EBKPlayState, NewPlaystate);

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable, BlueprintType)
class UBK_MusicSceneManagerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class BKMUSICCORE_API IBK_MusicSceneManagerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "unDAW|Transport")
	const EBKPlayState GetCurrentPlaybackState();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "unDAW|Transport")
	bool SendTransportCommand(const EBKTransportCommands InCommand);

	virtual FOnPlaybackStateChanged* GetPlaybackStateDelegate() = 0;
	
};
