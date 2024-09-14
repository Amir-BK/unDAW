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
#include "MidiClipEditor/SMidiClipEditor.h"
#include <M2SoundGraphData.h>


class SDawSequencerTrackSection;

DECLARE_DELEGATE_OneParam(
	FOnVerticalMajorSlotResized,
	/** The new size coefficient of the slot */
	float);

DECLARE_DELEGATE_OneParam(
	FOnVerticalMajorSlotHover,
	/** called when the spacer is hovered so we can change its color */
	bool);

DECLARE_DELEGATE_OneParam(
	FOnSectionSelected,
	/** called when the spacer is hovered so we can change its color */
	TSharedPtr<SDawSequencerTrackSection>);

typedef TTuple<FDawSequencerTrack*, FLinkedNotesClip*> ClipTrackTuple;

DECLARE_DELEGATE_OneParam(
	FOnMidiClipsFocused,
	TArray<ClipTrackTuple>);

class SDawSequencerTrackSection : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDawSequencerTrackSection) {}
		SLATE_ATTRIBUTE(FLinearColor, TrackColor)
		SLATE_ATTRIBUTE(FVector2D, Position)
		SLATE_ATTRIBUTE(FVector2D, Zoom)
	SLATE_END_ARGS()

	FLinkedNotesClip* Clip = nullptr;
	FDawSequencerTrack* ParentTrack = nullptr;
	bool bIsHovered = false;
	bool bIsSelected = false;

	TAttribute<FVector2D> Position;
	TAttribute<FVector2D> Zoom;


	TAttribute<FLinearColor> TrackColor;

	void Construct(const FArguments& InArgs, FLinkedNotesClip* InClip, FDawSequencerTrack* InParentTrack)
	{
		Clip = InClip;
		TrackColor = InArgs._TrackColor;
		ParentTrack = InParentTrack;
		Position = InArgs._Position;
		Zoom = InArgs._Zoom;
	}

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

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
	TArray<TSharedPtr<SDawSequencerTrackSection>> Sections;
	bool bIsHoveringOverSectionDragArea = false;
	bool bIsHoveringOverSectionResizeArea = false;
	int32 HoveringOverSectionIndex = INDEX_NONE;
	int32 SelectedSectionIndex = INDEX_NONE;
	TAttribute<FVector2D> Position;
	TAttribute<FVector2D> Zoom;

	float HorizontalOffset = 0.0f;

	void Construct(const FArguments& InArgs, UDAWSequencerData* InSequenceToEdit, int32 InTrackId)
	{
		SequenceData = InSequenceToEdit;
		TrackId = InTrackId;
		Position = InArgs._Position;
		Zoom = InArgs._Zoom;

		//PopulateSections();

		for (auto& Clip : SequenceData->Tracks[TrackId].LinkedNotesClips)
		{
			TSharedPtr<SDawSequencerTrackSection> Section;


			SAssignNew(Section, SDawSequencerTrackSection, &Clip, &SequenceData->Tracks[TrackId])
				.TrackColor(TAttribute<FLinearColor>::CreateLambda([this]() {return SequenceData->GetTrackMetadata(TrackId).TrackColor; }))
				.Position(InArgs._Position)
				.Zoom(InArgs._Zoom);

			Section->AssignParentWidget(SharedThis(this));
			Sections.Add(Section);
		}

	}

	//void PopulateSections()
	//{

	//}

	TOptional<EMouseCursor::Type> GetCursor() const override {
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

	FOnSectionSelected OnSectionSelected;

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override {

		//UE_LOG(LogTemp, Warning, TEXT("Mouse moved over lane"));
		//check if hovering over section
		const float MouseLocalX = MouseEvent.GetScreenSpacePosition().X - MyGeometry.GetAbsolutePosition().X + HorizontalOffset;
		const float MouseToPixel = MouseLocalX * 200;
		constexpr int32 SectionResizeAreaWidth = 5;

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


class BKMUSICWIDGETS_API SUndawMusicSequencer: public SMidiEditorPanelBase
{
public:
	SLATE_BEGIN_ARGS(SUndawMusicSequencer) {}
		SLATE_ARGUMENT(SMidiEditorPanelBase::FArguments, ParentArgs);
		SLATE_ARGUMENT_DEFAULT(float, TimelineHeight) = 25.0f;
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UDAWSequencerData* InSequenceToEdit);
	FOnMidiClipsFocused OnMidiClipsFocused;

protected:

	
	bool bIsPanning = false;

	TSharedPtr<SDawSequencerTrackSection> SelectedSection;


	void PopulateSequencerFromDawData();
	void OnSectionSelected(TSharedPtr<SDawSequencerTrackSection> InSelectedSection)
	{
		if(SelectedSection.IsValid())	SelectedSection->bIsSelected = false;
		SelectedSection = InSelectedSection;
		SelectedSection->bIsSelected = true;

		//in the future might support multiple focused clips
		if (OnMidiClipsFocused.IsBound())
		{
			TArray<TTuple<FDawSequencerTrack*, FLinkedNotesClip*>> FocusedClips;
			FocusedClips.Add(TTuple<FDawSequencerTrack*, FLinkedNotesClip*>(SelectedSection->ParentTrack, SelectedSection->Clip));
			OnMidiClipsFocused.Execute(FocusedClips);

		}
	}

	int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	int32 PaintBackgroundGrid(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;
	

	FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;


	FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	void OnVerticalScroll(float ScrollAmount) override
	{
		ScrollBox->SetScrollOffset(ScrollBox->GetScrollOffset() - (GetGlobalScrollAmount() * ScrollAmount));
	}
public:
	bool AreAssetsAcceptableForDrop(const TArray<FAssetData>& Assets)
	{
		//accepts midi files, USoundWaves, UDAWSequencerData and UMetaSoundsource
		for (const auto& Asset : Assets)
		{
			if (Asset.GetClass()->IsChildOf(UMidiFile::StaticClass()) || Asset.GetClass()->IsChildOf(USoundWave::StaticClass()) ||
				Asset.GetClass()->IsChildOf(UDAWSequencerData::StaticClass()))
			{
				return true;
			}
		}

		return false;
	}
protected:

	TSharedPtr<SScrollBox> ScrollBox;
	TSharedPtr<SSplitter> Splitter;

	TArray<SScrollBox::FSlot*> TrackSlotsScrollBox;



	TArray<TSharedPtr<SDawSequencerTrackRoot>> TrackRoots;

	
};