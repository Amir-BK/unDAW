// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Evaluation/IMovieSceneCustomClockSource.h"
#include "UUndawMovieSceneClockSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class BKMUSICCORE_API UUUndawMovieSceneClockSubsystem : public UEngineSubsystem, public IMovieSceneCustomClockSource
{
	GENERATED_BODY()
	
};
