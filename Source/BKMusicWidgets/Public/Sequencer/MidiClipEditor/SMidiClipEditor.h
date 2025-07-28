// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Events.h"
#include "Widgets/SCompoundWidget.h"
#include "M2SoundGraphData.h"
#include "UndawMusicDrawingStatics.h"
#include "SlateFwd.h"
#include "Components/Widget.h"
#include "Widgets/Layout/SSplitter.h"
#include "Styling/AppStyle.h"
#include <AudioWidgetsSlateTypes.h>


/**
implementations for panning and zooming

*/

namespace UnDAW
{
	const TRange<int> MidiNoteRange{ 0, 127 };
}

DECLARE_DELEGATE_OneParam(
	FOnPanelZoomChangedByUser,
	FVector2D);

DECLARE_DELEGATE_TwoParams(
	FOnPanelPositionChangedByUser,
	FVector2D, bool);

// Midi editor panel base class provides basic functionality for drawing a musical editor
// this includes panning, zooming, providing methods to paint grid points and timeline
// and receiving play cursor, to sync two panels together provide a shared position offset attribute
// also drawing the resizable 'major tab' 
class SMidiEditorPanelBase : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SMidiEditorPanelBase)
		{
		}
		SLATE_ARGUMENT_DEFAULT(float, TimelineHeight) = 25.0f;
		SLATE_ATTRIBUTE(FVector2D, Position);
		SLATE_ATTRIBUTE(FMusicTimestamp, PlayCursor);
		SLATE_ATTRIBUTE(bool, bFollowCursor)
		SLATE_EVENT(FOnPanelPositionChangedByUser, OnPanelPositionChangedByUser);
		SLATE_EVENT(FOnPanelZoomChangedByUser, OnPanelZoomByUser);
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UDAWSequencerData* InSequence)
	{
		SequenceData = InSequence;
		TimelineHeight = InArgs._TimelineHeight;
		Position = InArgs._Position;
		OnPanelPositionChangedByUser = InArgs._OnPanelPositionChangedByUser;
		OnPanelZoomByUser = InArgs._OnPanelZoomByUser;
		PlayCursor = InArgs._PlayCursor;
		bFollowCursor = InArgs._bFollowCursor;

		RecalculateGrid();
	};

	// called to set the position offset of the panel without firing the delegates, i.e when an external source changes the position
	void SetPosition(FVector2D InPosition, bool bIgnoreVertical = false)
	{
		// Ignore y component if bLockVerticalPan
		if (bIgnoreVertical || bLockVerticalPan)
		{
			InPosition.Y = Position.Get().Y;
		}

		Position.Set(InPosition);
		RecalculateGrid();
	}

	FVector2D PositionOffset = FVector2D(0, 0);
	TAttribute<FVector2D> Position = FVector2D(0, 0);
	TAttribute<FMusicTimestamp> PlayCursor = FMusicTimestamp();
	TAttribute<FVector2D> Zoom = FVector2D(1, 1);
	TAttribute<bool> bFollowCursor = true;
	FOnPanelPositionChangedByUser OnPanelPositionChangedByUser;
	FOnPanelZoomChangedByUser OnPanelZoomByUser;
	FSlateBrush* PlayCursorHandleBrush = nullptr;


	bool bIsPanActive = false;
	bool bLockVerticalPan = false;
	UnDAW::FGridPointMap GridPoints;
	UnDAW::EGridPointType GridPointType = UnDAW::EGridPointType::Bar;
	UnDAW::EMusicTimeLinePaintMode PaintMode = UnDAW::EMusicTimeLinePaintMode::Music;
	UnDAW::ETimeDisplayMode TimeDisplayMode = UnDAW::ETimeDisplayMode::TimeLinear;
	float RowHeight = 10.0f;
	float TextHeight = 10.0f;
	float MajorTabWidth = 0.0f;
	float MajorTabAlpha = 0.0f;
	float TimelineHeight;

	UDAWSequencerData* SequenceData = nullptr;

	TRange<int> ContentRange;

	TOptional<EMouseCursor::Type> GetCursor() const override
	{

		if (bIsPanActive)
		{
			return EMouseCursor::GrabHand;
		}

		return TOptional<EMouseCursor::Type>();
	}



	FReply OnMousePan(const FGeometry & MyGeometry, const FPointerEvent & MouseEvent)
	{
		if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
		{
			auto& CurrentPos = Position.Get();
			const float ContentWidth = TickToPixel(SequenceData->HarmonixMidiFile->GetLastEventTick());

			// Only calculate Y delta if vertical pan is NOT locked
			const float ContentHeight = bLockVerticalPan ? 0 : 127 * RowHeight;
			const float DeltaY = bLockVerticalPan ? 0 : MouseEvent.GetCursorDelta().Y;

			const auto NewPositionX = FMath::Clamp(
				CurrentPos.X + MouseEvent.GetCursorDelta().X,
				-ContentWidth + MyGeometry.Size.X,
				0.0f
			);

			const auto NewPositionY = bLockVerticalPan ?
				CurrentPos.Y : // Preserve existing Y
				FMath::Clamp(
					CurrentPos.Y + DeltaY,
					-ContentHeight + MyGeometry.Size.Y,
					0.0f
				);

			const FVector2D NewPosition = FVector2D(NewPositionX, NewPositionY);

			if (OnPanelPositionChangedByUser.IsBound())
			{
				// Pass bLockVerticalPan to enforce Y ignore in linked panels
				OnPanelPositionChangedByUser.Execute(NewPosition, bLockVerticalPan);
			}
			else
			{
				SetPosition(NewPosition);
			}

			bIsPanActive = true;
			return FReply::Handled().CaptureMouse(AsShared());
		}
		bIsPanActive = false;
		return FReply::Unhandled();
	}


	FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		 if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton && bIsPanActive)
		{
			bIsPanActive = false;
			return FReply::Handled().ReleaseMouseCapture();
		}
		return SCompoundWidget::OnMouseButtonUp(MyGeometry, MouseEvent);
	}

	const FVector2D GetLocalMousePosition(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
		return MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()) - Position.Get();
	}

	void SetZoom(const FVector2D NewZoom)
	{


		Zoom.Set(NewZoom);
	}

	FReply OnZoom(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
	{

		bool bIsCtrlPressed = MouseEvent.IsControlDown();
		bool bIsShiftPressed = MouseEvent.IsShiftDown();
		FVector2D NewZoom;

		if (MouseEvent.GetWheelDelta() > 0)
		{
			if (bIsShiftPressed)
			{
				//VerticalZoom += 0.1f;
				NewZoom = FVector2D(Zoom.Get().X, Zoom.Get().Y * 1.1f);

			}
			else if (bIsCtrlPressed)
			{
				NewZoom = FVector2D(Zoom.Get().X * 1.1f, Zoom.Get().Y);

			}
			else {
				OnVerticalScroll(MouseEvent.GetWheelDelta());
				return FReply::Handled();
			}
		}
		else
		{
			if (bIsShiftPressed)
			{
				NewZoom = FVector2D(Zoom.Get().X, Zoom.Get().Y * 0.9f);

			}
			else if (bIsCtrlPressed)
			{
				NewZoom = FVector2D(Zoom.Get().X * 0.9f, Zoom.Get().Y);

			}
			else {
				OnVerticalScroll(MouseEvent.GetWheelDelta());
				return FReply::Handled();
			}
		}

		if (OnPanelZoomByUser.IsBound())
		{
			OnPanelZoomByUser.Execute(NewZoom);
		}
		else {

			SetZoom(NewZoom);
		}

		return FReply::Handled();
	}

	const float TickToPixel(const float Tick) const
	{
		return (SequenceData->HarmonixMidiFile->GetSongMaps()->TickToMs(Tick) * Zoom.Get().X) + Position.Get().X + MajorTabWidth;
	}

	const float PixelToTick(const float Pixel) const
	{
		// Step 1: Remove position and tab offset
		float Ms = (Pixel - Position.Get().X - MajorTabWidth) / Zoom.Get().X;
		// Step 2: Convert ms to tick
		return SequenceData->HarmonixMidiFile->GetSongMaps()->MsToTick(Ms);
	}
 
	virtual void OnVerticalScroll(float ScrollAmount) {};

	FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		return OnMousePan(MyGeometry, MouseEvent);
	}

	FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{

		return OnZoom(MyGeometry, MouseEvent);
	}

	virtual void CacheDesiredSize(float InLayoutScaleMultiplier) override {
		RecalculateGrid();
	}

	void RecalculateGrid(const FGeometry* OptionalGeometry = nullptr)
	{
		GridPoints.Empty();
		const auto& SongsMap = SequenceData->HarmonixMidiFile->GetSongMaps();

		// Use provided geometry or try to get cached geometry, with fallback
		FVector2D GeometrySize;
		if (OptionalGeometry)
		{
			GeometrySize = OptionalGeometry->GetLocalSize();
		}
		else
		{
			const FGeometry& CachedGeom = GetCachedGeometry();
			GeometrySize = CachedGeom.GetLocalSize();
			
			// If cached geometry is invalid, use a reasonable default to ensure timeline draws
			if (GeometrySize.X <= 0.0f || GeometrySize.Y <= 0.0f)
			{
				GeometrySize = FVector2D(1920.0f, 1080.0f); // Fallback size
			}
		}

		// Calculate visible range based on current position and zoom
		const float VisibleStartTick = SongsMap->MsToTick((-Position.Get().X - MajorTabWidth) / Zoom.Get().X);
		const float VisibleEndTick = SongsMap->MsToTick((GeometrySize.X - Position.Get().X - MajorTabWidth) / Zoom.Get().X);

		// Calculate density parameters for optimal spacing
		const float TicksPerMs = 1.0f / SongsMap->TickToMs(1.0f);
		const float PixelsPerTick = Zoom.Get().X / TicksPerMs;
		
		// Get musical timing information - using default 4/4 time if not available
		const int32 TicksPerBar = SongsMap->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Bar, 0);
		const int32 TicksPerBeat = SongsMap->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Beat, 0);
		
		// Calculate optimal grid density
		const auto OptimalDensity = UnDAW::FTimelineGridDensityCalculator::CalculateOptimalDensity(
			PixelsPerTick, 
			GeometrySize.X, 
			TicksPerBar, 
			TicksPerBeat
		);

		// For display purposes, we want "Bar 1" to always appear at the beginning of our timeline
		// regardless of the MIDI file's internal StartBar value
		int32 DisplayBarNumber = 1;
		
		// Start from tick 0 (beginning of timeline) and increment by bar durations
		float BarTick = 0.0f;

		// If we're viewing later in the timeline, skip forward to the first visible bar
		while (BarTick < VisibleStartTick && DisplayBarNumber <= 200)
		{
			BarTick += SongsMap->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Bar, BarTick);
			DisplayBarNumber++;
		}

		// Add grid points for all bars in the visible range based on density
		while (BarTick <= VisibleEndTick && DisplayBarNumber <= 200) // Safety limit
		{
			// Create a grid point for this bar
			const UnDAW::FMusicalGridPoint BarGridPoint = {
				UnDAW::EGridPointType::Bar,
				DisplayBarNumber,
				1,
				0
			};

			// Only add the grid point if it should be shown at current density
			if (UnDAW::FTimelineGridDensityCalculator::ShouldShowGridPoint(BarGridPoint, OptimalDensity, DisplayBarNumber))
			{
				GridPoints.Add(BarTick, BarGridPoint);
				
				// Add beats and subdivisions if density allows
				if (OptimalDensity == UnDAW::TimelineConstants::EGridDensity::Subdivisions || 
					OptimalDensity == UnDAW::TimelineConstants::EGridDensity::Beats)
				{
					AddBeatsAndSubdivisions(BarTick, DisplayBarNumber, OptimalDensity, SongsMap, VisibleEndTick);
				}
			}

			// Move to next bar
			BarTick += SongsMap->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Bar, BarTick);
			DisplayBarNumber++;
		}

		// Store current density for use in painting
		CurrentGridDensity = OptimalDensity;

		// Update row height for vertical scaling
		const float LastRowHeight = RowHeight;
		RowHeight = 10.0f * Zoom.Get().Y;
	}

	int32 PaintTimeline(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
	{
		// Recalculate grid with current geometry to ensure we have up-to-date grid points
		const_cast<SMidiEditorPanelBase*>(this)->RecalculateGrid(&AllottedGeometry);

		const bool bShouldPaintTimelineBar = TimelineHeight > 0.0f;

		if (bShouldPaintTimelineBar)
		{
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(
					FVector2f(AllottedGeometry.Size.X - MajorTabWidth, TimelineHeight),
					FSlateLayoutTransform(FVector2f(MajorTabWidth, 0))
				),
				FAppStyle::GetBrush("Graph.Panel.SolidBackground"),
				ESlateDrawEffect::None,
				FLinearColor::Black
			);
		}

		// Setup drawing parameters
		const auto* MidiSongMap = SequenceData->HarmonixMidiFile->GetSongMaps();
		const float Height = (AllottedGeometry.Size.Y / 127.0f) / Zoom.Get().Y;
		const FLinearColor BarLineColor = FLinearColor::Gray.CopyWithNewOpacity(0.1f);
		const FLinearColor BeatLineColor = FLinearColor::Blue.CopyWithNewOpacity(0.1f);
		const FLinearColor SubdivisionLineColor = FLinearColor::Black.CopyWithNewOpacity(0.05f);
		const FLinearColor BarTextColor = FLinearColor::Gray.CopyWithNewOpacity(0.5f);
		
		// Use the proper font constructor
		const FSlateFontInfo LargeFont = FCoreStyle::GetDefaultFontStyle("Regular", 14);
		const FSlateFontInfo SmallFont = FCoreStyle::GetDefaultFontStyle("Regular", 7);

		// Calculate pixels per bar for text density decisions
		const float TicksPerBar = MidiSongMap->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Bar, 0);
		const float TicksPerMs = 1.0f / MidiSongMap->TickToMs(1.0f);
		const float PixelsPerTick = Zoom.Get().X / TicksPerMs;
		const float PixelsPerBar = PixelsPerTick * TicksPerBar;

		// Draw grid points
		for (const auto& [OriginTick, GridPoint] : GridPoints)
		{
			const float Tick = OriginTick - GetStartOffset();
			const float PixelPosition = TickToPixel(Tick);

			// Only draw timeline elements if they're at or to the right of the controls area
			const bool bShouldDrawTimelineElements = PixelPosition >= MajorTabWidth;

			switch (GridPoint.Type)
			{
			case UnDAW::EGridPointType::Bar:
				if (bShouldPaintTimelineBar && bShouldDrawTimelineElements)
				{
					// Only show bar text if density calculator says we should
					if (UnDAW::FTimelineGridDensityCalculator::ShouldShowBarText(GridPoint.Bar, CurrentGridDensity, PixelsPerBar))
					{
						// Bar number
						FSlateDrawElement::MakeText(
							OutDrawElements,
							LayerId + 1, // Ensure text is above background
							AllottedGeometry.ToPaintGeometry(
								FVector2f(50.0f, Height),
								FSlateLayoutTransform(FVector2f(PixelPosition, 0))
							),
							FText::FromString(FString::FromInt(GridPoint.Bar)),
							LargeFont,
							ESlateDrawEffect::None,
							BarTextColor
						);

						// Beat number
						FSlateDrawElement::MakeText(
							OutDrawElements,
							LayerId + 1,
							AllottedGeometry.ToPaintGeometry(
								FVector2f(50.0f, Height),
								FSlateLayoutTransform(FVector2f(PixelPosition, 18))
							),
							FText::FromString(FString::FromInt(GridPoint.Beat)),
							SmallFont,
							ESlateDrawEffect::None,
							FLinearColor::White
						);

						// Subdivision number
						FSlateDrawElement::MakeText(
							OutDrawElements,
							LayerId + 1,
							AllottedGeometry.ToPaintGeometry(
								FVector2f(50.0f, Height),
								FSlateLayoutTransform(FVector2f(PixelPosition, 30))
							),
							FText::FromString(FString::FromInt(GridPoint.Subdivision)),
							SmallFont,
							ESlateDrawEffect::None,
							FLinearColor::White
						);
					}
				}

				// Only draw bar lines if they're at or to the right of the controls area
				if (bShouldDrawTimelineElements)
				{
					FSlateDrawElement::MakeLines(
						OutDrawElements,
						LayerId + 2, // Lines above background and text
						AllottedGeometry.ToPaintGeometry(),
						{
							FVector2D(PixelPosition, 0),
							FVector2D(PixelPosition, AllottedGeometry.GetLocalSize().Y)
						},
						ESlateDrawEffect::None,
						BarLineColor,
						false,
						1.0f
					);
				}
				break;

			case UnDAW::EGridPointType::Beat:
				// Only draw beat lines if they're at or to the right of the controls area
				if (bShouldDrawTimelineElements)
				{
					FSlateDrawElement::MakeLines(
						OutDrawElements,
						LayerId + 2,
						AllottedGeometry.ToPaintGeometry(),
						{
							FVector2D(PixelPosition, 0),
							FVector2D(PixelPosition, AllottedGeometry.GetLocalSize().Y)
						},
						ESlateDrawEffect::None,
						BeatLineColor,
						false,
						1.0f
					);
				}
				break;

			case UnDAW::EGridPointType::Subdivision:
				// Only draw subdivision lines if they're at or to the right of the controls area
				if (bShouldDrawTimelineElements)
				{
					FSlateDrawElement::MakeLines(
						OutDrawElements,
						LayerId + 2,
						AllottedGeometry.ToPaintGeometry(),
						{
							FVector2D(PixelPosition, 0),
							FVector2D(PixelPosition, AllottedGeometry.GetLocalSize().Y)
						},
						ESlateDrawEffect::None,
						SubdivisionLineColor,
						false,
						1.0f
					);
				}
				break;
			}
		}

		return LayerId + 3; // Incremented LayerId based on drawn elements
	}

	virtual float GetStartOffset() const
	{
		return 0.0f;
	}

	int32 PaintPlayCursor(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
	{
		const auto& PlayCursorTick = SequenceData->HarmonixMidiFile->GetSongMaps()->CalculateMidiTick(PlayCursor.Get(), EMidiClockSubdivisionQuantization::None) - GetStartOffset();
		const float CursorPixel = TickToPixel(PlayCursorTick);

		// Don't create offset geometry - use the base geometry
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),  // Use base geometry
			{ FVector2D(CursorPixel, 0), FVector2D(CursorPixel, AllottedGeometry.GetLocalSize().Y) },
			ESlateDrawEffect::None,
			FLinearColor::Red
		);

		if (TimelineHeight > 0.0f)
		{
			constexpr float PlayCursorWidth = 10.0f;
			constexpr float PlayCursorHalfWidth = PlayCursorWidth / 2.0f;

			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(
					FVector2f(PlayCursorWidth, TimelineHeight),
					FSlateLayoutTransform(1.0f, FVector2f(CursorPixel - PlayCursorHalfWidth, 0))
				),
				FAppStyle::GetBrush("Graph.Panel.SolidBackground"),
				ESlateDrawEffect::None,
				FLinearColor(255.f, 0.1f, 0.2f, 1.f)
			);
		}

		return LayerId;
	}

