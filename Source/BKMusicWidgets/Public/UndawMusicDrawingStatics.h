// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UndawMusicDrawingStatics.generated.h"

enum class EGridPointType : uint8
{
	Bar,
	Beat,
	Subdivision
};

struct FMusicalGridPoint
{
	EGridPointType Type = EGridPointType::Bar;
	int32 Bar = 0;
	int8 Beat = 1;
	int8 Subdivision = 1;
};

/**
 * 
 */
UCLASS()
class BKMUSICWIDGETS_API UUndawMusicDrawingStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
};
