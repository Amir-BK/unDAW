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
#include "Framework/Application/SlateApplication.h"
#include "Input/CursorReply.h"
#include "GenericPlatform/ICursor.h"


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
		SLATE_ARGUMENT_DEFAULT(float, CursorFollowAnchorPosition) = 0.5f;
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
		CursorFollowAnchorPosition = InArgs._CursorFollowAnchorPosition;

		// Enable ticking if we have valid sequence data for follow cursor functionality
		if (SequenceData)
		{
			SetCanTick(true);
		}

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
		// Only recalculate grid if position change is significant or we're not actively panning
		// This prevents flickering during continuous pan operations
		if (!bIsPanActive)
		{
			RecalculateGrid();
		}
	}

	// Override Tick to implement follow cursor functionality
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
	{
		SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

		// Only perform follow cursor logic if we have sequence data and follow cursor is enabled
		if (!SequenceData || !bFollowCursor.Get())
		{
			return;
		}

		// Check if transport is playing
		if (SequenceData->PlayState != EBKPlayState::TransportPlaying)
		{
			return;
		}

		auto* MidiSongMap = SequenceData->HarmonixMidiFile->GetSongMaps();
		if (!MidiSongMap)
		{
			return;
		}

		// Calculate current play position in ticks relative to clip start
		const auto PlayCursorTick = MidiSongMap->CalculateMidiTick(PlayCursor.Get(), EMidiClockSubdivisionQuantization::None);
		const auto RelativePlayCursorTick = PlayCursorTick - GetStartOffset();
		
		// Don't follow cursor if we're at or before time 0
		if (RelativePlayCursorTick <= 0.0f)
		{
			return;
		}
		
		// Calculate desired cursor position on screen (as a fraction of viewport width)
		const float ViewportWidth = AllottedGeometry.GetLocalSize().X - MajorTabWidth;
		const float DesiredCursorScreenX = ViewportWidth * CursorFollowAnchorPosition;
		
		// Calculate where the cursor would be with current position
		const float CurrentCursorScreenX = TickToPixel(RelativePlayCursorTick) - MajorTabWidth;
		
		// Calculate how much we need to adjust position to center cursor at desired location
		const float PositionAdjustment = DesiredCursorScreenX - CurrentCursorScreenX;
		
		// Calculate what the new position would be
		FVector2D NewPosition = Position.Get();
		NewPosition.X += PositionAdjustment;
		
		// Check if this new position would cause negative time to be displayed
		// Calculate what tick would be at the left edge with the new position
		const float LeftEdgeMs = (-NewPosition.X - MajorTabWidth) / Zoom.Get().X;
		const float LeftEdgeTick = MidiSongMap->MsToTick(LeftEdgeMs) + GetStartOffset();
		
		// Only apply follow cursor if the left edge would still be at or after the start offset (time 0)
		if (LeftEdgeTick >= GetStartOffset())
		{
			// Update position (this will notify linked panels if delegates are bound)
			if (OnPanelPositionChangedByUser.IsBound())
			{
				OnPanelPositionChangedByUser.Execute(NewPosition, bLockVerticalPan);
			}
			else
			{
				SetPosition(NewPosition, bLockVerticalPan);
			}
		}
		// If we would show negative time, don't follow the cursor - just let it move freely
	}

	FVector2D PositionOffset = FVector2D(0, 0);
	TAttribute<FVector2D> Position = FVector2D(0, 0);
	TAttribute<FMusicTimestamp> PlayCursor = FMusicTimestamp();
	TAttribute<FVector2D> Zoom = FVector2D(1, 1);
	TAttribute<bool> bFollowCursor = true;
	float CursorFollowAnchorPosition = 0.5f;
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

	virtual TOptional<EMouseCursor::Type> GetCursor() const override
	{
		if (bIsPanActive)
		{
			return EMouseCursor::GrabHand;
		}

		return SCompoundWidget::GetCursor();
	}

	FReply OnMousePan(const FGeometry & MyGeometry, const FPointerEvent & MouseEvent)
	{
		// Debug: Log mouse pan details when zoomed out to identify erratic behavior
		const bool bIsZoomedOut = Zoom.Get().X < 0.5f;
		if (bIsZoomedOut)
		{
			UE_LOG(LogTemp, Warning, TEXT("OnMousePan Debug: RMB=%s, CursorDelta=%s, Position=%s, bIsPanActive=%s"),
				MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton) ? TEXT("Down") : TEXT("Up"),
				*MouseEvent.GetCursorDelta().ToString(),
				*Position.Get().ToString(),
				bIsPanActive ? TEXT("true") : TEXT("false"));
		}

		if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
		{
			const auto& CurrentPos = Position.Get();
			
			// Cache ContentWidth to prevent fluctuation between frames during panning
			static float CachedContentWidth = 0.0f;
			static float LastZoomX = 0.0f;
			
			// Only recalculate ContentWidth if zoom has changed significantly
			const float CurrentZoomX = Zoom.Get().X;
			if (FMath::Abs(CurrentZoomX - LastZoomX) > 0.001f || CachedContentWidth == 0.0f)
			{
				CachedContentWidth = TickToPixel(SequenceData->HarmonixMidiFile->GetLastEventTick());
				LastZoomX = CurrentZoomX;
			}
			
			const float ContentWidth = CachedContentWidth;

			// Only calculate Y delta if vertical pan is NOT locked
			const float ContentHeight = bLockVerticalPan ? 0 : 127 * RowHeight;
			const float DeltaY = bLockVerticalPan ? 0 : MouseEvent.GetCursorDelta().Y;

			// Fix the broken clamping logic - when viewport is larger than content, don't create impossible constraints
			float MinPositionX, MaxPositionX;
			if (MyGeometry.Size.X >= ContentWidth)
			{
				// When viewport is larger than content, allow full range positioning
				// to center content or position it anywhere within the viewport
				MinPositionX = -(ContentWidth);
				MaxPositionX = MyGeometry.Size.X;
			}
			else
			{
				// When content is larger than viewport, use normal clamping
				MinPositionX = -ContentWidth + MyGeometry.Size.X;
				MaxPositionX = 0.0f;
			}

			const auto NewPositionX = FMath::Clamp(
				CurrentPos.X + MouseEvent.GetCursorDelta().X,
				MinPositionX,
				MaxPositionX
			);

			const auto NewPositionY = bLockVerticalPan ?
				CurrentPos.Y : // Preserve existing Y
				FMath::Clamp(
					CurrentPos.Y + DeltaY,
					-ContentHeight + MyGeometry.Size.Y,
					0.0f
				);

			const FVector2D NewPosition = FVector2D(NewPositionX, NewPositionY);

			// Debug: Log the position change calculation when zoomed out
			if (bIsZoomedOut)
			{
				UE_LOG(LogTemp, Warning, TEXT("  Position Change: Current=%s -> New=%s (Delta=%s)"),
					*CurrentPos.ToString(), *NewPosition.ToString(), *MouseEvent.GetCursorDelta().ToString());
				UE_LOG(LogTemp, Warning, TEXT("  Geometry Size=%s, ContentWidth=%.1f, MinX=%.1f, MaxX=%.1f"),
					*MyGeometry.Size.ToString(), ContentWidth, MinPositionX, MaxPositionX);
			}

			if (OnPanelPositionChangedByUser.IsBound())
			{
				// Pass bLockVerticalPan to enforce Y ignore in linked panels
				OnPanelPositionChangedByUser.Execute(NewPosition, bLockVerticalPan);
			}
			else
			{
				SetPosition(NewPosition);
			}

			// Only set pan active if we actually have a valid delta to avoid false positives
			if (!MouseEvent.GetCursorDelta().IsNearlyZero(0.1f))
			{
				bIsPanActive = true;
			}
			return FReply::Handled().CaptureMouse(AsShared());
		}
		
		// Don't immediately set bIsPanActive to false - let mouse button up handle it
		// This prevents erratic state changes during mouse move events
		return FReply::Unhandled();
	}


	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		 if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton && bIsPanActive)
		{
			bIsPanActive = false;
			// Recalculate grid after panning is complete to ensure correct state
			RecalculateGrid();
			return FReply::Handled().ReleaseMouseCapture();
		}
		return SCompoundWidget::OnMouseButtonUp(MyGeometry, MouseEvent);
	}

	const FVector2D GetLocalMousePosition(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
		return MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()) - Position.Get();
	}

	void SetZoom(const FVector2D NewZoom)
	{
		const FVector2D OldZoom = Zoom.Get();
		Zoom.Set(NewZoom);
		
		// Debug: Log zoom changes to track potential issues
		if (!OldZoom.Equals(NewZoom, 0.01f))
		{
			UE_LOG(LogTemp, VeryVerbose, TEXT("SetZoom: %s -> %s"), *OldZoom.ToString(), *NewZoom.ToString());
			
			// Recalculate grid after zoom change
			RecalculateGrid();
		}
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

	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		return OnMousePan(MyGeometry, MouseEvent);
	}

	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
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
		// Use smart buffer calculation - larger buffer when far from edge, smaller when near edge
		// This prevents bars from being truncated while not being overly generous near boundaries
		const float SmartBuffer = FMath::Min(200.0f, FMath::Max(50.0f, (GeometrySize.X - MajorTabWidth) * 0.1f));
		const float VisibleEndTick = SongsMap->MsToTick((GeometrySize.X - Position.Get().X - MajorTabWidth + SmartBuffer) / Zoom.Get().X);

		// Log essential grid calculation info only in verbose mode
		UE_LOG(LogTemp, VeryVerbose, TEXT("RecalculateGrid: GeometrySize=%s, Zoom=%s, GridPoints=%d, SmartBuffer=%.1f"), 
			*GeometrySize.ToString(), *Zoom.Get().ToString(), GridPoints.Num(), SmartBuffer);

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

		// Log density changes only when they occur
		if (PreviousDensity != OptimalDensity)
		{
			UE_LOG(LogTemp, Log, TEXT("Grid density changed: %d -> %d (PixelsPerBar=%.1f)"), 
				(int32)PreviousDensity, (int32)OptimalDensity, (double)(PixelsPerTick * TicksPerBar));
			PreviousDensity = OptimalDensity;
		}
		
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

		int32 BarsAdded = 0;
		int32 BarsSkipped = 0;

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

			// Calculate pixel position for this bar for debugging
			const float BarPixel = TickToPixel(BarTick - GetStartOffset());

			// Only add the grid point if it should be shown at current density
			const bool bShouldShow = UnDAW::FTimelineGridDensityCalculator::ShouldShowGridPoint(BarGridPoint, OptimalDensity, DisplayBarNumber);

			if (bShouldShow)
			{
				GridPoints.Add(BarTick, BarGridPoint);
				BarsAdded++;
			}
			else
			{
				BarsSkipped++;
			}

			// Calculate next bar tick
			const float NextBarTickIncrement = SongsMap->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Bar, BarTick);
			const float NextBarTick = BarTick + NextBarTickIncrement;
			
			// Check if we're about to exit the loop
			if (NextBarTick > VisibleEndTick || DisplayBarNumber >= 200)
			{
				break;
			}

			// Move to next bar
			BarTick = NextBarTick;
			DisplayBarNumber++;
		}

		// Log final summary only in verbose mode
		UE_LOG(LogTemp, VeryVerbose, TEXT("Grid calculation completed: %d bars added, %d skipped"), BarsAdded, BarsSkipped);

		// Store current density for use in painting
		CurrentGridDensity = OptimalDensity;

		// Update row height for vertical scaling
		const float LastRowHeight = RowHeight;
		RowHeight = 10.0f * Zoom.Get().Y;
	}

	int32 PaintTimeline(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;

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
	
	// Store previous density per-instance to detect changes (not static to avoid cross-panel interference)
	UnDAW::TimelineConstants::EGridDensity PreviousDensity = UnDAW::TimelineConstants::EGridDensity::Bars;

	// Helper method to add beats and subdivisions for a bar
	void AddBeatsAndSubdivisions(float BarStartTick, int32 BarNumber, UnDAW::TimelineConstants::EGridDensity Density, const FSongMaps* SongsMap, float VisibleEndTick);

	// Helper method to add subdivisions for a beat
	void AddSubdivisions(float BeatStartTick, int32 BarNumber, int32 BeatNumber, const FSongMaps* SongsMap, float VisibleEndTick);
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

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual float GetStartOffset() const override
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

	virtual float GetStartOffset() const override
	{
		return ClipStartOffset;
	}

	 void Construct(const FArguments& InArgs, UDAWSequencerData* InSequence) {
		SMidiEditorPanelBase::Construct(InArgs._ParentArgs, InSequence);
		SequenceData = InSequence;
		TimelineHeight = InArgs._TimelineHeight;
		bLockVerticalPan = true;
	};

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
};

