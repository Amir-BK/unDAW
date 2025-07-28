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

	// Constants for timeline marking density
	namespace TimelineConstants
	{
		/** The minimum amount of pixels between each major ticks on the widget */
		constexpr int32 MinPixelsPerMajorTick = 50;

		/** The minimum amount of pixels between each minor ticks on the widget */
		constexpr int32 MinPixelsPerMinorTick = 20;

		/** The minimum amount of pixels between bar numbers/text */
		constexpr int32 MinPixelsPerBarText = 80;

		/** Grid density levels from most dense to least dense */
		enum class EGridDensity : uint8
		{
			Subdivisions,	// Show bars, beats, and subdivisions
			Beats,			// Show bars and beats
			Bars,			// Show only bars
			SparseBar,		// Show every other bar
			VerySparseBars	// Show every 4th bar
		};
	}

	/**
	 * Determines optimal grid density based on zoom level and available space
	 */
	struct BKMUSICWIDGETS_API FTimelineGridDensityCalculator
	{
		static TimelineConstants::EGridDensity CalculateOptimalDensity(
			float PixelsPerTick, 
			float GeometryWidth,
			int32 TicksPerBar,
			int32 TicksPerBeat
		);

		/**
		 * Gets the minimum interval (in ticks) that should be shown at the given density level
		 */
		static int32 GetMinimumInterval(
			TimelineConstants::EGridDensity Density,
			int32 TicksPerBar,
			int32 TicksPerBeat
		);

		/**
		 * Determines if a grid point should be shown based on the current density level
		 */
		static bool ShouldShowGridPoint(
			const FMusicalGridPoint& GridPoint,
			TimelineConstants::EGridDensity Density,
			int32 BarNumber
		);

		/**
		 * Determines if bar text should be shown for this bar based on density
		 */
		static bool ShouldShowBarText(
			int32 BarNumber,
			TimelineConstants::EGridDensity Density,
			float PixelsPerBar
		);
	};
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
