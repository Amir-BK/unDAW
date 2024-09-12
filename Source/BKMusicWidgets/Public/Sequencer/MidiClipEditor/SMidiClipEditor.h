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


/**
implementations for panning and zooming

*/

namespace UnDAW
{
	const TRange<int> MidiNoteRange{ 0, 127 };
}

DECLARE_DELEGATE_OneParam(
	FOnPanelPositionChangedByUser,
	/** called when the spacer is hovered so we can change its color */
	FVector2D);

// Midi editor panel base class provides basic functionality for drawing a musical editor
// this includes panning, zooming, providing methods to paint grid points and timeline
// and receiving play cursor, to sync two panels together provide a shared position offset attribute
// also drawing the resizable 'major tab' 
class SMidiEditorPanelBase: public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SMidiEditorPanelBase)
		{}
		SLATE_ARGUMENT_DEFAULT(float, TimelineHeight) = 25.0f;
		SLATE_ATTRIBUTE(FVector2D, Position);
		SLATE_EVENT(FOnPanelPositionChangedByUser, OnPanelPositionChangedByUser);
		SLATE_EVENT(FOnPanelPositionChangedByUser, OnPanelZoomByUser);
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UDAWSequencerData* InSequence)
	{
		UE_LOG(LogTemp, Warning, TEXT("Constructing SMidiEditorPanelBase"));
		SequenceData = InSequence;
		TimelineHeight = InArgs._TimelineHeight;
		Position = InArgs._Position;
		OnPanelPositionChangedByUser = InArgs._OnPanelPositionChangedByUser;
		OnPanelZoomByUser = InArgs._OnPanelZoomByUser;

	};

	// called to set the position offset of the panel without firing the delegates, i.e when an external source changes the position
	void SetPosition(FVector2D InPosition)
	{
		//ignore y component if bLockVerticalPan

		if (bLockVerticalPan) InPosition.Y = Position.Get().Y;
		
		Position.Set(InPosition);
	}


	FVector2D PositionOffset = FVector2D(0, 0);
	TAttribute<FVector2D> Position = FVector2D(0, 0);
	TAttribute<FVector2D> Zoom = FVector2D(1, 1);
	FOnPanelPositionChangedByUser OnPanelPositionChangedByUser;
	FOnPanelPositionChangedByUser OnPanelZoomByUser;


	bool bIsPanActive = false;
	bool bLockVerticalPan = false;
	UnDAW::FGridPointMap GridPoints;
	UnDAW::EGridPointType GridPointType = UnDAW::EGridPointType::Bar;
	UnDAW::EMusicTimeLinePaintMode PaintMode = UnDAW::EMusicTimeLinePaintMode::Music;
	UnDAW::ETimeDisplayMode TimeDisplayMode = UnDAW::ETimeDisplayMode::TimeLinear;
	float RowHeight = 10.0f;
	float MajorTabWidth = 150.0f;
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


	FReply OnMousePan(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
	{
		if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
		{
			auto& CurrentPos = Position.Get();
			auto NewPosition = CurrentPos + MouseEvent.GetCursorDelta();

			if (OnPanelPositionChangedByUser.IsBound())
			{
				OnPanelPositionChangedByUser.Execute(NewPosition);
			}
			else {
				SetPosition(NewPosition);
			}


			bIsPanActive = true;
			return FReply::Handled();
		}
		bIsPanActive = false;
		return FReply::Unhandled();
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
		return SequenceData->HarmonixMidiFile->GetSongMaps()->TickToMs(Tick - Position.Get().X) * Zoom.Get().X;
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
		const auto& BarMap = SongsMap->GetBarMap();
		const auto& BeatMap = SongsMap->GetBeatMap();
		//const auto& SubdivisionMap = SongsMap->GetSubdivisionMap();

		const float FirstTickOfFirstBar = SongsMap->GetBarMap().BarBeatTickIncludingCountInToTick(1, 1, 0);
		const float LastTickOfLastBar = SequenceData->HarmonixMidiFile->GetLastEventTick();

		using namespace UnDAW;

		RowHeight = (GetCachedGeometry().Size.Y / 127) * Zoom.Get().Y;

		int32 BarCount = 1;
		float BarTick = 0;

		while (BarTick <= LastTickOfLastBar)
		{
			//VisibleBars.Add(MidiSongMap->CalculateMidiTick(MidiSongMap->GetBarMap().TickToMusicTimestamp(BarTick), TimeSpanToSubDiv(EMusicTimeSpanOffsetUnits::Bars)));
			GridPoints.Add(BarTick, { EGridPointType::Bar, BarCount++, 1, 1 });
			BarTick += SongsMap->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Bar, BarTick);
		}

		//for (const auto& Beat : BeatMap)
		//{
		//	FMusicalGridPoint Point;
		//	Point.Type = EGridPointType::Beat;
		//	Point.Bar = Beat.Value.Bar;
		//	Point.Beat = Beat.Value.Beat;
		//	GridPoints.Add(Beat.Key, Point);
		//}

		//for (const auto& Subdivision : SubdivisionMap)
		//{
		//	FMusicalGridPoint Point;
		//	Point.Type = EGridPointType::Subdivision;
		//	Point.Bar = Subdivision.Value.Bar;
		//	Point.Beat = Subdivision.Value.Beat;
		//	Point.Subdivision = Subdivision.Value.Subdivision;
		//	GridPoints.Add(Subdivision.Key, Point);
		//}
	}

	int32 PaintTimelineMarks(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
	{
		
		return LayerId;
	
	}
	
	int32 PaintTimeline(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
	{
		// first draw the timeline backgroumd a black box
		//UE_LOG(LogTemp, Warning, TEXT("Painting timeline, timeline height: %f"), TimelineHeight);

		const auto& TimeLinePaintGeometry = AllottedGeometry.ToPaintGeometry(
			FVector2f(MajorTabWidth, 0),
			FVector2f(AllottedGeometry.Size.X, TimelineHeight)
		);

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			TimeLinePaintGeometry,
			FAppStyle::GetBrush("Graph.Panel.SolidBackground"),
			ESlateDrawEffect::None,
			FLinearColor::Black
		);

		// draw 30 vertical lines for fun, 
		TRange<float> DrawRange(Position.Get().X, AllottedGeometry.Size.X + Position.Get().X);
		for (int i = 0; i < 30; i++)
		{
			const float X = i * 100;
			if (!DrawRange.Contains(X))
			{
				continue;
			}
			const FVector2D Start(X - Position.Get().X, 0);
			const FVector2D End(X - Position.Get().X, TimelineHeight);


			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				TimeLinePaintGeometry,
				{ Start, End },
				ESlateDrawEffect::None,
				FLinearColor::White
			);
		}

		auto OffsetGeometryChild = AllottedGeometry;
		const auto* MidiSongMap = SequenceData->HarmonixMidiFile->GetSongMaps();
		using namespace UnDAW;
		const float Height = (AllottedGeometry.Size.Y / 127) / Zoom.Get().Y;

		for (const auto& [Tick, GridPoint] : GridPoints)
		{
			FLinearColor LineColor;

			//ugly as heck but probably ok for now
			switch (GridPoint.Type)
			{
			case EGridPointType::Bar:
				//draw bar number
				FSlateDrawElement::MakeText(OutDrawElements,
					LayerId,
					OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, Height), FSlateLayoutTransform(1.0f, FVector2D(TickToPixel(Tick), -Position.Get().Y))),
					FText::FromString(FString::FromInt(GridPoint.Bar)),
					FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 14),
					ESlateDrawEffect::None,
					FLinearColor::White
				);

				//draw beat number
				FSlateDrawElement::MakeText(OutDrawElements,
					LayerId,
					OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, Height), FSlateLayoutTransform(1.0f, FVector2D(TickToPixel(Tick), -Position.Get().Y + 18))),
					FText::FromString(FString::FromInt(GridPoint.Beat)),
					FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7),
					ESlateDrawEffect::None,
					FLinearColor::White
				);

				//draw subdivision number
				FSlateDrawElement::MakeText(OutDrawElements,
					LayerId,
					OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, Height), FSlateLayoutTransform(1.0f, FVector2D(TickToPixel(Tick), -Position.Get().Y + 30))),
					FText::FromString(FString::FromInt(GridPoint.Subdivision)),
					FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7),
					ESlateDrawEffect::None,
					FLinearColor::White
				);


				LineColor = FLinearColor::Gray;
				break;

			case EGridPointType::Subdivision:
				//draw subdivision number
				FSlateDrawElement::MakeText(OutDrawElements,
					LayerId,
					OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, Height), FSlateLayoutTransform(1.0f, FVector2D(TickToPixel(Tick), -Position.Get().Y + 30))),
					FText::FromString(FString::FromInt(GridPoint.Subdivision)),
					FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7),
					ESlateDrawEffect::None,
					FLinearColor::White
				);

				LineColor = FLinearColor::Black;
				break;

			case EGridPointType::Beat:
				//draw beat number
				FSlateDrawElement::MakeText(OutDrawElements,
					LayerId,
					OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, Height), FSlateLayoutTransform(1.0f, FVector2D(TickToPixel(Tick), -Position.Get().Y + 18))),
					FText::FromString(FString::FromInt(GridPoint.Beat)),
					FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7),
					ESlateDrawEffect::None,
					FLinearColor::White
				);

				//draw subdivision number
				FSlateDrawElement::MakeText(OutDrawElements,
					LayerId,
					OffsetGeometryChild.ToPaintGeometry(FVector2D(50.0f, Height), FSlateLayoutTransform(1.0f, FVector2D(TickToPixel(Tick), -Position.Get().Y + 30))),
					FText::FromString(FString::FromInt(GridPoint.Subdivision)),
					FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Regular.ttf"), 7),
					ESlateDrawEffect::None,
					FLinearColor::White
				);

				LineColor = FLinearColor::Blue;
				break;


			}


		}


		return LayerId;
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
	
	FLinkedNotesClip* Clip = nullptr;

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UDAWSequencerData* InSequence);

	void OnClipsFocused(TArray< TTuple<FDawSequencerTrack*, FLinkedNotesClip*> > Clips);

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;



	void ZoomToContent()
	{
		const auto& CachedGeometry = GetCachedGeometry();
		const auto& SongsMap = SequenceData->HarmonixMidiFile->GetSongMaps();

		const float Width = TickToPixel(Clip->EndTick) - TickToPixel(Clip->StartTick);

		const auto& HorizontalZoom = CachedGeometry.Size.X / Width;
		if (OnPanelPositionChangedByUser.IsBound())
		{
			OnPanelPositionChangedByUser.Execute(FVector2D(-TickToPixel(Clip->StartTick), 0));
		}
		else {
			Position.Set(FVector2D(-TickToPixel(Clip->StartTick), 0));
		}


		TRange<int8> ClipNoteRange { Clip->MinNote, Clip->MaxNote };

		const auto& VerticalZoom = CachedGeometry.Size.Y / UnDAW::MidiNoteRange.Size<int8>();

		SetZoom(FVector2D(HorizontalZoom, VerticalZoom));

		//Position.Get().Y = -(127 - Clip->MaxNote) * RowHeight;

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

	void OnClipsFocused(TArray< TTuple<FDawSequencerTrack*, FLinkedNotesClip*> > Clips) {
		if (!Clips.IsEmpty())
		{
			TrackIndex = Clips[0].Key->MetadataIndex;
			TrackMetaDataName = FText::FromString(SequenceData->GetTrackMetadata(TrackIndex).TrackName);
			Clip = Clips[0].Value;
			TrackColor = SequenceData->GetTrackMetadata(TrackIndex).TrackColor;

			//ZoomToContent();

		}
	};

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
		SLATE_ARGUMENT_DEFAULT(FVector2D, Zoom) = FVector2D(1, 1);
	SLATE_END_ARGS()

	TSharedPtr<SMidiClipEditor> MidiClipEditor;
	TSharedPtr<SMidiClipVelocityEditor> MidiClipVelocityEditor;

	FVector2D Position = FVector2D(0, 0);
	FVector2D Zoom = FVector2D(1, 1);

	void OnClipsFocused(TArray< TTuple<FDawSequencerTrack*, FLinkedNotesClip*> > Clips) 
	{
		MidiClipEditor->OnClipsFocused(Clips);
		MidiClipVelocityEditor->OnClipsFocused(Clips);

	}

	void OnInternalPanelMovedByUser(FVector2D NewPosition)
	{
		MidiClipEditor->SetPosition(NewPosition);
		MidiClipVelocityEditor->SetPosition(NewPosition);
	}

	void OnInternalPanelZoomByUser(FVector2D NewZoom)
	{
		MidiClipEditor->SetZoom(NewZoom);
		MidiClipVelocityEditor->SetZoom(NewZoom);
	}

	void Construct(const FArguments& InArgs, UDAWSequencerData* InSequence)
	{

		Position = InArgs._Position;

		SMidiEditorPanelBase::FArguments BaseArgs;
		//BaseArgs.Position(TAttribute<FVector2D>::Create(TAttribute<FVector2D>::FGetter::CreateLambda([this]() { return Position; })));
		BaseArgs.OnPanelPositionChangedByUser(FOnPanelPositionChangedByUser::CreateSP(this, &SMidiClipLinkedPanelsContainer::OnInternalPanelMovedByUser));
		BaseArgs.OnPanelZoomByUser(FOnPanelPositionChangedByUser::CreateSP(this, &SMidiClipLinkedPanelsContainer::OnInternalPanelZoomByUser));
		
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