private:
	// Store current grid density for painting decisions
	UnDAW::TimelineConstants::EGridDensity CurrentGridDensity = UnDAW::TimelineConstants::EGridDensity::Bars;

	// Helper method to add beats and subdivisions for a bar
	void AddBeatsAndSubdivisions(float BarStartTick, int32 BarNumber, UnDAW::TimelineConstants::EGridDensity Density, const FSongMaps* SongsMap, float VisibleEndTick)
	{
		const int32 TicksPerBeat = SongsMap->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Beat, BarStartTick);
		const int32 TicksPerBar = SongsMap->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Bar, BarStartTick);
		
		// Assuming 4/4 time signature for now - could be made dynamic later
		const int32 BeatsPerBar = 4;
		
		for (int32 Beat = 1; Beat <= BeatsPerBar; Beat++)
		{
			const float BeatTick = BarStartTick + (Beat - 1) * TicksPerBeat;
			
			if (BeatTick > VisibleEndTick) break;
			if (BeatTick == BarStartTick) continue; // Skip beat 1 as it's the same as the bar
			
			const UnDAW::FMusicalGridPoint BeatGridPoint = {
				UnDAW::EGridPointType::Beat,
				BarNumber,
				static_cast<int8>(Beat),
				1
			};
			
			if (UnDAW::FTimelineGridDensityCalculator::ShouldShowGridPoint(BeatGridPoint, Density, BarNumber))
			{
				GridPoints.Add(BeatTick, BeatGridPoint);
				
				// Add subdivisions if density allows
				if (Density == UnDAW::TimelineConstants::EGridDensity::Subdivisions)
				{
					AddSubdivisions(BeatTick, BarNumber, Beat, SongsMap, VisibleEndTick);
				}
			}
		}
	}

	// Helper method to add subdivisions for a beat
	void AddSubdivisions(float BeatStartTick, int32 BarNumber, int32 BeatNumber, const FSongMaps* SongsMap, float VisibleEndTick)
	{
		const int32 TicksPerBeat = SongsMap->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Beat, BeatStartTick);
		
		// Assuming 16th note subdivisions (4 per beat)
		const int32 SubdivisionsPerBeat = 4;
		const int32 TicksPerSubdivision = TicksPerBeat / SubdivisionsPerBeat;
		
		for (int32 Subdivision = 1; Subdivision <= SubdivisionsPerBeat; Subdivision++)
		{
			const float SubdivisionTick = BeatStartTick + (Subdivision - 1) * TicksPerSubdivision;
			
			if (SubdivisionTick > VisibleEndTick) break;
			if (SubdivisionTick == BeatStartTick) continue; // Skip subdivision 1 as it's the same as the beat
			
			const UnDAW::FMusicalGridPoint SubdivisionGridPoint = {
				UnDAW::EGridPointType::Subdivision,
				BarNumber,
				static_cast<int8>(BeatNumber),
				static_cast<int8>(Subdivision)
			};
			
			GridPoints.Add(SubdivisionTick, SubdivisionGridPoint);
		}
	}
};

