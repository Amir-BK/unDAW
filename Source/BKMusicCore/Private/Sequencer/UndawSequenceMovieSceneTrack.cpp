#include "Sequencer/UndawSequenceMovieSceneTrack.h"

#include "MovieScene.h"
//#include "MidiObjects/MidiTrack.h"
#include "Sequencer/UndawMidiMovieSceneTrackSection.h"


UMovieSceneSection* UUndawSequenceMovieSceneTrack::AddNewDAWDataOnRow(UDAWSequencerData* DAWData, FFrameNumber Time, int32 RowIndex)
{
	
	check(DAWData);

	FFrameRate FrameRate = GetTypedOuter<UMovieScene>()->GetTickResolution();
	FFrameTime DurationToUse = 1.f * FrameRate; // if all else fails, use 1 second duration


	const float Duration = DAWData->SequenceDuration;

	DurationToUse = Duration * FrameRate;

	//add the section
	UUndawMidiMovieSceneTrackSection* NewEvaluationSection = Cast<UUndawMidiMovieSceneTrackSection>(CreateNewSection());
	NewEvaluationSection->InitialPlacementOnRow(DAWSections, Time, DurationToUse.FrameNumber.Value, RowIndex);
	NewEvaluationSection->DAWSequencerData = DAWData;

	DAWSections.Add(NewEvaluationSection);

	return NewEvaluationSection;
}

UMovieSceneSection* UUndawSequenceMovieSceneTrack::CreateNewSection()
{
	return NewObject<UUndawMidiMovieSceneTrackSection>(this, NAME_None, RF_Transactional);
}

void UUndawSequenceMovieSceneTrack::AddSection(UMovieSceneSection& Section)
{
	DAWSections.Add(Cast<UUndawMidiMovieSceneTrackSection>(&Section));
}

bool UUndawSequenceMovieSceneTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	if (SectionClass == UUndawMidiMovieSceneTrackSection::StaticClass())
		return true;

	//DEPRECATED
	//if(SectionClass == UMidiSceneSection::StaticClass())
		//return true;

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




