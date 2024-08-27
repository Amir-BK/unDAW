#pragma once


#include "CoreMinimal.h"

#include "Widgets/SWidget.h"
#include "ISequencerSection.h"
#include "MovieSceneTrack.h"
#include "ISequencer.h"
#include "ISequencerTrackEditor.h"
#include "MovieSceneTrackEditor.h"
#include "MovieSceneSection.h"
#include "IContentBrowserSingleton.h"
#include <Sequencer/UndawSequenceMovieSceneTrack.h>


class FMidiSceneConductorSectionPainter : public ISequencerSection
{
	FMidiSceneConductorSectionPainter(UMovieSceneSection& InSection, TWeakPtr<ISequencer> InSequencer);


	virtual FText GetSectionTitle() const override;

	virtual float GetSectionHeight() const override;

	virtual int32 OnPaintSection(FSequencerSectionPainter& InPainter) const override;

	virtual UMovieSceneSection* GetSectionObject() override;

	/** The section we are visualizing. */
	UMovieSceneSection& Section;

	TSharedPtr<ITimeSlider> TimeSlider;

};

class FMidiSceneSectionPainter : public ISequencerSection
{
public:
	FMidiSceneSectionPainter(UMovieSceneSection& InSection, TWeakPtr<ISequencer> InSequencer);

	virtual FText GetSectionTitle() const override;

	virtual float GetSectionHeight() const override;

	virtual int32 OnPaintSection(FSequencerSectionPainter& InPainter) const override;

	virtual UMovieSceneSection* GetSectionObject() override;

	/** The section we are visualizing. */
	UMovieSceneSection& Section;
};

class FUndawMovieTrackEditor : public FMovieSceneTrackEditor
{
	
	public:
	FUndawMovieTrackEditor(TSharedRef<ISequencer> InSequencer);
	// ISequencerTrackEditor interface


	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> InSequencer);


	virtual bool HandleAssetAdded(UObject* Asset, const FGuid& TargetObjectGuid) override;
	virtual bool SupportsType(TSubclassOf<UMovieSceneTrack> TrackClass) const override;
	virtual void BuildAddTrackMenu(FMenuBuilder& MenuBuilder) override;
	virtual TSharedPtr<SWidget> BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params) override;
	virtual TSharedRef<ISequencerSection> MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding) override;
	

	//

private:

	/** Audio sub menu */
	TSharedRef<SWidget> BuildDAWSubMenu(FOnAssetSelected OnAssetSelected, FOnAssetEnterPressed OnAssetEnterPressed);

	/** Audio asset selected */
	void OnDawAssetSelected(const FAssetData& AssetData, UMovieSceneTrack* Track);

	/** Audio asset enter pressed */
	void OnDawAssetEnterPressed(const TArray<FAssetData>& AssetData, UMovieSceneTrack* Track);

	// The sequencer bound to this handler.  Used to access movie scene and time info during auto-key
protected:

	/** Delegate for AnimatablePropertyChanged in HandleAssetAdded for sounds */
	FKeyPropertyResult AddNewSound(FFrameNumber KeyTime, class UDAWSequencerData* Sound, UUndawSequenceMovieSceneTrack* Track, int32 RowIndex);

	//TSharedPtr<ISequencer> Sequencer;
};