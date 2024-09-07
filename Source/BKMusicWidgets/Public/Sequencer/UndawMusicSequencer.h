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
#include <M2SoundGraphData.h>

DECLARE_DELEGATE_OneParam(
	FOnVerticalMajorSlotResized,
	/** The new size coefficient of the slot */
	float);

DECLARE_DELEGATE_OneParam(
	FOnVerticalMajorSlotHover,
	/** called when the spacer is hovered so we can change its color */
	bool);

class SDawSequencerTrackSection : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDawSequencerTrackSection) {}
	SLATE_END_ARGS()

	FLinkedNotesClip* Clip = nullptr;
	bool bIsHovered = false;


	void Construct(const FArguments& InArgs, FLinkedNotesClip* InClip)
	{
		Clip = InClip;
	}

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

};

class SDawSequencerTrackLane : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDawSequencerTrackLane) {}
	SLATE_END_ARGS()

	UDAWSequencerData* SequenceData = nullptr;
	int32 TrackId = INDEX_NONE;
	TArray<TSharedPtr<SDawSequencerTrackSection>> Sections;
	bool bIsHoveringOverSectionDragArea = false;
	bool bIsHoveringOverSectionResizeArea = false;
	int32 HoveringOverSectionIndex = INDEX_NONE;

	void Construct(const FArguments& InArgs, UDAWSequencerData* InSequenceToEdit, int32 InTrackId)
	{
		SequenceData = InSequenceToEdit;
		TrackId = InTrackId;

		PopulateSections();

	}

	void PopulateSections()
	{
		for (auto& Clip : SequenceData->Tracks[TrackId].LinkedNotesClips)
		{
			TSharedPtr<SDawSequencerTrackSection> Section;
			
			SAssignNew(Section, SDawSequencerTrackSection, &Clip);
			Section->AssignParentWidget(SharedThis(this));
			Sections.Add(Section);
		}
	}

	TOptional<EMouseCursor::Type> GetCursor() const override {
		if (bIsHoveringOverSectionResizeArea)
		{
			return EMouseCursor::ResizeLeftRight;
		}
		else if (bIsHoveringOverSectionDragArea)
		{
			return EMouseCursor::CardinalCross;
		}
		else {
			return EMouseCursor::Default;
		}

	}

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override {

		//UE_LOG(LogTemp, Warning, TEXT("Mouse moved over lane"));
		//check if hovering over section
		const float MouseLocalX = MouseEvent.GetScreenSpacePosition().X - MyGeometry.GetAbsolutePosition().X;
		const float MouseToPixel = MouseLocalX * 200;
		constexpr int32 SectionResizeAreaWidth = 5;

		//UE_LOG(LogTemp, Warning, TEXT("MouseLocalX: %f"), MouseLocalX);
		bool bAnySectionHovered = false;
		for (int32 i = 0; i < Sections.Num(); i++)
		{
			TRange<int32> SectionRange{ Sections[i]->Clip->StartTick, Sections[i]->Clip->EndTick };
			if (SectionRange.Contains((FMath::FloorToInt32(MouseToPixel))))
			{
				HoveringOverSectionIndex = i;
				Sections[i]->bIsHovered = true;
				bAnySectionHovered = true;
				TRange<int32> NonResizeRange{ Sections[i]->Clip->StartTick + SectionResizeAreaWidth * 200, Sections[i]->Clip->EndTick - SectionResizeAreaWidth * 200 };
				if (NonResizeRange.Contains((FMath::FloorToInt32(MouseToPixel))))
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

};

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

class SDawSequencerTrackRoot : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SDawSequencerTrackRoot) {}
		//SLATE_ARGUMENT(SBorder::FArguments, ParentArgs)
	SLATE_END_ARGS()

	SSplitter::FOnSlotResized OnSlotResized;
	TSharedPtr<SSplitter> Splitter;
	FOnVerticalMajorSlotResized OnVerticalMajorSlotResized;
	TSharedPtr<SDAwSequencerTrackControlsArea> ControlsArea;
	TSharedPtr<SBox> LaneBox;
	TSharedPtr<SDawSequencerTrackLane> Lane;

	void Construct(const FArguments& InArgs, UDAWSequencerData* InSequenceToEdit, int32 TrackId);

	void ResizeSplitter(float InNewSize) {
		ControlsArea->ResizeControlsBox(InNewSize);
	}


private:
	SScrollBox::FSlot* ScrollBoxSlot;
	TSharedPtr<SBox> Box;
};


class BKMUSICWIDGETS_API SUndawMusicSequencer: public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUndawMusicSequencer) {}
		SLATE_ARGUMENT_DEFAULT(float, TimelineHeight) = 25.0f;
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UDAWSequencerData* InSequenceToEdit);


protected:

	float HorizontalScrollOffset = 0.0f;
	float TimelineHeight;
	bool bIsPanning = false;

	void PopulateSequencerFromDawData();

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	int32 PaintBackgroundGrid(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;
	int32 PaintTimeline(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;


	TOptional<EMouseCursor::Type> GetCursor() const override;

	FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;



	UDAWSequencerData* SequenceData = nullptr;

	TSharedPtr<SScrollBox> ScrollBox;
	TSharedPtr<SGridPanel> GridPanel;
	TArray<SGridPanel::FSlot*> LaneSlots;
	TArray<SGridPanel::FSlot*> TrackSlots;
	TSharedPtr<SSplitter> Splitter;

	TArray<SScrollBox::FSlot*> TrackSlotsScrollBox;

	float MajorTabWidth = 150.0f;
	float MajorTabAlpha = 0.0f;

	TArray<TSharedPtr<SDawSequencerTrackRoot>> TrackRoots;

	TMap<int32, FMusicalGridPoint> GridPointMap;
	
};