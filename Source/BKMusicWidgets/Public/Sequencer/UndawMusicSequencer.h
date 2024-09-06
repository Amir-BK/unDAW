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
#include <M2SoundGraphData.h>

DECLARE_DELEGATE_OneParam(
	FOnVerticalMajorSlotResized,
	/** The new size coefficient of the slot */
	float);

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

	TOptional<EMouseCursor::Type> GetCursor() const override;

	FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

};

class SDawSequencerTrackRoot : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SDawSequencerTrackRoot) {}
		//SLATE_ARGUMENT(SBorder::FArguments, ParentArgs)
	SLATE_END_ARGS()

	SSplitter::FOnSlotResized OnSlotResized;
	TSharedPtr<SSplitter> Splitter;
	FOnVerticalMajorSlotResized OnVerticalMajorSlotResized;

	void Construct(const FArguments& InArgs);

	void ResizeSplitter(float InNewSize) {
		Splitter->SlotAt(0).SetSizeValue(InNewSize);
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

	float TimelineHeight;

	void CreateGridPanel();
	void CreateScrollBox();

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	int32 PaintBackgroundGrid(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;
	int32 PaintTimeline(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;

	FReply OnLaneBorderMouseDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	FReply OnLaneBorderMouseUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	FReply OnLaneBorderMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

	UDAWSequencerData* SequenceData = nullptr;

	TSharedPtr<SScrollBox> ScrollBox;
	TSharedPtr<SGridPanel> GridPanel;
	TArray<SGridPanel::FSlot*> LaneSlots;
	TArray<SGridPanel::FSlot*> TrackSlots;
	TSharedPtr<SSplitter> Splitter;

	TArray<SScrollBox::FSlot*> TrackSlotsScrollBox;

	TArray<TSharedPtr<SDawSequencerTrackRoot>> TrackRoots;
	
};