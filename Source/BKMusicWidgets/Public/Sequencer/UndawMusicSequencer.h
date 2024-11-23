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
#include "UndawSequencerTrack.h"
#include "UndawSequencerSection.h"
#include <M2SoundGraphData.h>


typedef TTuple<FDawSequencerTrack*, FLinkedNotesClip*> ClipTrackTuple;

DECLARE_DELEGATE_OneParam(
	FOnMidiClipsFocused,
	TArray<ClipTrackTuple>);

template<typename T>
class SDawSequencerSection : public SLeafWidget
{
	T* ContentClip = nullptr;
	int32 TrackId = INDEX_NONE;
	UDAWSequencerData* SequenceData = nullptr;
	TAttribute<float> HorizontalZoom;

public:
	
	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(0.0f, 0.0f);
	}

private:
	double SectionTickDuration = 0.0;
	double SectionStartTick = 0.0;
	double SectionEndTick = 0.0;

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

	TSharedPtr<SDawSequencerTrackMidiSection> SelectedSection;


	void PopulateSequencerFromDawData();
	void OnSectionSelected(TSharedPtr<SDawSequencerTrackMidiSection> InSelectedSection)
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