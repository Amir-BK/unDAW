#include "Sequencer/UndawSequencerTrack.h"
#include "SlateFwd.h"
#include "Components/Widget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"




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


void SDawSequencerTrackRoot::Construct(const FArguments& InArgs, UDAWSequencerData* InSequenceToEdit, int32 TrackId)
{
	//print constructing "SequencerTrackRoot with track ID"
	UE_LOG(LogTemp, Log, TEXT("Coinstructing Sequencer Track root with ID %d"), TrackId)
	
	
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
								.Zoom(InArgs._Zoom)
								.Position(InArgs._Position)

						]


				]
		];
}

void SDawSequencerTrackRoot::ResizeSplitter(float InNewSize) {
	ControlsArea->ResizeControlsBox(InNewSize);
}



void SDawSequencerTrackLane::Construct(const FArguments& InArgs, UDAWSequencerData* InSequenceToEdit, int32 InTrackId)
{
	
	UE_LOG(LogTemp, Log, TEXT("Constructing track lane with Id %d"), InTrackId)

	SequenceData = InSequenceToEdit;
	TrackId = InTrackId;
	Position = InArgs._Position;
	Zoom = InArgs._Zoom;
	TrackType = InArgs._LaneTrackType;

	//PopulateSections();

	for (auto& Clip : SequenceData->Tracks[TrackId].LinkedNotesClips)
	{
		TSharedPtr<SDawSequencerTrackMidiSection> Section;

		//TotalTrackWidth = FMath::Max(TotalTrackWidth, Clip.EndTick * Zoom.Get().X);

		SAssignNew(Section, SDawSequencerTrackMidiSection, &Clip, &SequenceData->Tracks[TrackId], SequenceData)
			.TrackColor(TAttribute<FLinearColor>::CreateLambda([this]() {return SequenceData->GetTrackMetadata(TrackId).TrackColor; }))
			.Position(InArgs._Position)
			.Zoom(InArgs._Zoom);

		Section->AssignParentWidget(SharedThis(this));
		Sections.Add(Section);

		
	}

}

TOptional<EMouseCursor::Type> SDawSequencerTrackLane::GetCursor() const {
	if (bIsHoveringOverSectionResizeArea)
	{
		return EMouseCursor::ResizeLeftRight;
	}
	else if (bIsHoveringOverSectionDragArea)
	{
		if (Sections[HoveringOverSectionIndex]->bIsSelected)
			return EMouseCursor::CardinalCross;
		else
			return EMouseCursor::Crosshairs;
	}
	else {
		return TOptional<EMouseCursor::Type>();
	}

}

int32 SDawSequencerTrackLane::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// Let Slate handle clipping via MyCullingRect
	for (int32 i = 0; i < Sections.Num(); i++)
	{
		const auto& Section = Sections[i];
		const float SectionStartTick = Section->Clip->StartTick;
		const float SectionEndTick = Section->Clip->EndTick;
		const float SectionStartPixel = TickToPixel(SectionStartTick);
		const float SectionEndPixel = TickToPixel(SectionEndTick);
		const float SectionWidth = SectionEndPixel - SectionStartPixel;

		// Create section geometry with proper size and offset
		FGeometry SectionGeometry = AllottedGeometry.MakeChild(
			FVector2D(SectionWidth, AllottedGeometry.GetLocalSize().Y),
			FSlateLayoutTransform(1.0f, FVector2D(SectionStartPixel + Position.Get().X, Position.Get().Y))
		);

		LayerId = Section->Paint(Args, SectionGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	}

	return LayerId;
}

inline FReply SDawSequencerTrackLane::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {

	// Get mouse position relative to the actual track area (accounting for position offset)
	const FVector2D LocalMousePosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	
	// Convert to tick space using the sequence data directly
	const float MouseMs = (LocalMousePosition.X - Position.Get().X) / Zoom.Get().X;
	const float MouseTick = SequenceData->HarmonixMidiFile->GetSongMaps()->MsToTick(MouseMs);

	constexpr int32 SectionResizeAreaWidth = 5;

	bool bAnySectionHovered = false;
	for (int32 i = 0; i < Sections.Num(); i++)
	{
		const auto& Section = Sections[i];
		const int32 SectionStartTick = Section->Clip->StartTick;
		const int32 SectionEndTick = Section->Clip->EndTick;
		
		// Check if mouse is within this section's tick range
		if (MouseTick >= SectionStartTick && MouseTick <= SectionEndTick)
		{
			HoveringOverSectionIndex = i;
			Section->bIsHovered = true;
			bAnySectionHovered = true;
			
			// Check if we're in the resize area (5 pixels from edges)
			const float SectionStartPixel = TickToPixel(SectionStartTick);
			const float SectionEndPixel = TickToPixel(SectionEndTick);
			const bool bNearStart = FMath::Abs(LocalMousePosition.X - SectionStartPixel) < SectionResizeAreaWidth;
			const bool bNearEnd = FMath::Abs(LocalMousePosition.X - SectionEndPixel) < SectionResizeAreaWidth;
			
			if (bNearStart || bNearEnd)
			{
				bIsHoveringOverSectionResizeArea = true;
				bIsHoveringOverSectionDragArea = false;
			}
			else
			{
				bIsHoveringOverSectionDragArea = true;
				bIsHoveringOverSectionResizeArea = false;
			}
		}
		else {
			Section->bIsHovered = false;
		}
	}

	if (!bAnySectionHovered)
	{
		HoveringOverSectionIndex = INDEX_NONE;
		bIsHoveringOverSectionDragArea = false;
		bIsHoveringOverSectionResizeArea = false;
	}

	return FReply::Unhandled();
}

inline FReply SDawSequencerTrackLane::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {

	const bool bIsLeftMouseButtonEffecting = MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;

	if (bIsLeftMouseButtonEffecting && HoveringOverSectionIndex != INDEX_NONE && bIsHoveringOverSectionDragArea)
	{
		OnSectionSelected.ExecuteIfBound(Sections[HoveringOverSectionIndex]);
		SelectedSectionIndex = HoveringOverSectionIndex;
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

float SDawSequencerTrackLane::TickToPixel(const float Tick) const
{
	const auto& SongsMap = SequenceData->GetSongMaps();

	const float Milisecond = SongsMap.TickToMs(Tick);

	return Milisecond * Zoom.Get().X;


}
