#include "Sequencer/UndawSequenceMovieSceneTrack.h"

#include "MovieScene.h"
//#include "MidiObjects/MidiTrack.h"
#include "Sections/MovieSceneEventTriggerSection.h"
#include "Sequencer/UndawMidiMovieSceneTrackSection.h"


UMovieSceneSection* UUndawSequenceMovieSceneTrack::AddNewDAWDataOnRow(UDAWSequencerData* DAWData, FFrameNumber Time, int32 RowIndex)
{
	
	check(DAWData);

	FFrameRate FrameRate = GetTypedOuter<UMovieScene>()->GetTickResolution();
	FFrameTime DurationToUse = 1.f * FrameRate; // if all else fails, use 1 second duration


	const float Duration = DAWData->SequenceDuration * .001f;

	DurationToUse = Duration * FrameRate;



	
	//add the section
	UUndawMidiMovieSceneTrackSection* NewEvaluationSection = Cast<UUndawMidiMovieSceneTrackSection>(CreateNewSection());
	NewEvaluationSection->InitialPlacementOnRow(DAWSections, Time, DurationToUse.FrameNumber.Value, RowIndex);
	NewEvaluationSection->DAWData = DAWData;
	NewEvaluationSection->TrackIndexInParentSession = RowIndex;
	NewEvaluationSection->ParentMovieScene = GetTypedOuter<UMovieScene>();

	const auto& SongsMap = DAWData->HarmonixMidiFile->GetSongMaps();
	//add all notes to the section's NoteEvents
	//for (const FMidiNoteEvent& NoteEvent : DAWData->NoteEvents)

	/*
	TArray<FFrameNumber> NoteTimes;
	TArray<int> NotePitches;
	for (const auto& Note : DAWData->LinkedNoteDataMap[RowIndex].LinkedNotes)
	{
		const auto& NoteTime = SongsMap->TickToMs(Note.StartTick) * .001f;
		const auto NoteFrameTime = FFrameTime(NoteTime * FrameRate);

		NoteTimes.Add(NoteFrameTime.FrameNumber);
		NotePitches.Add(Note.Pitch);
	}
	NewEvaluationSection->MidiNotes.AddKeys(NoteTimes, NotePitches);
	*/
	
	for (int i = 0; i < DAWData->LinkedNoteDataMap.Num(); i++)
	{
		TArray<int> NotePitchesChannels;
		TArray<FFrameNumber> NoteTimesChannels;
		NewEvaluationSection->MidiNoteChannels.Add(FMovieSceneIntegerChannel());
		auto& LastChannel = NewEvaluationSection->MidiNoteChannels.Last();
		for (const auto& Note : DAWData->LinkedNoteDataMap[i].LinkedNotes)
		{
			const auto& NoteTime = SongsMap->TickToMs(Note.StartTick) * .001f;
			const auto NoteFrameTime = FFrameTime(NoteTime * FrameRate);

			NoteTimesChannels.Add(NoteFrameTime.FrameNumber);
			NotePitchesChannels.Add(Note.Pitch);


		}
		LastChannel.AddKeys(NoteTimesChannels, NotePitchesChannels);
	
	}
	

	//NewEvaluationSection->MidiNotes.
	DAWSections.Add(NewEvaluationSection);

	return NewEvaluationSection;
}

UMovieSceneSection* UUndawSequenceMovieSceneTrack::CreateNewSection()
{
	return NewObject<UUndawMidiMovieSceneTrackSection>(this, NAME_None, RF_Transactional);
}

void UUndawSequenceMovieSceneTrack::AddSection(UMovieSceneSection& Section)
{
	DAWSections.Add(&Section);
}

bool UUndawSequenceMovieSceneTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	if (SectionClass == UUndawMidiMovieSceneTrackSection::StaticClass())
		return true;


	return false;
}

bool UUndawSequenceMovieSceneTrack::HasSection(const UMovieSceneSection& Section) const
{
	return DAWSections.Contains(&Section);
}

bool UUndawSequenceMovieSceneTrack::IsEmpty() const
{
	return DAWSections.Num() == 0;
}


void UUndawSequenceMovieSceneTrack::RemoveSection(UMovieSceneSection& Section)
{
	DAWSections.Remove(&Section);

}

void UUndawSequenceMovieSceneTrack::RemoveSectionAt(int32 SectionIndex)
{
	DAWSections.RemoveAt(SectionIndex);
	
}

const TArray<UMovieSceneSection*>& UUndawSequenceMovieSceneTrack::GetAllSections() const
{
	return DAWSections;
}

bool UUndawSequenceMovieSceneTrack::SupportsMultipleRows() const
{
	return true;
}




