
#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWidget.h"
#include "Input/Events.h"
#include "Components/TextBlock.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBox.h"
#include "UndawMusicDrawingStatics.h"
#include "Sequencer/UndawSequencerSection.h"
#include "Types/SlateConstants.h"
#include "MidiClipEditor/SMidiClipEditor.h"
#include "M2SoundGraphData.h"
//#include "UndawMusicSequencer.h"


DECLARE_DELEGATE_OneParam(
	FOnSectionSelected,
	/** called when the spacer is hovered so we can change its color */
	TSharedPtr<SDawSequencerTrackMidiSection>);

DECLARE_DELEGATE_OneParam(
	FOnVerticalMajorSlotResized,
	/** The new size coefficient of the slot */
	float);

DECLARE_DELEGATE_OneParam(
	FOnVerticalMajorSlotHover,
	/** called when the spacer is hovered so we can change its color */
	bool);


class SDAwSequencerTrackControlsArea : public SCompoundWidget
{

public:
	SLATE_BEGIN_ARGS(SDAwSequencerTrackControlsArea) {}
		SLATE_ATTRIBUTE(float, DesiredWidth)
	SLATE_END_ARGS()

	TAttribute<float> DesiredWidth;
	bool bIsResizing = false;
	bool bIsHoveringOverResizeArea = false;
	void Construct(const FArguments& InArgs);

	FOnVerticalMajorSlotResized OnVerticalMajorSlotResized;
	FOnVerticalMajorSlotHover OnVerticalMajorSlotHover;
	TSharedPtr<SBox> ControlsBox;
	TSharedPtr<SBox> LaneBox;

	TOptional<EMouseCursor::Type> GetCursor() const override;

	FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	void ResizeControlsBox(float InNewSize) {
		ControlsBox->SetWidthOverride(InNewSize);
	}

};

class SDawSequencerTrackLane : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDawSequencerTrackLane) {}
		SLATE_ATTRIBUTE(FVector2D, Position)
		SLATE_ATTRIBUTE(FVector2D, Zoom)
	SLATE_END_ARGS()

	UDAWSequencerData* SequenceData = nullptr;
	int32 TrackId = INDEX_NONE;
	TArray<TSharedPtr<SDawSequencerTrackMidiSection>> Sections;
	bool bIsHoveringOverSectionDragArea = false;
	bool bIsHoveringOverSectionResizeArea = false;
	int32 HoveringOverSectionIndex = INDEX_NONE;
	int32 SelectedSectionIndex = INDEX_NONE;
	TAttribute<FVector2D> Position;
	TAttribute<FVector2D> Zoom;

	float HorizontalOffset = 0.0f;

	void Construct(const FArguments& InArgs, UDAWSequencerData* InSequenceToEdit, int32 InTrackId);

	//void PopulateSections()
	//{

	//}

	TOptional<EMouseCursor::Type> GetCursor() const override;

	FOnSectionSelected OnSectionSelected;

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override {

		//UE_LOG(LogTemp, Warning, TEXT("Mouse moved over lane"));
		//check if hovering over section
		const float MouseLocalX = MouseEvent.GetScreenSpacePosition().X - MyGeometry.GetAbsolutePosition().X + Position.Get().X;
		const float MouseToPixel = MouseLocalX; //* Zoom.Get().X;
		constexpr int32 SectionResizeAreaWidth = 5;

		bool bAnySectionHovered = false;
		for (int32 i = 0; i < Sections.Num(); i++)
		{
			TRange<double> SectionRange{ Sections[i]->Clip->StartTick * Zoom.Get().X, Sections[i]->Clip->EndTick * Zoom.Get().X };
			if (SectionRange.Contains(MouseToPixel))
			{
				HoveringOverSectionIndex = i;
				Sections[i]->bIsHovered = true;
				bAnySectionHovered = true;
				TRange<double> NonResizeRange{ Sections[i]->Clip->StartTick + SectionResizeAreaWidth * Zoom.Get().X, Sections[i]->Clip->EndTick - SectionResizeAreaWidth * Zoom.Get().X };
				if (NonResizeRange.Contains(MouseToPixel))
				{
					bIsHoveringOverSectionDragArea = true;
					bIsHoveringOverSectionResizeArea = false;
				}
				else {
					bIsHoveringOverSectionDragArea = false;
					bIsHoveringOverSectionResizeArea = true;
				}

			}
			else {
				Sections[i]->bIsHovered = false;
			}
		}

		if (!bAnySectionHovered)
		{
			HoveringOverSectionIndex = INDEX_NONE;
			bIsHoveringOverSectionDragArea = false;
			bIsHoveringOverSectionResizeArea = false;
		}
		else {
			bIsHoveringOverSectionDragArea = true;
		}


		return FReply::Unhandled();

	}

	FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override {

		const bool bIsLeftMoustButtonEffecting = MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;

		if (bIsLeftMoustButtonEffecting && bIsHoveringOverSectionDragArea)
		{
			OnSectionSelected.ExecuteIfBound(Sections[HoveringOverSectionIndex]);
			SelectedSectionIndex = HoveringOverSectionIndex;
			return FReply::Handled();
		};

		return FReply::Unhandled();
	}
};



class SDawSequencerTrackRoot : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SDawSequencerTrackRoot) {}
		SLATE_ATTRIBUTE(FVector2D, Position)
		SLATE_ATTRIBUTE(FVector2D, Zoom)
		//SLATE_ARGUMENT(SBorder::FArguments, ParentArgs)
	SLATE_END_ARGS()

	SSplitter::FOnSlotResized OnSlotResized;
	TSharedPtr<SSplitter> Splitter;
	FOnVerticalMajorSlotResized OnVerticalMajorSlotResized;
	TSharedPtr<SDAwSequencerTrackControlsArea> ControlsArea;
	TSharedPtr<SBox> LaneBox;
	TSharedPtr<SDawSequencerTrackLane> Lane;

	float HorizontalOffset = 0.0f;

	void Construct(const FArguments& InArgs, UDAWSequencerData* InSequenceToEdit, int32 TrackId);

	void ResizeSplitter(float InNewSize) {
		ControlsArea->ResizeControlsBox(InNewSize);
	}


private:
	SScrollBox::FSlot* ScrollBoxSlot;
	TSharedPtr<SBox> Box;
};
