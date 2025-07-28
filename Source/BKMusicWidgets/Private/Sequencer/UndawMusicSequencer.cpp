#include "Sequencer/UndawMusicSequencer.h"
#include "SlateFwd.h"
#include "Components/Widget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"


void SUndawMusicSequencer::Construct(const FArguments& InArgs, UDAWSequencerData* InSequenceToEdit)
{
	SMidiEditorPanelBase::Construct(InArgs._ParentArgs, InSequenceToEdit);
	SequenceData = InSequenceToEdit;
	MajorTabWidth = 150.0f;
	const auto& SequenceName = SequenceData->GetFName();

	TimelineHeight = InArgs._TimelineHeight;

	ChildSlot[
		SAssignNew(ScrollBox, SScrollBox)
			.ConsumeMouseWheel(EConsumeMouseWheel::Never)
			+SScrollBox::Slot()
			.AutoSize()
			[
				SAssignNew(Splitter, SSplitter)
				
					.Orientation(Orient_Vertical)
					.HitDetectionSplitterHandleSize(5.0f)
					.ResizeMode(ESplitterResizeMode::FixedSize)
			]
	];

	//CreateGridPanel();
	PopulateSequencerFromDawData();
}


void SUndawMusicSequencer::PopulateSequencerFromDawData()
{

	int i = 0;
	for (auto& Track : SequenceData->M2TrackMetadata)
	{
		auto ColorLambda = TAttribute<FSlateColor>::CreateLambda([this, i]() { return SequenceData->GetTrackMetadata(i).TrackColor; });

		TSharedPtr<SDawSequencerTrackRoot> TrackRoot;
		Splitter->AddSlot(i)
			[
				SAssignNew(TrackRoot, SDawSequencerTrackRoot, SequenceData, i)
					.Zoom_Lambda([this]() { return Zoom.Get(); })
					.Position_Lambda([this]() { return Position.Get(); })


			];

		TrackRoot->ControlsArea->ControlsBox->SetContent(
			SNew(SBorder)
			.Content()
			[
				SNew(STextBlock)
					.Text(FText::FromString(Track.TrackName))
					.ColorAndOpacity(ColorLambda)
			]
		);


		TrackRoot->ControlsArea->ControlsBox->SetWidthOverride(MajorTabWidth);

		TrackRoot->Lane->OnSectionSelected.BindSP(this, &SUndawMusicSequencer::OnSectionSelected);

		TrackRoot->ControlsArea->OnVerticalMajorSlotResized.BindLambda([this](float InNewSize) { 
			MajorTabWidth = InNewSize;
			for (auto& TrackRoot : TrackRoots) { TrackRoot->ResizeSplitter(InNewSize); }});

		TrackRoot->ControlsArea->OnVerticalMajorSlotHover.BindLambda([this](bool bIsHovering) {
			MajorTabAlpha = bIsHovering ? 0.0f : 0.25f;
			});

		TrackRoots.Add(TrackRoot);
		//TrackSlotsScrollBox.Add(TrackSlot);
		i++;
	}

	//oversize box?
	Splitter->AddSlot()
		[
			SNullWidget::NullWidget
		];


}

int32 SUndawMusicSequencer::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{

	// Calculate the geometry for the track area
	FGeometry TrackAreaGeometry = AllottedGeometry.MakeChild(
		FVector2f(0, TimelineHeight),
		FVector2f(AllottedGeometry.Size.X, AllottedGeometry.Size.Y - TimelineHeight)
	);

	// Paint the background grid
	LayerId = PaintBackgroundFill(Args, TrackAreaGeometry, MyCullingRect, OutDrawElements, LayerId);

	// Paint the scroll box
	LayerId = ScrollBox->Paint(Args, TrackAreaGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	LayerId = PaintTimeline(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId++);

	// Paint the major tab line on top of everything
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		TrackAreaGeometry.ToPaintGeometry(),
		{ FVector2D(MajorTabWidth + 2, 0), FVector2D(MajorTabWidth + 2, AllottedGeometry.Size.Y) },
		ESlateDrawEffect::None,
		FLinearColor::Black.CopyWithNewOpacity(0.2f),
		false,
		5.0f
	);

	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		TrackAreaGeometry.ToPaintGeometry(),
		{ FVector2D(MajorTabWidth + 2, 0), FVector2D(MajorTabWidth + 2, AllottedGeometry.Size.Y) },
		ESlateDrawEffect::None,
		FLinearColor::Gray.CopyWithNewOpacity(MajorTabAlpha),
		false,
		5.0f
	);

	// Paint the play cursor
	LayerId = PaintPlayCursor(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId);

	return LayerId;
}

