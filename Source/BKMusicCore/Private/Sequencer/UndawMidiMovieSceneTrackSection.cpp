#include "Sequencer/UndawMidiMovieSceneTrackSection.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "LevelSequencePlayer.h"
//#include "MidiBroadcasters/MidiBroadcasterPlayHead.h"
//#include "Tracks/MidiAssetSceneTrack.h"


void UUndawMidiMovieSceneTrackSection::CreateMarksOnBars()
{
	UMovieScene* MovieScene = GetTypedOuter<UMovieScene>();

	FFrameRate FrameRate = MovieScene->GetTickResolution();

	FFrameTime SectionStartTime = GetInclusiveStartFrame();
	FFrameTime SectionEndTime = GetExclusiveEndFrame();

	const auto& BarMap = DAWSequencerData->HarmonixMidiFile->GetSongMaps()->GetBarMap();
	const float FirstTickOfFirstBar = BarMap.MusicTimestampBarToTick(0);
	const float LastTickOfLastBar = DAWSequencerData->HarmonixMidiFile->GetLastEventTick();

	MovieScene->DeleteMarkedFrames();

	float BarTick = 0;
	int i = 0;
	while (BarTick <= LastTickOfLastBar)
	{
		const auto& BarTime = DAWSequencerData->HarmonixMidiFile->GetSongMaps()->TickToMs(BarTick);
		FFrameTime BarFrameTime = FFrameTime(FrameRate.AsFrameTime(BarTime * .001f));


		auto MarkedFrameTest = FMovieSceneMarkedFrame(FFrameNumber(BarFrameTime.FrameNumber));
		MarkedFrameTest.Label = FString::Printf(TEXT("Bar %d"), ++i);
		MarkedFrameTest.Color = FLinearColor::Green;
		MovieScene->AddMarkedFrame(MarkedFrameTest);

		BarTick += DAWSequencerData->HarmonixMidiFile->GetSongMaps()->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Bar, BarTick);
	}

}

void UUndawMidiMovieSceneTrackSection::CreateMarksOnBeats()
{
	UMovieScene* MovieScene = GetTypedOuter<UMovieScene>();

	FFrameRate FrameRate = MovieScene->GetTickResolution();

	FFrameTime SectionStartTime = GetInclusiveStartFrame();
	FFrameTime SectionEndTime = GetExclusiveEndFrame();

	const auto& BarMap = DAWSequencerData->HarmonixMidiFile->GetSongMaps()->GetBarMap();
	const float FirstTickOfFirstBar = BarMap.MusicTimestampBarToTick(0);
	const float LastTickOfLastBar = DAWSequencerData->HarmonixMidiFile->GetLastEventTick();

	MovieScene->DeleteMarkedFrames();

	float BarTick = 0;

	while (BarTick <= LastTickOfLastBar)
	{
		const auto& BarTime = DAWSequencerData->HarmonixMidiFile->GetSongMaps()->TickToMs(BarTick);
		FFrameTime BarFrameTime = FFrameTime(FrameRate.AsFrameTime(BarTime * .001f));


		auto MarkedFrameTest = FMovieSceneMarkedFrame(FFrameNumber(BarFrameTime.FrameNumber));
		//MarkedFrameTest.Label = FString::Printf(TEXT("Bar %d"), ++i);
		MarkedFrameTest.Color = FLinearColor::Gray;
		MovieScene->AddMarkedFrame(MarkedFrameTest);

		BarTick += DAWSequencerData->HarmonixMidiFile->GetSongMaps()->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Beat, BarTick);
	}
}

void UUndawMidiMovieSceneTrackSection::CreateMarksForNotesInRange()
{
	UMovieScene* MovieScene = GetTypedOuter<UMovieScene>();

	FFrameRate FrameRate = MovieScene->GetTickResolution();

	FFrameTime SectionStartTime = GetInclusiveStartFrame();
	FFrameTime SectionEndTime = GetExclusiveEndFrame();

	const auto& BarMap = DAWSequencerData->HarmonixMidiFile->GetSongMaps()->GetBarMap();

	MovieScene->DeleteMarkedFrames();

	auto LinkedNotesTracks = DAWSequencerData->LinkedNoteDataMap;
	const auto& Track = LinkedNotesTracks[TrackIndexInParentSession];
	auto TrackColor = DAWSequencerData->GetTracksDisplayOptions(TrackIndexInParentSession).trackColor;

	for (const auto& Note : Track.LinkedNotes)
	{
		const auto& NoteTime = DAWSequencerData->HarmonixMidiFile->GetSongMaps()->TickToMs(Note.StartTick);

		FFrameTime NoteFrameTime = FFrameTime(FrameRate.AsFrameTime(NoteTime * .001f));

		if (NoteFrameTime >= SectionStartTime && NoteFrameTime <= SectionEndTime)
		{
			auto MarkedFrameTest = FMovieSceneMarkedFrame(FFrameNumber(NoteFrameTime.FrameNumber));
			MarkedFrameTest.Label = FString::Printf(TEXT("Note %d"), Note.pitch);
			MarkedFrameTest.Color = TrackColor;
			MovieScene->AddMarkedFrame(MarkedFrameTest);
		}
	}
}

