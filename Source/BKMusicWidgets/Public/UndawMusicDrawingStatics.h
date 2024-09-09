// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UndawMusicDrawingStatics.generated.h"

namespace UnDAW
{
	enum class EGridPointType : uint8
	{
		Bar,
		Beat,
		Subdivision
	};

	enum class EMusicTimeLinePaintMode : uint8
	{
		Music,
		Seconds
	};

	enum class ETimeDisplayMode : uint8
	{
		TimeLinear,
		TickLinear,
		BeatLinear
	};

	struct FMusicalGridPoint
	{
		EGridPointType Type = EGridPointType::Bar;
		int32 Bar = 0;
		int8 Beat = 1;
		int8 Subdivision = 1;
	};

	typedef TMap<int32, FMusicalGridPoint> FGridPointMap;
}

class SMidiEditorPanelBase;

/**
 * 
 */
UCLASS()
class BKMUSICWIDGETS_API UUndawMusicDrawingStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	
};
