#include "UndawMusicSequencer.h"
#include "SlateFwd.h"
#include "Components/Widget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"


void SUndawMusicSequencer::Construct(const FArguments& InArgs, UDAWSequencerData* InSequenceToEdit)
{
	SequenceData = InSequenceToEdit;

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

	//auto TimelineCullingRect = MyCullingRect.IntersectionWith(FSlateRect::FromPointAndExtent(TimelineGeometry.LocalToAbsolute(FVector2D(0, 0)), TimelineGeometry.Size));

	auto TrackAreaGeometry = AllottedGeometry.MakeChild(
		FVector2f(0, TimelineHeight),
		FVector2f(AllottedGeometry.Size.X, AllottedGeometry.Size.Y - TimelineHeight)
	);

	
	LayerId = PaintTimeline(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId);
	LayerId++;
	//PaintTimelineMarks(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	LayerId = PaintBackgroundGrid(Args, TrackAreaGeometry, MyCullingRect, OutDrawElements, LayerId);
	LayerId = ScrollBox->Paint(Args, TrackAreaGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	//paint the major tab line on top of everything
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

	
	return LayerId;
}

int32 SUndawMusicSequencer::PaintBackgroundGrid(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
	//just fill the background with a gray box
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
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

	if (bIsPanning)
	{
		HorizontalOffset -= MouseEvent.GetCursorDelta().X;
		ScrollBox->SetScrollOffset(ScrollBox->GetScrollOffset() - MouseEvent.GetCursorDelta().Y);
		HorizontalOffset = FMath::Max(0.0f, HorizontalOffset);
		for (auto& TrackRoot : TrackRoots) { TrackRoot->Lane->HorizontalOffset = HorizontalOffset; }
		//ScrollBox->ScrollTo(FVector2D(HorizontalScrollOffset, 0));
	}
	
	return FReply::Unhandled();
}

void SDawSequencerTrackRoot::Construct(const FArguments& InArgs, UDAWSequencerData* InSequenceToEdit, int32 TrackId)
{
	ChildSlot
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(ControlsArea, SDAwSequencerTrackControlsArea)
				]
				+ SHorizontalBox::Slot()
				[
					SAssignNew(LaneBox, SBox)
						.MinDesiredHeight(150)
						.Content()
						[
							SAssignNew(Lane, SDawSequencerTrackLane, InSequenceToEdit, TrackId)
								.Clipping(EWidgetClipping::ClipToBounds)
								
						]


				]
		];
}

void SDAwSequencerTrackControlsArea::Construct(const FArguments& InArgs)
{
	DesiredWidth = InArgs._DesiredWidth;

	ChildSlot
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SAssignNew(ControlsBox, SBox)

				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SSpacer)
						.Size(FVector2D(5, 1))
				]

		];

}

TOptional<EMouseCursor::Type> SDAwSequencerTrackControlsArea::GetCursor() const
{
	//if hovering over the rightmost 5 pixels, show the resize cursor
	if (bIsHoveringOverResizeArea)
	{
		return EMouseCursor::ResizeLeftRight;
	}
	else
	{
		return EMouseCursor::Default;
	}

}

FReply SDAwSequencerTrackControlsArea::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	//is hovering over the sspacer are? 5 rightmost pixels)

	const bool bIsLeftMouseButtonDown = MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton);

	if (bIsLeftMouseButtonDown)
	{
		if (bIsHoveringOverResizeArea)
		{
			bIsResizing = true;
			return FReply::Handled().CaptureMouse(AsShared());
		}
	}


	
	return FReply::Unhandled();
}

FReply SDAwSequencerTrackControlsArea::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const bool bIsLeftMouseButtonAffecting = MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;

	if (bIsLeftMouseButtonAffecting)
	{
		if (bIsResizing)
		{
			bIsResizing = false;
			return FReply::Handled().ReleaseMouseCapture();
		}
	}
	
	return FReply::Unhandled();
}

FReply SDAwSequencerTrackControlsArea::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	
	////resize the slot
	const FVector2D LocalMousePosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	////UE_LOG(LogTemp, Warning, TEXT("Mouse position %s, target val %f"), *LocalMousePosition.ToString(), MyGeometry.Size.X - 5);



	if (LocalMousePosition.X > MyGeometry.Size.X - 5)
	{
		
		bIsHoveringOverResizeArea = true;

	
	}
	else {
		bIsHoveringOverResizeArea = false;
	}

	if (bIsResizing)
	{
		OnVerticalMajorSlotResized.ExecuteIfBound(LocalMousePosition.X);
	}
	
	return FReply::Unhandled();
}

int32 SDawSequencerTrackSection::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	//print section height cause wtf
	//UE_LOG(LogTemp, Warning, TEXT("Section Height %f"), AllottedGeometry.Size.Y);
	const float SectionDuration = (Clip->EndTick - Clip->StartTick) / 200;
	//just fill the background with a gray box

	const bool bIsHoveredStrong = bIsHovered && GetParentWidget()->IsHovered();
	const FLinearColor ColorToUse = bIsSelected ? TrackColor.Get().CopyWithNewOpacity(0.5f) : bIsHoveredStrong ? TrackColor.Get().CopyWithNewOpacity(0.2f) : TrackColor.Get().CopyWithNewOpacity(0.1f);

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId++,
		AllottedGeometry.ToPaintGeometry(FVector2D(SectionDuration, AllottedGeometry.Size.Y), 1.0f),
		FAppStyle::GetBrush("Sequencer.Section.Background_Contents"),
		ESlateDrawEffect::None,
		ColorToUse
	);



	//draw notes
	const float Height = AllottedGeometry.Size.Y / 127;
	LayerId++;
	for (const auto& Note : Clip->LinkedNotes)
	{
		const float X = (Note.StartTick)/ 200;
		const float Width = (Note.EndTick - Note.StartTick) / 200;
		const float Y = (127 - Note.Pitch) * Height;

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			{ FVector2D(X, Y), FVector2D(X + Width, Y) },
			ESlateDrawEffect::None,
			FLinearColor::White
		);
	}

	// draw start and end ticks it helps with debugging
	FSlateDrawElement::MakeText(
		OutDrawElements,
		LayerId++,
		AllottedGeometry.ToPaintGeometry(),
		FText::FromString(FString::Printf(TEXT("Start: %d, Stop: %d"), Clip->StartTick, Clip->EndTick)),
		FAppStyle::GetFontStyle("NormalFont"),
		ESlateDrawEffect::None,
		FLinearColor::White
	);
	
	return LayerId;
}

int32 SDawSequencerTrackLane::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	
	for (const auto& Section : Sections)
	{
		auto LayoutTransform = FSlateLayoutTransform(FVector2D(0, 0));
		
		const float SectionDuration = (Section->Clip->EndTick - Section->Clip->StartTick) / 200;
		//UE_LOG(LogTemp, Warning, TEXT("Section Duration %f"), SectionDuration);
		auto SectionGeometry = AllottedGeometry.MakeChild(
			FVector2f((Section->Clip->StartTick / 200) - HorizontalOffset, 0),
			FVector2f(SectionDuration, AllottedGeometry.Size.Y)
		);
			


		auto SectionCullingRect = MyCullingRect.IntersectionWith(FSlateRect::FromPointAndExtent(SectionGeometry.LocalToAbsolute(FVector2D(0, 0)), SectionGeometry.Size));
		
		LayerId = Section->OnPaint(Args, SectionGeometry, SectionCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	}
	
	return LayerId;
}
