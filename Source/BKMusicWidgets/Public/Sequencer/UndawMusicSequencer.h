#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWidget.h"
#include "Components/TextBlock.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include <M2SoundGraphData.h>

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

	UDAWSequencerData* SequenceData = nullptr;

	TSharedPtr<SScrollBox> ScrollBox;
	TSharedPtr<SGridPanel> GridPanel;
	TArray<SGridPanel::FSlot*> LaneSlots;
	TArray<SGridPanel::FSlot*> TrackSlots;

	TArray<SScrollBox::FSlot*> TrackSlotsScrollBox;
	
};