UUndawMidiMovieSceneTrackSection::UUndawMidiMovieSceneTrackSection(const FObjectInitializer& ObjInit) : Super(ObjInit)
{
	//bRequiresRangedHook = true;
	//bRequiresTriggerHooks = true;
	EvalOptions.CompletionMode = EMovieSceneCompletionMode::RestoreState;

	//Remove const limitations.
	This = this;

}

TOptional<FFrameTime> UUndawMidiMovieSceneTrackSection::GetOffsetTime() const
{
	return TOptional<FFrameTime>();
}

EMovieSceneChannelProxyType UUndawMidiMovieSceneTrackSection::CacheChannelProxy()
{
	FMovieSceneChannelProxyData Channels;

	UUndawMidiMovieSceneTrackSection* MutableThis = const_cast<UUndawMidiMovieSceneTrackSection*>(this);
	UMovieScene* MovieScene = MutableThis->GetTypedOuter<UMovieScene>();

	FMovieSceneChannelMetaData MetaData;
	MetaData.Name = FName("SoundVolume");
	MetaData.DisplayText = NSLOCTEXT("MovieScene", "SoundVolume", "Sound Volume");
	Channels.Add(SoundVolume, MetaData, TMovieSceneExternalValue<float>());

	MetaData.Name = FName("PitchBend");
	MetaData.DisplayText = NSLOCTEXT("MovieScene", "PitchBend", "Pitch Bend");
	Channels.Add(PitchBend, MetaData, TMovieSceneExternalValue<float>());

	//MetaData.Name = FName("MidiNotes");
	//MetaData.DisplayText = NSLOCTEXT("MovieScene", "MidiNotes", "Midi Notes");
	//Channels.Add(MidiNotes, MetaData, TMovieSceneExternalValue<FLinkedMidiEvents>());
	
	ChannelProxy = MakeShared<FMovieSceneChannelProxy>(MoveTemp(Channels));
	return EMovieSceneChannelProxyType::Dynamic;
}



#if WITH_EDITOR
void UUndawMidiMovieSceneTrackSection::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName=PropertyChangedEvent.GetPropertyName();

	
	
}
#endif



double UUndawMidiMovieSceneTrackSection::GetPlayerTimeAsSeconds(const UE::MovieScene::FEvaluationHookParams& Params) const
{
	FFrameTime CurrentFrameTime = Params.Context.GetTime();

	FFrameRate CurrentFrameRate = Params.Context.GetFrameRate();

	double CurrentTimeAsSeconds = CurrentFrameTime/CurrentFrameRate;

	return CurrentTimeAsSeconds;

}

double UUndawMidiMovieSceneTrackSection::UpdateSectionTime( const UE::MovieScene::FEvaluationHookParams& Params) const
{

	FFrameNumber SectionStartFrameNumber = GetInclusiveStartFrame();

	FFrameTime SectionStartFrameTime(SectionStartFrameNumber);

	FFrameRate CurrentFrameRate = Params.Context.GetFrameRate();

	double SectionStartTimeAsSeconds=SectionStartFrameTime/CurrentFrameRate;

	double PlayerTimeAsSeconds = GetPlayerTimeAsSeconds(Params);

	This->SectionLocalCurrentTime=PlayerTimeAsSeconds-SectionStartTimeAsSeconds;

	return SectionLocalCurrentTime;

}

double UUndawMidiMovieSceneTrackSection::GetSectionTimeAhead(const UE::MovieScene::FEvaluationHookParams& Params) const
{
	//The time between frames according to FPS, testing with 60fps here.
	double DeltaFrameSeconds = 1.0 / 60.0;

	double CurrentSectionTime= UpdateSectionTime(Params);


	double NextFrameTimeAsSeconds = CurrentSectionTime + DeltaFrameSeconds;

	return NextFrameTimeAsSeconds;

}

double UUndawMidiMovieSceneTrackSection::GetPlayerTimeAhead(const UE::MovieScene::FEvaluationHookParams& Params) const
{

	//The time between frames according to FPS, testing with 60fps here.
	double DeltaFrameSeconds=1.0/60.0;


	double CurrentTimeAsSeconds = GetPlayerTimeAsSeconds(Params);


	double NextFrameTimeAsSeconds = CurrentTimeAsSeconds+DeltaFrameSeconds;

	return NextFrameTimeAsSeconds;
}

double UUndawMidiMovieSceneTrackSection::GetDeltaTimeAsSeconds(const UE::MovieScene::FEvaluationHookParams& Params) const
{

	FFrameTime DeltaTime=Params.Context.GetDelta();

	double DeltaSeconds=DeltaTime/Params.Context.GetFrameRate();

	return DeltaSeconds;
}

bool FMovieSceneMidiTrackChannel::Serialize(FArchive& Ar)
{
	return false;
}

void FMovieSceneMidiTrackChannel::PostSerialize(const FArchive& Ar)
{
}