class BKMUSICWIDGETS_API SMidiClipLinkedPanelsContainer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMidiClipLinkedPanelsContainer)
		{}
		SLATE_ARGUMENT_DEFAULT(float, TimelineHeight) = 25.0f;
		SLATE_ARGUMENT_DEFAULT(FVector2D, Position) = FVector2D(0, 0);
		SLATE_ATTRIBUTE(FMusicTimestamp, PlayCursor);
		SLATE_ATTRIBUTE(bool, bFollowCursor);
		SLATE_ARGUMENT_DEFAULT(float, CursorFollowAnchorPosition) = 0.5f;
		SLATE_ARGUMENT_DEFAULT(FVector2D, Zoom) = FVector2D(1, 1);
	SLATE_END_ARGS()

	TSharedPtr<SMidiClipEditor> MidiClipEditor;
	TSharedPtr<SMidiClipVelocityEditor> MidiClipVelocityEditor;
	TAttribute<FMusicTimestamp> PlayCursor = FMusicTimestamp();
	TAttribute<bool> bFollowCursor = true;
	float CursorFollowAnchorPosition = 0.5f;

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
		bFollowCursor = InArgs._bFollowCursor;
		CursorFollowAnchorPosition = InArgs._CursorFollowAnchorPosition;

		SMidiEditorPanelBase::FArguments BaseArgs;
		//BaseArgs.Position(TAttribute<FVector2D>::Create(TAttribute<FVector2D>::FGetter::CreateLambda([this]() { return Position; })));
		BaseArgs.OnPanelPositionChangedByUser(FOnPanelPositionChangedByUser::CreateSP(this, &SMidiClipLinkedPanelsContainer::OnInternalPanelMovedByUser));
		BaseArgs.OnPanelZoomByUser(FOnPanelZoomChangedByUser::CreateSP(this, &SMidiClipLinkedPanelsContainer::OnInternalPanelZoomByUser));
		BaseArgs.PlayCursor(PlayCursor);
		BaseArgs.bFollowCursor(bFollowCursor);
		BaseArgs.CursorFollowAnchorPosition(CursorFollowAnchorPosition);
		
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
							.ParentArgs(BaseArgs)
				]
			];

	}
};