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
#include "Types/SlateConstants.h"
#include <M2SoundGraphData.h>



class SDawSequencerTrackMidiSection : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDawSequencerTrackMidiSection) {}
		SLATE_ATTRIBUTE(FLinearColor, TrackColor)
		SLATE_ATTRIBUTE(FVector2D, Position)
		SLATE_ATTRIBUTE(FVector2D, Zoom)
	SLATE_END_ARGS()

	FLinkedNotesClip* Clip = nullptr;
	FDawSequencerTrack* ParentTrack = nullptr;
	bool bIsHovered = false;
	bool bIsSelected = false;

	//needed for songs map the such 
	TObjectPtr<UDAWSequencerData> SequenceData;

	TAttribute<FVector2D> Position;
	TAttribute<FVector2D> Zoom;

	TAttribute<FLinearColor> TrackColor;

	void Construct(const FArguments& InArgs, FLinkedNotesClip* InClip, FDawSequencerTrack* InParentTrack, UDAWSequencerData* InSequenceToEdit);

	float CalculateXPosition(float Tick) const;

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	const float TickToPixel(const float Tick) const;

};