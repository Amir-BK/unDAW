#include "TrackEditor/UndawTrackEditor.h"
#include "ITimeSlider.h"
#include "M2SoundGraphData.h"
#include "SequencerSectionPainter.h"
#include "EditorStyleSet.h"
#include "MVVM/Views/ViewUtilities.h"
#include "TimeToPixel.h"
#include "ISequencer.h"
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
		
		for (size_t trackID = 0; trackID < DAWData->M2TrackMetadata.Num(); trackID++)
		{
			auto SectionMetaData = DAWData->M2TrackMetadata[trackID];
			// Create a new section
			auto* NewSection = Track->AddNewDAWDataOnRow(DAWData, KeyTime, trackID);
			
			//NewSection->DAWSequencerData = Sound;
			if (TrackResult.bWasCreated)
			{
				//NewSection->
				Track->SetTrackRowDisplayName(FText::FromString(SectionMetaData.trackName), trackID);
				if (GetSequencer().IsValid())
				{
					GetSequencer()->OnAddTrack(Track, FGuid());
				}
			}
			KeyPropertyResult.SectionsCreated.Add(NewSection);
		}


		KeyPropertyResult.bTrackModified = true;
		

	

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
	auto UDawSection = Cast<UUndawMidiMovieSceneTrackSection>(&Section);
	auto SequencerData = UDawSection->DAWSequencerData;
	auto TrackName = SequencerData ? SequencerData->GetTracksDisplayOptions(UDawSection->TrackIndexInParentSession).trackName : "DAW Sequence";
	
	return FText::FromString(TrackName);
}

float FMidiSceneSectionPainter::GetSectionHeight() const
{
	return 150.0f;
}

int32 FMidiSceneSectionPainter::OnPaintSection(FSequencerSectionPainter& InPainter) const
{
	//InPainter.

	FSlateDrawElement::MakeBox(InPainter.DrawElements, InPainter.LayerId, InPainter.SectionGeometry.ToPaintGeometry(), FEditorStyle::GetBrush("Sequencer.Section.Background"), ESlateDrawEffect::None, FLinearColor::White);

	//for fun, draw grid lines. vertical
	//for (int i = 0; i < 128; i++)


	//auto TimeSlider = InSequencer.Pin()->GetTopTimeSliderWidget();
	//auto TimeSliderParent = TimeSlider->GetParentWidget();
	////print timeline slider name and parent name
	//UE_LOG(LogTemp, Warning, TEXT("TimeSlider Name: %s"), *TimeSlider->GetWidgetClass().GetWidgetType().ToString());
	//UE_LOG(LogTemp, Warning, TEXT("TimeSlider Parent Name: %s"), *TimeSliderParent->GetWidgetClass().GetWidgetType().ToString());



	//InPainter->DrawText(FVector2D(0, 0), FText::FromString("DAW Sequence"), FEditorStyle::GetFontStyle("NormalFont"), 1.0f, FLinearColor::White);
	//Draw some text just for textin
	//FSlateDrawElement::MakeText(InPainter.DrawElements, InPainter.LayerId, InPainter.SectionGeometry.ToPaintGeometry(), FText::FromString("DAW Sequence TEST"), FEditorStyle::GetFontStyle("NormalFont"), ESlateDrawEffect::None, FLinearColor::White);
	const FTimeToPixel& TimeToPixelConverter = InPainter.GetTimeConverter();
	FFrameRate TickResolution = TimeToPixelConverter.GetTickResolution();
	auto UDawSection = Cast<UUndawMidiMovieSceneTrackSection>(&Section);
	auto SequencerData = UDawSection->DAWSequencerData;
	if (SequencerData)
	{

		//const auto& BarMap = SequencerData->HarmonixMidiFile->GetSongMaps()->GetBarMap();

		////draw 40 bars I'm lazy
		//for (int i = 0; i < 40; i++)
		//{
		//	const auto& Bar = BarMap.MusicTimestampBarToTick(i);
		//	const auto& BarTime = SequencerData->HarmonixMidiFile->GetSongMaps()->TickToMs(Bar);

		//	//FFrameTime BarFrameTime = FFrameTime(FrameRate.AsFrameTime(BarTime * .001f));
		//	float BarPixel = TimeToPixelConverter.SecondsToPixel(BarTime * .001f);

		//	//auto MarkedFrameTest = FMovieSceneMarkedFrame(FFrameNumber(BarFrameTime.FrameNumber));
		//	//MarkedFrameTest.Label = FString::Printf(TEXT("Bar %d"), i);
		//	//MarkedFrameTest.Color = FLinearColor::Green;
		//	// 
		//	// 
		//	//MovieScene->AddMarkedFrame(MarkedFrameTest);

		//	FSlateDrawElement::MakeLines(InPainter.DrawElements, InPainter.LayerId, InPainter.SectionGeometry.ToPaintGeometry(), TArray<FVector2D>{FVector2D(BarPixel, 0), FVector2D(BarPixel, 150.0f)}, ESlateDrawEffect::None, FLinearColor::White, false);
		//}



		//FSlateDrawElement::MakeText(InPainter.DrawElements, InPainter.LayerId, InPainter.SectionGeometry.ToPaintGeometry(), FText::FromString(SequencerData->GetFName().ToString()), FEditorStyle::GetFontStyle("NormalFont"), ESlateDrawEffect::None, FLinearColor::White);
		auto LinkedNotesTracks = SequencerData->LinkedNoteDataMap;
		const auto& Track = LinkedNotesTracks[UDawSection->TrackIndexInParentSession];
		const float SectionStartTime = TickResolution.AsSeconds(UDawSection->GetInclusiveStartFrame());
		//const auto& SectionOffset = UDawSection->SectionRange.GetLowerBound().GetValue();
		auto TrackColor = SequencerData->GetTracksDisplayOptions(UDawSection->TrackIndexInParentSession).trackColor;
		for (const auto& Note : Track.LinkedNotes)
		{
			const auto& NoteOffset = (Note.StartTime * .001f) + SectionStartTime;
			float StartPixel = TimeToPixelConverter.SecondsToPixel(NoteOffset);
			float EndPixel = TimeToPixelConverter.SecondsToPixel(NoteOffset + (Note.Duration * .001f));
			//draw line a line from start time to end time
			FSlateDrawElement::MakeLines(InPainter.DrawElements, InPainter.LayerId, InPainter.SectionGeometry.ToPaintGeometry(), TArray<FVector2D>{FVector2D(StartPixel, 127 - Note.pitch), FVector2D(EndPixel, 127 - Note.pitch)}, ESlateDrawEffect::None, TrackColor, false);
		}
	
	}

	InPainter.PaintSectionBackground();
	return InPainter.LayerId;
}

UMovieSceneSection* FMidiSceneSectionPainter::GetSectionObject()
{
	return &Section;
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
