// Fill out your copyright notice in the Description page of Project Settings.


#include "UndawMusicDrawingStatics.h"

namespace UnDAW
{
	TimelineConstants::EGridDensity FTimelineGridDensityCalculator::CalculateOptimalDensity(
		float PixelsPerTick, 
		float GeometryWidth,
		int32 TicksPerBar,
		int32 TicksPerBeat)
	{
		// Calculate pixel spacing for different musical intervals
		const float PixelsPerBeat = PixelsPerTick * TicksPerBeat;
		const float PixelsPerBar = PixelsPerTick * TicksPerBar;
		
		// Start with the most detailed level and work our way up
		// Check if we can show subdivisions (assuming 16th notes as default subdivision)
		const int32 TicksPerSubdivision = TicksPerBeat / 4; // 16th notes
		const float PixelsPerSubdivision = PixelsPerTick * TicksPerSubdivision;
		
		// Add hysteresis to prevent flickering by using larger thresholds
		// This makes the density changes less sensitive to small zoom adjustments
		const float HysteresisMultiplier = 1.5f;
		
		if (PixelsPerSubdivision >= TimelineConstants::MinPixelsPerMinorTick * HysteresisMultiplier)
		{
			return TimelineConstants::EGridDensity::Subdivisions;
		}
		
		// Check if we can show beats
		if (PixelsPerBeat >= TimelineConstants::MinPixelsPerMinorTick * HysteresisMultiplier)
		{
			return TimelineConstants::EGridDensity::Beats;
		}
		
		// Check if we can show all bars - use larger threshold to prevent bar 3 from disappearing
		if (PixelsPerBar >= TimelineConstants::MinPixelsPerMajorTick * HysteresisMultiplier)
		{
			return TimelineConstants::EGridDensity::Bars;
		}
		
		// Check if we can show every other bar - use even larger threshold
		if (PixelsPerBar * 2 >= TimelineConstants::MinPixelsPerMajorTick * HysteresisMultiplier)
		{
			return TimelineConstants::EGridDensity::SparseBar;
		}
		
		// Show every 4th bar as a last resort
		return TimelineConstants::EGridDensity::VerySparseBars;
	}

	int32 FTimelineGridDensityCalculator::GetMinimumInterval(
		TimelineConstants::EGridDensity Density,
		int32 TicksPerBar,
		int32 TicksPerBeat)
	{
		switch (Density)
		{
		case TimelineConstants::EGridDensity::Subdivisions:
			return TicksPerBeat / 4; // 16th notes
		case TimelineConstants::EGridDensity::Beats:
			return TicksPerBeat;
		case TimelineConstants::EGridDensity::Bars:
			return TicksPerBar;
		case TimelineConstants::EGridDensity::SparseBar:
			return TicksPerBar * 2;
		case TimelineConstants::EGridDensity::VerySparseBars:
			return TicksPerBar * 4;
		default:
			return TicksPerBar;
		}
	}

	bool FTimelineGridDensityCalculator::ShouldShowGridPoint(
		const FMusicalGridPoint& GridPoint,
		TimelineConstants::EGridDensity Density,
		int32 BarNumber)
	{
		switch (Density)
		{
		case TimelineConstants::EGridDensity::Subdivisions:
			return true; // Show all grid points
			
		case TimelineConstants::EGridDensity::Beats:
			return GridPoint.Type != EGridPointType::Subdivision;
			
		case TimelineConstants::EGridDensity::Bars:
			return GridPoint.Type == EGridPointType::Bar;
			
		case TimelineConstants::EGridDensity::SparseBar:
			return GridPoint.Type == EGridPointType::Bar && (BarNumber % 2 == 1);
			
		case TimelineConstants::EGridDensity::VerySparseBars:
			return GridPoint.Type == EGridPointType::Bar && (BarNumber % 4 == 1);
			
		default:
			return GridPoint.Type == EGridPointType::Bar;
		}
	}

	bool FTimelineGridDensityCalculator::ShouldShowBarText(
		int32 BarNumber,
		TimelineConstants::EGridDensity Density,
		float PixelsPerBar)
	{
		// If bars are very close together, show text less frequently
		if (PixelsPerBar < TimelineConstants::MinPixelsPerBarText)
		{
			switch (Density)
			{
			case TimelineConstants::EGridDensity::Bars:
				return BarNumber % 2 == 1; // Show every other bar
			case TimelineConstants::EGridDensity::SparseBar:
				return BarNumber % 2 == 1; // Already sparse, show all visible bars
			case TimelineConstants::EGridDensity::VerySparseBars:
				return true; // Very sparse already, show all visible bars
			default:
				return BarNumber % 4 == 1; // Show every 4th bar for denser modes
			}
		}
		
		// If we have enough space, show text for all visible bars
		return ShouldShowGridPoint(FMusicalGridPoint{EGridPointType::Bar, BarNumber, 1, 1}, Density, BarNumber);
	}
}

