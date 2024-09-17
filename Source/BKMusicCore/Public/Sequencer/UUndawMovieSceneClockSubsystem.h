// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Evaluation/IMovieSceneCustomClockSource.h"
#include "UUndawMovieSceneClockSubsystem.generated.h"



UCLASS()
class BKMUSICCORE_API UUndawClockProxyObject : public UObject, public IMovieSceneCustomClockSource
{
	GENERATED_BODY()

	UUndawClockProxyObject()
	{
		
		UE_LOG(LogTemp, Warning, TEXT("UUndawClockProxyObject::UUndawClockProxyObject"));
	}
};
/**
 * 
 */
UCLASS()
class BKMUSICCORE_API UUUndawMovieSceneClockSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override 
	
	{
		UE_LOG(LogTemp, Warning, TEXT("UUndawMovieSceneClockSubsystem::Initialize"));
		ClockProxyObject = NewObject<UUndawClockProxyObject>((UObject*)GetTransientPackage(), FName("Undaw MovieScene System Clock"), RF_Public | RF_Standalone);
		ClockProxyObject->AddToRoot();
		auto ClassPathNames = UMovieSceneCustomClockSource::StaticClass()->GetClassPathName();
		//print all class path names


	}
	
	UPROPERTY()
	TObjectPtr<UUndawClockProxyObject> ClockProxyObject;
};