/**
 * 
 */
class BKMUSICWIDGETS_API SMidiClipEditor : public SMidiEditorPanelBase
{
public:
	SLATE_BEGIN_ARGS(SMidiClipEditor)
	{}
		SLATE_ARGUMENT_DEFAULT(float, TimelineHeight) = 25.0f;
		SLATE_ARGUMENT(SMidiEditorPanelBase::FArguments, ParentArgs);
	SLATE_END_ARGS()

	FText TrackMetaDataName = FText::GetEmpty();
	int32 TrackIndex = INDEX_NONE;
	FLinearColor TrackColor = FLinearColor::White;
	//FSlateBrush* NoteBrush = nullptr;

	TArray<FLinearColor> PianoGridColors;
	
	FLinkedNotesClip* Clip = nullptr;
	float ClipStartOffset = 0.0f;

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UDAWSequencerData* InSequence);

	void OnClipsFocused(TArray< TTuple<FDawSequencerTrack*, FLinkedNotesClip*> > Clips)
	{
		if (!Clips.IsEmpty())
		{
			TrackIndex = Clips[0].Key->MetadataIndex;
			TrackMetaDataName = FText::FromString(SequenceData->GetTrackMetadata(TrackIndex).TrackName);
			Clip = Clips[0].Value;
			TrackColor = SequenceData->GetTrackMetadata(TrackIndex).TrackColor;

			ClipStartOffset = Clip->StartTick;
			//ZoomToContent();

		}
	}

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	float GetStartOffset() const override
	{
		return ClipStartOffset;
	}


	void ZoomToContent()
	{
		const auto& CachedGeometry = GetCachedGeometry();
		const auto& SongsMap = SequenceData->HarmonixMidiFile->GetSongMaps();

		// Calculate width before zoom
		const float ClipDurationMs = SongsMap->TickToMs(Clip->EndTick - Clip->StartTick);
		const float HorizontalZoom = CachedGeometry.Size.X / ClipDurationMs;

		const auto VerticalZoom = CachedGeometry.Size.Y / UnDAW::MidiNoteRange.Size<int8>();

		SetZoom(FVector2D(HorizontalZoom, VerticalZoom));

		// Calculate position after zoom is set
		const float StartPixel = SongsMap->TickToMs(Clip->StartTick) * HorizontalZoom;
		FVector2D NewPosition{ -StartPixel, -(127 - Clip->MaxNote) * RowHeight };
		SetPosition(NewPosition, true);
	}
};