int32 SUndawMusicSequencer::PaintBackgroundFill(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	// Create geometry that excludes the controls area (starts at MajorTabWidth)
	FGeometry GridGeometry = AllottedGeometry.MakeChild(
		FVector2f(AllottedGeometry.GetLocalSize().X - MajorTabWidth, AllottedGeometry.GetLocalSize().Y),
		FSlateLayoutTransform(1.0f, FVector2f(MajorTabWidth, Position.Get().Y))
	);

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		GridGeometry.ToPaintGeometry(),
		FAppStyle::GetBrush("Graph.Panel.SolidBackground"),
		ESlateDrawEffect::None,
		FLinearColor::Green.CopyWithNewOpacity(0.2f)
	);

	return LayerId + 1;
}

FReply SUndawMusicSequencer::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	
	const bool bIsRightButtonDown = MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton);

	if (bIsRightButtonDown)
	{
		bIsPanActive = true;
		return FReply::Handled().CaptureMouse(AsShared());
	}
	
	return FReply::Unhandled();
}

FReply SUndawMusicSequencer::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const bool bIsRightButtonAffecting = MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;

	if (bIsRightButtonAffecting)
	{
		if (bIsPanActive)
		{
			bIsPanActive = false;
			return FReply::Handled().ReleaseMouseCapture();
		}
	}
	
	return FReply::Unhandled();
}

FReply SUndawMusicSequencer::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	TRange<float> MajorTabRange(MajorTabWidth + 2, MajorTabWidth + 7);
	const FVector2D LocalMousePosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	if (MajorTabRange.Contains(LocalMousePosition.X))
	{
		MajorTabAlpha = 0.25f;
	}
	else
	{
		MajorTabAlpha = 0.0f;
	}

	if (bIsPanActive)
	{
		//const float HorizontalOffset -= MouseEvent.GetCursorDelta().X;
		ScrollBox->SetScrollOffset(ScrollBox->GetScrollOffset() - MouseEvent.GetCursorDelta().Y);
		Position.Set(FVector2D{ FMath::Min(0.0f, Position.Get().X + MouseEvent.GetCursorDelta().X), 0.0f });

		//invalidate children
		for (auto& TrackRoot : TrackRoots) { TrackRoot->Invalidate(EInvalidateWidgetReason::Layout); };

		//for (auto& TrackRoot : TrackRoots) { TrackRoot->Lane->HorizontalOffset = HorizontalOffset; }
		//ScrollBox->ScrollTo(FVector2D(HorizontalScrollOffset, 0));
	}
	
	return FReply::Unhandled();
}

void SUndawMusicSequencer::AbsoluteCursorPositionToTime(const FVector2D& MousePosition, float& OutTick, FMusicTimestamp& OutTimestamp)
{
	
	const auto LocalPositionAtGeometry = GetCachedGeometry().AbsoluteToLocal(MousePosition);

	

	OutTick = PixelToTick(LocalPositionAtGeometry.X - PositionOffset.X);

	OutTimestamp = SequenceData->HarmonixMidiFile->GetSongMaps()->TickToMusicTimestamp(OutTick);




}

TTuple<int, ETrackType> SUndawMusicSequencer::GetHoveredTrackAndType(const FVector2D& MousePosition) const
{
	const auto LocalPositionAtGeometry = GetCachedGeometry().AbsoluteToLocal(MousePosition);

	for (int32 i = 0; i < TrackRoots.Num(); ++i)
	{
		const auto& TrackRoot = TrackRoots[i];
		if (TrackRoot->GetCachedGeometry().IsUnderLocation(MousePosition))
		{
			return MakeTuple(TrackRoot->Lane->TrackId, TrackRoot->Lane->TrackType);
		}
	}
	return MakeTuple(INDEX_NONE, ETrackType::None);

}

int32 SUndawMusicSequencer::PaintPlayCursor(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	const auto& PlayCursorTick = SequenceData->HarmonixMidiFile->GetSongMaps()->CalculateMidiTick(PlayCursor.Get(), EMidiClockSubdivisionQuantization::None);
	const float CursorPixel = TickToPixel(PlayCursorTick);

	// Only draw cursor if it's to the right of the controls area
	if (CursorPixel > MajorTabWidth)
	{
		// Create geometry that excludes the controls area
		FGeometry TrackAreaGeometry = AllottedGeometry.MakeChild(
			FVector2f(0, TimelineHeight),
			FVector2f(AllottedGeometry.Size.X, AllottedGeometry.Size.Y - TimelineHeight)
		);

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			TrackAreaGeometry.ToPaintGeometry(),
			{ FVector2D(CursorPixel, 0), FVector2D(CursorPixel, TrackAreaGeometry.GetLocalSize().Y) },
			ESlateDrawEffect::None,
			FLinearColor::Red,
			false,
			2.0f
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
	}

	return LayerId;
}

