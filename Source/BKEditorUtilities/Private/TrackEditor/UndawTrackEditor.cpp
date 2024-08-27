#include "TrackEditor/UndawTrackEditor.h"
#include "ITimeSlider.h"
#include "M2SoundGraphData.h"
#include "SequencerSectionPainter.h"
#include "EditorStyleSet.h"
#include "MVVM/Views/ViewUtilities.h"
#include <Sequencer/UndawMidiMovieSceneTrackSection.h>

FUndawMovieTrackEditor::FUndawMovieTrackEditor(TSharedRef<ISequencer> InSequencer) : FMovieSceneTrackEditor(InSequencer)

{
	//Sequencer = InSequencer;

}

TSharedRef<ISequencerTrackEditor> FUndawMovieTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> InSequencer)
{

	return MakeShareable(new FUndawMovieTrackEditor(InSequencer));
}

bool FUndawMovieTrackEditor::HandleAssetAdded(UObject* Asset, const FGuid& TargetObjectGuid)
{
	UE_LOG(LogTemp, Warning, TEXT("HandleAssetAdded"));
	GetSequencer()->GetTopTimeSliderWidget()->SetColorAndOpacity(FLinearColor::Blue);
	if (Asset->IsA<UDAWSequencerData>())
	{
		
		UDAWSequencerData* DAWData = Cast<UDAWSequencerData>(Asset);
		UUndawSequenceMovieSceneTrack* DummyTrack = nullptr;
		TArray<TWeakObjectPtr<>> OutObjects;

		const FScopedTransaction Transaction(FText::FromString("Add Daw Session"));

		int32 RowIndex = INDEX_NONE;
		AnimatablePropertyChanged(FOnKeyProperty::CreateRaw(this, &FUndawMovieTrackEditor::AddNewSound, DAWData, DummyTrack, RowIndex));
		
		
		return true;
	}

	return false;
}
bool FUndawMovieTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> TrackClass) const
{
	return UUndawSequenceMovieSceneTrack::StaticClass() == TrackClass;
}
void FUndawMovieTrackEditor::BuildAddTrackMenu(FMenuBuilder& MenuBuilder)
{
}
TSharedPtr<SWidget> FUndawMovieTrackEditor::BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params)
{
	//auto Timeline = Sequencer->GetTopTimeSliderWidget();
	//Timeline->SetForegroundColor(FLinearColor::Blue);

	
	return UE::Sequencer::MakeAddButton(INVTEXT("MIDI"), FOnGetContent::CreateSP(this, &FUndawMovieTrackEditor::BuildDAWSubMenu, FOnAssetSelected::CreateRaw(this, &FUndawMovieTrackEditor::OnDawAssetSelected, Track), FOnAssetEnterPressed::CreateRaw(this, &FUndawMovieTrackEditor::OnDawAssetEnterPressed, Track)), Params.ViewModel);
}
TSharedRef<ISequencerSection> FUndawMovieTrackEditor::MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding)
{
	return MakeShareable(new FMidiSceneSectionPainter(SectionObject, GetSequencer()));
}

TSharedRef<SWidget> FUndawMovieTrackEditor::BuildDAWSubMenu(FOnAssetSelected OnAssetSelected, FOnAssetEnterPressed OnAssetEnterPressed)
{
	return SNullWidget::NullWidget;
}

void FUndawMovieTrackEditor::OnDawAssetSelected(const FAssetData& AssetData, UMovieSceneTrack* Track)
{
}

void FUndawMovieTrackEditor::OnDawAssetEnterPressed(const TArray<FAssetData>& AssetData, UMovieSceneTrack* Track)
{
}

FKeyPropertyResult FUndawMovieTrackEditor::AddNewSound(FFrameNumber KeyTime, UDAWSequencerData* Sound, UUndawSequenceMovieSceneTrack* Track, int32 RowIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("Add New Daw Sequence"));
	
	FKeyPropertyResult KeyPropertyResult;

	UMovieScene* FocusedMovieScene = GetFocusedMovieScene();
	if (FocusedMovieScene->IsReadOnly())
	{
		return KeyPropertyResult;
	}

	FFindOrCreateRootTrackResult< UUndawSequenceMovieSceneTrack > TrackResult;// = FindOrCreateRootTrack<UUndawSequenceMovieSceneTrack>(FocusedMovieScene);

	TrackResult.Track = Track;

	if (!Track)
	{
		TrackResult = FindOrCreateRootTrack<UUndawSequenceMovieSceneTrack>();
		Track = TrackResult.Track;
	}

	if (ensure(Track))
	{
		Track->DAWSequencerData = Sound;
		Track->Modify();
		

		// Create a new section
		auto* NewSection = Track->AddNewDAWDataOnRow(Sound, KeyTime, RowIndex);
		//NewSection->DAWSequencerData = Sound;
		if (TrackResult.bWasCreated)
		{
			Track->SetDisplayName(FText::FromName(Sound->GetFName()));

			if (GetSequencer().IsValid())
			{
				GetSequencer()->OnAddTrack(Track, FGuid());
			}
		}
		KeyPropertyResult.bTrackModified = true;
		KeyPropertyResult.SectionsCreated.Add(NewSection);

	

	}
	
	return KeyPropertyResult;
}
;

FMidiSceneSectionPainter::FMidiSceneSectionPainter(UMovieSceneSection& InSection, TWeakPtr<ISequencer> InSequencer)
	: Section(InSection)
{
	

}

FText FMidiSceneSectionPainter::GetSectionTitle() const
{
	return INVTEXT("DAW Sequence test");
}

float FMidiSceneSectionPainter::GetSectionHeight() const
{
	return 100.0f;
}

int32 FMidiSceneSectionPainter::OnPaintSection(FSequencerSectionPainter& InPainter) const
{
	FSlateDrawElement::MakeBox(InPainter.DrawElements, InPainter.LayerId, InPainter.SectionGeometry.ToPaintGeometry(), FEditorStyle::GetBrush("Sequencer.Section.Background"), ESlateDrawEffect::None, FLinearColor::White);
	//InPainter->DrawText(FVector2D(0, 0), FText::FromString("DAW Sequence"), FEditorStyle::GetFontStyle("NormalFont"), 1.0f, FLinearColor::White);
	InPainter.PaintSectionBackground();
	return InPainter.LayerId;
}

UMovieSceneSection* FMidiSceneSectionPainter::GetSectionObject()
{
	return &Section;
}