class BKMUSICWIDGETS_API SMidiClipVelocityEditor : public SMidiEditorPanelBase
{
public:
	SLATE_BEGIN_ARGS(SMidiClipVelocityEditor)
		{}
		SLATE_ARGUMENT_DEFAULT(float, TimelineHeight) = 0.0f;
		SLATE_ARGUMENT(SMidiEditorPanelBase::FArguments, ParentArgs);
	SLATE_END_ARGS()


	FText TrackMetaDataName = FText::GetEmpty();
	int32 TrackIndex = INDEX_NONE;
	FLinearColor TrackColor = FLinearColor::White;
	FLinkedNotesClip* Clip = nullptr;
	float ClipStartOffset = 0.0f;

	void OnClipsFocused(TArray< TTuple<FDawSequencerTrack*, FLinkedNotesClip*> > Clips) {
		if (!Clips.IsEmpty())
		{
			TrackIndex = Clips[0].Key->MetadataIndex;
			TrackMetaDataName = FText::FromString(SequenceData->GetTrackMetadata(TrackIndex).TrackName);
			Clip = Clips[0].Value;
			TrackColor = SequenceData->GetTrackMetadata(TrackIndex).TrackColor;
			ClipStartOffset = Clip->StartTick;

			//ZoomToContent();

		}
	};

