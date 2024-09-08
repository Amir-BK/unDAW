#include "TrackEditor/UndawTrackEditor.h"
#include "ITimeSlider.h"
#include "M2SoundGraphData.h"
#include "SequencerSectionPainter.h"
#include "EditorStyleSet.h"
#include "MVVM/Views/ViewUtilities.h"
#include "Sections/MovieSceneEventTriggerSection.h"
#include "TimeToPixel.h"
#include "ISequencer.h"
#include "IDetailsView.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include <Sequencer/UndawMidiMovieSceneTrackSection.h>

FUndawMovieTrackEditor::FUndawMovieTrackEditor(TSharedRef<ISequencer> InSequencer) : FMovieSceneTrackEditor(InSequencer)

{

}

TSharedRef<ISequencerTrackEditor> FUndawMovieTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> InSequencer)
{

	return MakeShareable(new FUndawMovieTrackEditor(InSequencer));
}

bool FUndawMovieTrackEditor::HandleAssetAdded(UObject* Asset, const FGuid& TargetObjectGuid)
{
	UE_LOG(LogTemp, Warning, TEXT("HandleAssetAdded"));
	//GetSequencer()->GetTopTimeSliderWidget()->SetColorAndOpacity

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
	return MakeShareable(new FUndawSequencerSectionPainter(SectionObject, GetSequencer()));
}

void FUndawMovieTrackEditor::Resize(float NewSize, UMovieSceneTrack* InTrack)
{
	auto Track = Cast<UUndawSequenceMovieSceneTrack>(InTrack);
	if (Track)
	{
		for (auto& Section : Track->GetAllSections())
		{
			auto MidiSection = Cast<UUndawMidiMovieSceneTrackSection>(Section);
			if (MidiSection)
			{
				MidiSection->SectionHeight = NewSize;
			}
		}
	}
}

TSharedRef<SWidget> FUndawMovieTrackEditor::BuildDAWSubMenu(FOnAssetSelected OnAssetSelected, FOnAssetEnterPressed OnAssetEnterPressed)
{
	//test hack add a new event section to the track
	UE_LOG(LogTemp, Warning, TEXT("Build DAW Sub Menu"));

	
	auto TrackResult = FindOrCreateRootTrack<UUndawSequenceMovieSceneTrack>();
	auto Track = TrackResult.Track;



	if (Track)
	{

		auto NewSection = NewObject<UUndawMidiMovieSceneTrackSection>(Track, NAME_None, RF_Transactional);
		NewSection->SetRange(TRange<FFrameNumber>(FFrameNumber(0), FFrameNumber(1000)));
		Track->AddSection(*NewSection);
		GetSequencer()->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
	}
	
	return SNullWidget::NullWidget;
}

void FUndawMovieTrackEditor::OnDawAssetSelected(const FAssetData& AssetData, UMovieSceneTrack* Track)
{
}

void FUndawMovieTrackEditor::OnDawAssetEnterPressed(const TArray<FAssetData>& AssetData, UMovieSceneTrack* Track)
{
}

FKeyPropertyResult FUndawMovieTrackEditor::AddNewSound(FFrameNumber KeyTime, UDAWSequencerData* DAWData, UUndawSequenceMovieSceneTrack* Track, int32 RowIndex)
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
		Track->DAWSequencerData = DAWData;
		Track->Modify();
		Track->SetDisplayName(FText::FromName(DAWData->GetFName()));

		auto* NewSection = Track->AddNewDAWDataOnRow(DAWData, KeyTime, RowIndex);

		if (TrackResult.bWasCreated)
		{

			if (GetSequencer().IsValid())
			{
				GetSequencer()->OnAddTrack(Track, FGuid());
			}
		}
		KeyPropertyResult.SectionsCreated.Add(NewSection);


		KeyPropertyResult.bTrackModified = true;


	}
	
	return KeyPropertyResult;
}
;

FUndawSequencerSectionPainter::FUndawSequencerSectionPainter(UMovieSceneSection& InSection, TWeakPtr<ISequencer> InSequencer)
	: Section(InSection)
{

}

FText FUndawSequencerSectionPainter::GetSectionTitle() const
{
	auto UDawSection = Cast<UUndawMidiMovieSceneTrackSection>(&Section);
	auto SequencerData = UDawSection->DAWData;
	auto TrackName = SequencerData ? SequencerData->GetTrackMetadata(UDawSection->TrackIndexInParentSession).TrackName : "DAW Sequence";
	
	return FText::FromString(TrackName);
}

