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
	}


	FVector2D PositionOffset = FVector2D(0, 0);
	TAttribute<FVector2D> Position = FVector2D(0, 0);
	TAttribute<FMusicTimestamp> PlayCursor = FMusicTimestamp();
	TAttribute<FVector2D> Zoom = FVector2D(1, 1);
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

	void RecalculateGrid()
	{
		GridPoints.Empty();
		const auto& SongsMap = SequenceData->HarmonixMidiFile->GetSongMaps();

		// Calculate visible range based on current position and zoom
		// Since we're now adding Position.X, we need to adjust the calculation
		const float VisibleStartTick = SongsMap->MsToTick(-Position.Get().X / Zoom.Get().X);  // Start of visible area
		const float VisibleEndTick = SongsMap->MsToTick((GetCachedGeometry().Size.X - Position.Get().X) / Zoom.Get().X);  // End of visible area

		// Find the first bar that starts before or at VisibleStartTick
		const auto StartBarInfo = SongsMap->GetBarMap().TickToMusicTimestamp(VisibleStartTick);
		float BarTick = SongsMap->GetBarMap().BarBeatTickIncludingCountInToTick(
			StartBarInfo.Bar,
			1,
			0
		);

		// Add grid points for all bars in the visible range
		while (BarTick <= VisibleEndTick)
		{
			const auto BarInfo = SongsMap->GetBarMap().TickToMusicTimestamp(BarTick);
			GridPoints.Add(BarTick, {
				UnDAW::EGridPointType::Bar,
				BarInfo.Bar,
				1,
				0
				});

			// Move to next bar
			BarTick += SongsMap->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Bar, BarTick);
		}

		// Update row height for vertical scaling
		RowHeight = 10.0f * Zoom.Get().Y;
	}

	int32 PaintTimelineMarks(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
	{

		return LayerId;

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
					FVector2D(CursorPixel - PlayCursorHalfWidth, 0),  // Position directly
					FVector2D(PlayCursorWidth, TimelineHeight)
				),
				FAppStyle::GetBrush("Graph.Panel.SolidBackground"),
				ESlateDrawEffect::None,
				FLinearColor(255.f, 0.1f, 0.2f, 1.f)
			);
		}

		return LayerId;
	}

	int32 PaintTimeline(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
	{
		// Draw timeline background
		const bool bShouldPaintTimelineBar = TimelineHeight > 0.0f;

		if (bShouldPaintTimelineBar)
		{
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(
					FVector2D(MajorTabWidth, 0),
					FVector2D(AllottedGeometry.Size.X - MajorTabWidth, TimelineHeight)
				),
				FAppStyle::GetBrush("Graph.Panel.SolidBackground"),
				ESlateDrawEffect::None,
				FLinearColor::Black
			);
		}

		// Setup drawing parameters
		const auto* MidiSongMap = SequenceData->HarmonixMidiFile->GetSongMaps();
		const float Height = (AllottedGeometry.Size.Y / 127) / Zoom.Get().Y;
		const FLinearColor BarLineColor = FLinearColor::Gray.CopyWithNewOpacity(0.1f);
		const FLinearColor BarTextColor = FLinearColor::Gray.CopyWithNewOpacity(0.5f);
		const FSlateFontInfo LargeFont(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 14);
		const FSlateFontInfo SmallFont(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7);

		// Draw grid points
		for (const auto& [OriginTick, GridPoint] : GridPoints)
		{
			const float Tick = OriginTick - GetStartOffset();
			const float PixelPosition = TickToPixel(Tick);

			switch (GridPoint.Type)
			{
			case UnDAW::EGridPointType::Bar:
				// Draw bar numbers and info in timeline
				if (bShouldPaintTimelineBar)
				{
					// Bar number
					FSlateDrawElement::MakeText(
						OutDrawElements,
						LayerId,
						AllottedGeometry.ToPaintGeometry(
							FVector2D(PixelPosition, 0),
							FVector2D(50.0f, Height)
						),
						FText::FromString(FString::FromInt(GridPoint.Bar)),
						LargeFont,
						ESlateDrawEffect::None,
						BarTextColor
					);

					// Beat number
					FSlateDrawElement::MakeText(
						OutDrawElements,
						LayerId,
						AllottedGeometry.ToPaintGeometry(
							FVector2D(PixelPosition, 18),
							FVector2D(50.0f, Height)
						),
						FText::FromString(FString::FromInt(GridPoint.Beat)),
						SmallFont,
						ESlateDrawEffect::None,
						FLinearColor::White
					);

					// Subdivision number
					FSlateDrawElement::MakeText(
						OutDrawElements,
						LayerId,
						AllottedGeometry.ToPaintGeometry(
							FVector2D(PixelPosition, 30),
							FVector2D(50.0f, Height)
						),
						FText::FromString(FString::FromInt(GridPoint.Subdivision)),
						SmallFont,
						ESlateDrawEffect::None,
						FLinearColor::White
					);
				}

				// Draw alternating bar backgrounds
				if (GridPoint.Bar % 2 == 0)
				{
					const float BarDuration = MidiSongMap->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Bar, OriginTick);
					const float BarEndPixel = TickToPixel(Tick + BarDuration);
					const float BarWidth = BarEndPixel - PixelPosition;

					FSlateDrawElement::MakeBox(
						OutDrawElements,
						LayerId - 2,
						AllottedGeometry.ToPaintGeometry(
							FVector2D(PixelPosition, 0),
							FVector2D(BarWidth, AllottedGeometry.GetLocalSize().Y)
						),
						FAppStyle::GetBrush("Graph.Panel.SolidBackground"),
						ESlateDrawEffect::None,
						FLinearColor::Gray.CopyWithNewOpacity(0.4f)
					);
				}

				// Draw bar line
				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId - 2,
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
				break;

			case UnDAW::EGridPointType::Beat:
				// Draw beat line
				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId - 2,
					AllottedGeometry.ToPaintGeometry(),
					{
						FVector2D(PixelPosition, 0),
						FVector2D(PixelPosition, AllottedGeometry.GetLocalSize().Y)
					},
					ESlateDrawEffect::None,
					FLinearColor::Blue.CopyWithNewOpacity(0.1f),
					false,
					1.0f
				);
				break;

			case UnDAW::EGridPointType::Subdivision:
				// Draw subdivision line
				FSlateDrawElement::MakeLines(
					OutDrawElements,
					LayerId - 2,
					AllottedGeometry.ToPaintGeometry(),
					{
						FVector2D(PixelPosition, 0),
						FVector2D(PixelPosition, AllottedGeometry.GetLocalSize().Y)
					},
					ESlateDrawEffect::None,
					FLinearColor::Black.CopyWithNewOpacity(0.05f),
					false,
					1.0f
				);
				break;
			}
		}

		return LayerId;
	};
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
		MidiClipEditor->SetPosition(NewPosition, false);

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