	float GetStartOffset() const override
	{
		return ClipStartOffset;
	}

	void Construct(const FArguments& InArgs, UDAWSequencerData* InSequence) {
		SMidiEditorPanelBase::Construct(InArgs._ParentArgs, InSequence);
		SequenceData = InSequence;
		TimelineHeight = InArgs._TimelineHeight;
		bLockVerticalPan = true;
	};

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
};

class BKMUSICWIDGETS_API SMidiClipLinkedPanelsContainer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMidiClipLinkedPanelsContainer)
		{}
		SLATE_ARGUMENT_DEFAULT(float, TimelineHeight) = 25.0f;
		SLATE_ARGUMENT_DEFAULT(FVector2D, Position) = FVector2D(0, 0);
		SLATE_ATTRIBUTE(FMusicTimestamp, PlayCursor);
		SLATE_ARGUMENT_DEFAULT(FVector2D, Zoom) = FVector2D(1, 1);
	SLATE_END_ARGS()

	TSharedPtr<SMidiClipEditor> MidiClipEditor;
	TSharedPtr<SMidiClipVelocityEditor> MidiClipVelocityEditor;
	TAttribute<FMusicTimestamp> PlayCursor = FMusicTimestamp();

	FVector2D Position = FVector2D(0, 0);
	FVector2D Zoom = FVector2D(1, 1);

	TAttribute<FVector2D> NoteEditorPosition; // Tracks (X, Y) for note editor
	TAttribute<float> VelocityEditorPositionX; // Only track X for velocity editor

	void OnClipsFocused(TArray< TTuple<FDawSequencerTrack*, FLinkedNotesClip*> > Clips) 
	{
		MidiClipEditor->OnClipsFocused(Clips);
		MidiClipVelocityEditor->OnClipsFocused(Clips);

	}

	void OnInternalPanelMovedByUser(FVector2D NewPosition, bool bIgnoreVerticalPan)
	{
		// Note editor: Update full position (X and Y)
		MidiClipEditor->SetPosition(NewPosition, bIgnoreVerticalPan);

		// Velocity editor: Only update X, preserve its own Y (or ignore)
		MidiClipVelocityEditor->SetPosition(FVector2D(NewPosition.X, MidiClipVelocityEditor->Position.Get().Y), true);
	}

	void OnInternalPanelZoomByUser(FVector2D NewZoom)
	{
		MidiClipEditor->SetZoom(NewZoom);
		MidiClipVelocityEditor->SetZoom(NewZoom);
	}

	void Construct(const FArguments& InArgs, UDAWSequencerData* InSequence)
	{

		Position = InArgs._Position;
		PlayCursor = InArgs._PlayCursor;

		SMidiEditorPanelBase::FArguments BaseArgs;
		//BaseArgs.Position(TAttribute<FVector2D>::Create(TAttribute<FVector2D>::FGetter::CreateLambda([this]() { return Position; })));
		BaseArgs.OnPanelPositionChangedByUser(FOnPanelPositionChangedByUser::CreateSP(this, &SMidiClipLinkedPanelsContainer::OnInternalPanelMovedByUser));
		BaseArgs.OnPanelZoomByUser(FOnPanelZoomChangedByUser::CreateSP(this, &SMidiClipLinkedPanelsContainer::OnInternalPanelZoomByUser));
		BaseArgs.PlayCursor(PlayCursor);
		
		ChildSlot
			[
				SNew(SSplitter)
					.ResizeMode(ESplitterResizeMode::Fill)
					.Orientation(Orient_Vertical)
					+ SSplitter::Slot()
					.Value(0.75f)
					[
						SAssignNew(MidiClipEditor, SMidiClipEditor, InSequence)
							.Clipping(EWidgetClipping::ClipToBounds)
							.ParentArgs(BaseArgs)

					]
					+ SSplitter::Slot()
					.Value(0.25f)
					[
						SAssignNew(MidiClipVelocityEditor, SMidiClipVelocityEditor, InSequence)
							.Clipping(EWidgetClipping::ClipToBounds)
							.ParentArgs(BaseArgs)
					]
			];

	}
};