float FUndawSequencerSectionPainter::GetSectionHeight() const
{
	
	return Cast<UUndawMidiMovieSceneTrackSection>(&Section)->SectionHeight;
}

int32 FUndawSequencerSectionPainter::OnPaintSection(FSequencerSectionPainter& InPainter) const
{
	//InPainter.

	//FSlateDrawElement::MakeBox(InPainter.DrawElements, InPainter.LayerId, InPainter.SectionGeometry.ToPaintGeometry(), FEditorStyle::GetBrush("Sequencer.Section.Background"), ESlateDrawEffect::None, FLinearColor::White);
	InPainter.PaintSectionBackground();

	const FTimeToPixel& TimeToPixelConverter = InPainter.GetTimeConverter();
	FFrameRate TickResolution = TimeToPixelConverter.GetTickResolution();
	auto UDawSection = Cast<UUndawMidiMovieSceneTrackSection>(&Section);
	auto SequencerData = UDawSection->DAWData;
	if (SequencerData)
	{

		const float SectionStartTime = TickResolution.AsSeconds(UDawSection->GetInclusiveStartFrame());
		auto LinkedNotesTracks = SequencerData->LinkedNoteDataMap;
		for (const auto& [Index,Track] : LinkedNotesTracks)
		{
			const auto& TrackColor = SequencerData->GetTrackMetadata(Index).trackColor;
			auto StoredSectionHeight = GetSectionHeight();
			for (const auto& Note : Track.LinkedNotes)
			{
				//map pitch to height
				const auto& NotePitch = StoredSectionHeight / 127 * (127- Note.Pitch);

				//convert note start time to pixel
				const auto& NoteOffset = (Note.StartTime * .001f) + SectionStartTime;
				float StartPixel = TimeToPixelConverter.SecondsToPixel(NoteOffset);
				float EndPixel = TimeToPixelConverter.SecondsToPixel(NoteOffset + (Note.Duration * .001f));
				//draw line a line from start time to end time
				FSlateDrawElement::MakeLines(InPainter.DrawElements, InPainter.LayerId, InPainter.SectionGeometry.ToPaintGeometry(), TArray<FVector2D>{FVector2D(StartPixel, NotePitch), FVector2D(EndPixel, NotePitch)}, ESlateDrawEffect::None, TrackColor, false);
			}
		}

	}


	return InPainter.LayerId;
}

UMovieSceneSection* FUndawSequencerSectionPainter::GetSectionObject()
{
	return &Section;
}

void FUndawSequencerSectionPainter::CustomizePropertiesDetailsView(TSharedRef<IDetailsView> DetailsView, const FSequencerSectionPropertyDetailsViewCustomizationParams& InParams) const
{
	//DetailsView->Chil
}

FMidiSceneConductorSectionPainter::FMidiSceneConductorSectionPainter(UMovieSceneSection& InSection, TWeakPtr<ISequencer> InSequencer) : Section(InSection)
{
	TimeSlider = InSequencer.Pin()->GetTopTimeSliderWidget();
}

FText FMidiSceneConductorSectionPainter::GetSectionTitle() const
{
	return FText::GetEmpty();
}

float FMidiSceneConductorSectionPainter::GetSectionHeight() const
{
	return 50.0f;
}

int32 FMidiSceneConductorSectionPainter::OnPaintSection(FSequencerSectionPainter& InPainter) const
{
	FSlateDrawElement::MakeBox(InPainter.DrawElements, InPainter.LayerId, InPainter.SectionGeometry.ToPaintGeometry(), FEditorStyle::GetBrush("Sequencer.Section.Background"), ESlateDrawEffect::None, FLinearColor::White);
	//draw text saying "conductor"

	FSlateDrawElement::MakeText(InPainter.DrawElements, InPainter.LayerId, InPainter.SectionGeometry.ToPaintGeometry(), FText::FromString("Conductor"), FEditorStyle::GetFontStyle("NormalFont"), ESlateDrawEffect::None, FLinearColor::White);
	
	return int32();
}

UMovieSceneSection* FMidiSceneConductorSectionPainter::GetSectionObject()
{
	return &Section;
}
