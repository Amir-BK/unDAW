// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Events.h"
#include "Widgets/SCompoundWidget.h"
#include "M2SoundGraphData.h"
#include "SlateFwd.h"
#include "Components/Widget.h"


/**
implementations for panning and zooming

*/

class IMidiEditorPanelInterface
{
public:

	float Zoom = 1.0f;
	float HorizontalOffset = 0.0f;
	float VerticalOffset = 0.0f;
	bool bIsPanActive = false;
	bool bLockVerticalPan = false;

	FReply OnMousePan(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
	{
		if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
		{
			HorizontalOffset += MouseEvent.GetCursorDelta().X;
			if(!bLockVerticalPan) VerticalOffset += MouseEvent.GetCursorDelta().Y;
			bIsPanActive = true;
			return FReply::Handled();
		}
		bIsPanActive = false;
		return FReply::Unhandled();
	}

};

/**
 * 
 */
class BKMUSICWIDGETS_API SMidiClipEditor : public SCompoundWidget, public IMidiEditorPanelInterface
{
public:
	SLATE_BEGIN_ARGS(SMidiClipEditor)
	{}
	SLATE_END_ARGS()

	FText TrackMetaDataName = FText::GetEmpty();
	int32 TrackIndex = INDEX_NONE;
	FLinearColor TrackColor = FLinearColor::White;
	UDAWSequencerData* SequenceData = nullptr;
	FLinkedNotesClip* Clip = nullptr;

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, UDAWSequencerData* InSequence);

	void OnClipsFocused(TArray< TTuple<FDawSequencerTrack*, FLinkedNotesClip*> > Clips);

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if(OnMousePan(MyGeometry, MouseEvent).IsEventHandled()) return FReply::Handled();

		return FReply::Unhandled();
	}

};
