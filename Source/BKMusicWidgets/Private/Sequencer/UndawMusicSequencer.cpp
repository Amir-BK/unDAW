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
			.BorderBackgroundColor(ColorLambda)
			.Content()
			[
				SNew(STextBlock)
					.Text(FText::FromString(Track.TrackName))
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

	// Calculate the offset geometry for scrolling
	//FGeometry OffsetGeometryChild = AllottedGeometry.MakeChild(AllottedGeometry.GetLocalSize(), FSlateLayoutTransform(1.0f, Position.Get()));

	// Paint the timeline
	LayerId = PaintTimeline(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId);
	LayerId++;

	// Paint the background grid
	LayerId = PaintBackgroundGrid(Args, TrackAreaGeometry, MyCullingRect, OutDrawElements, LayerId);

	// Paint the scroll box
	LayerId = ScrollBox->Paint(Args, TrackAreaGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

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

int32 SUndawMusicSequencer::PaintBackgroundGrid(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	// Create transformed geometry that matches the sections
	FGeometry GridGeometry = AllottedGeometry.MakeChild(
		AllottedGeometry.GetLocalSize(),
		FSlateLayoutTransform(1.0f, Position.Get())
	);

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		GridGeometry.ToPaintGeometry(),
		FAppStyle::GetBrush("Graph.Panel.SolidBackground"),
		ESlateDrawEffect::None,
		FLinearColor::Green.CopyWithNewOpacity(0.2f)
	);

	return LayerId;
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

