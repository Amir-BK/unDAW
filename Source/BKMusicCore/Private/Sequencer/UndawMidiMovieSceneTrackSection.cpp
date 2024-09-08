#include "Sequencer/UndawMidiMovieSceneTrackSection.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "LevelSequencePlayer.h"
//#include "MidiBroadcasters/MidiBroadcasterPlayHead.h"
//#include "Tracks/MidiAssetSceneTrack.h"


void UUndawMidiMovieSceneTrackSection::MarkBars()
{
	UMovieScene* MovieScene = GetTypedOuter<UMovieScene>();

	FFrameRate FrameRate = MovieScene->GetTickResolution();

	FFrameTime SectionStartTime = GetInclusiveStartFrame();
	FFrameTime SectionEndTime = GetExclusiveEndFrame();

	const auto& BarMap = DAWData->HarmonixMidiFile->GetSongMaps()->GetBarMap();
	const float FirstTickOfFirstBar = BarMap.MusicTimestampBarToTick(0);
	const float LastTickOfLastBar = DAWData->HarmonixMidiFile->GetLastEventTick();

	MovieScene->DeleteMarkedFrames();

	float BarTick = 0;
	int i = 0;
	while (BarTick <= LastTickOfLastBar)
	{
		const auto& BarTime = DAWData->HarmonixMidiFile->GetSongMaps()->TickToMs(BarTick);
		FFrameTime BarFrameTime = FFrameTime(FrameRate.AsFrameTime(BarTime * .001f));


		auto MarkedFrameTest = FMovieSceneMarkedFrame(FFrameNumber(BarFrameTime.FrameNumber));
		MarkedFrameTest.Label = FString::Printf(TEXT("Bar %d"), ++i);
		MarkedFrameTest.Color = FLinearColor::Green;
		MovieScene->AddMarkedFrame(MarkedFrameTest);

		BarTick += DAWData->HarmonixMidiFile->GetSongMaps()->SubdivisionToMidiTicks(EMidiClockSubdivisionQuantization::Bar, BarTick);
	}

}

void UUndawMidiMovieSceneTrackSection::MarkSubdivisionsInRange()
{
	UMovieScene* MovieScene = GetTypedOuter<UMovieScene>();

	FFrameRate FrameRate = MovieScene->GetTickResolution();

	//FFrameTime SectionStartTime = GetInclusiveStartFrame();
	//FFrameTime SectionEndTime = GetExclusiveEndFrame();

	const auto& BarMap = DAWData->HarmonixMidiFile->GetSongMaps()->GetBarMap();
	const float FirstTickOfFirstBar = BarMap.MusicTimestampBarToTick(0);
	const float LastTickOfLastBar = DAWData->HarmonixMidiFile->GetLastEventTick();


	auto SelectionRange = MovieScene->GetSelectionRange();

	FFrameTime SectionStartTime = SelectionRange.GetLowerBoundValue();
	FFrameTime SectionEndTime = SelectionRange.GetUpperBoundValue();
	

	MovieScene->DeleteMarkedFrames();

	auto RangeStartSeconds = FrameRate.AsSeconds(SectionStartTime);
	auto RangeEndSeconds = FrameRate.AsSeconds(SectionEndTime);

	UE_LOG(LogTemp, Warning, TEXT("RangeStartSeconds %f, RangeEndSeconds %f"), RangeStartSeconds, RangeEndSeconds);

	float FirstTickInSelectionRange = DAWData->HarmonixMidiFile->GetSongMaps()->MsToTick(RangeStartSeconds * 1000);
	float LastTickInSelectionRange = DAWData->HarmonixMidiFile->GetSongMaps()->MsToTick(RangeEndSeconds * 1000);

	UE_LOG(LogTemp, Warning, TEXT("FirstTickInSelectionRange %f, LastTickInSelectionRange %f"), FirstTickInSelectionRange, LastTickInSelectionRange);

	const auto& SongsMap = DAWData->HarmonixMidiFile->GetSongMaps();

	float FirstSubdivisionInSelectionRange = SongsMap->QuantizeTickToNearestSubdivision(FMath::FloorToInt32(FirstTickInSelectionRange), EMidiFileQuantizeDirection::Up, MusicSubdivision);
	float LastSubdivisionInSelectionRange = SongsMap->QuantizeTickToNearestSubdivision(FMath::FloorToInt32(LastTickInSelectionRange), EMidiFileQuantizeDirection::Down, MusicSubdivision);
		//DAWSequencerData->HarmonixMidiFile->GetSongMaps()->SubdivisionToMidiTicks(MusicSubdivision, FMath::FloorToInt32(LastTickInSelectionRange));

	UE_LOG(LogTemp, Warning, TEXT("FirstSubdivisionInSelectionRange %f, LastSubdivisionInSelectionRange %f"), FirstSubdivisionInSelectionRange, LastSubdivisionInSelectionRange);

	while (FirstSubdivisionInSelectionRange <= LastSubdivisionInSelectionRange)
	{
	
		const auto& SubdivisionTime = DAWData->HarmonixMidiFile->GetSongMaps()->TickToMs(FirstSubdivisionInSelectionRange);
		FFrameTime SubdivisionFrameTime = FFrameTime(FrameRate.AsFrameTime(SubdivisionTime * .001f));

		auto MarkedFrameTest = FMovieSceneMarkedFrame(FFrameNumber(SubdivisionFrameTime.FrameNumber));
		MarkedFrameTest.Label = FString::Printf(TEXT("Subdivision %d"), FirstSubdivisionInSelectionRange);
		MarkedFrameTest.Color = FLinearColor::Gray;
		MovieScene->AddMarkedFrame(MarkedFrameTest);

		FirstSubdivisionInSelectionRange += DAWData->HarmonixMidiFile->GetSongMaps()->SubdivisionToMidiTicks(MusicSubdivision, FirstSubdivisionInSelectionRange);
	
	}

	////float FirstTickOfSubdivision = DAWSequencerData->HarmonixMidiFile->GetSongMaps()->Ca

	//float BarTick = DAWSequencerData->HarmonixMidiFile->GetSongMaps()->MsToTick(SectionStartTime.AsDecimal());
	//float BarEndTick = DAWSequencerData->HarmonixMidiFile->GetSongMaps()->MsToTick(SectionEndTime.AsDecimal());

	////float BarTick = DAWSequencerData->HarmonixMidiFile->GetSongMaps()->SubdivisionToMidiTicks(MusicSubdivision, SectionStartTime.AsDecimal());;
	////float BarEndTick = DAWSequencerData->HarmonixMidiFile->GetSongMaps()->SubdivisionToMidiTicks(MusicSubdivision, SectionEndTime.AsDecimal());;

	//while (BarTick <= BarEndTick)
	//{
	//	const auto& BarTime = DAWSequencerData->HarmonixMidiFile->GetSongMaps()->TickToMs(BarTick);
	//	FFrameTime BarFrameTime = FFrameTime(FrameRate.AsFrameTime(BarTime * .001f));


	//	auto MarkedFrameTest = FMovieSceneMarkedFrame(FFrameNumber(BarFrameTime.FrameNumber));
	//	//MarkedFrameTest.Label = FString::Printf(TEXT("Bar %d"), ++i);
	//	MarkedFrameTest.Color = FLinearColor::Gray;
	//	MovieScene->AddMarkedFrame(MarkedFrameTest);

	//	BarTick += DAWSequencerData->HarmonixMidiFile->GetSongMaps()->SubdivisionToMidiTicks(MusicSubdivision, BarTick);
	//}
}

void UUndawMidiMovieSceneTrackSection::MarkNotesInRange()
{
	UMovieScene* MovieScene = GetTypedOuter<UMovieScene>();

	FFrameRate FrameRate = MovieScene->GetTickResolution();

	auto SelectionRange = MovieScene->GetSelectionRange();

	FFrameTime SectionStartTime = SelectionRange.GetLowerBoundValue();
	FFrameTime SectionEndTime = SelectionRange.GetUpperBoundValue();

	const auto& BarMap = DAWData->HarmonixMidiFile->GetSongMaps()->GetBarMap();

	MovieScene->DeleteMarkedFrames();

	auto LinkedNotesTracks = DAWData->LinkedNoteDataMap;
	const auto& Track = LinkedNotesTracks[TrackIndexInParentSession];
	auto TrackColor = DAWData->GetTrackMetadata(TrackIndexInParentSession).TrackColor;

	for (const auto& Note : Track.LinkedNotes)
	{
		const auto& NoteTime = DAWData->HarmonixMidiFile->GetSongMaps()->TickToMs(Note.StartTick);

		FFrameTime NoteFrameTime = FFrameTime(FrameRate.AsFrameTime(NoteTime * .001f));

		if (NoteFrameTime >= SectionStartTime && NoteFrameTime <= SectionEndTime)
		{
			auto MarkedFrameTest = FMovieSceneMarkedFrame(FFrameNumber(NoteFrameTime.FrameNumber));
			MarkedFrameTest.Comment = FString::Printf(TEXT("Note %d"), Note.Pitch);
			MarkedFrameTest.Color = TrackColor;
			MovieScene->AddMarkedFrame(MarkedFrameTest);
		}
	}
}

void UUndawMidiMovieSceneTrackSection::RebuildNoteKeyFrames()
{
	//AddKeyToChannel(SoundVolume, 0, 0.5f);
	//MidiNoteChannels.Empty();

	if (CanModify())
	{
		Modify();
		const auto& SongsMap = DAWData->HarmonixMidiFile->GetSongMaps();
		FFrameRate FrameRate = ParentMovieScene->GetTickResolution();
		for (int i = 0; i < DAWData->LinkedNoteDataMap.Num(); i++)
		{
			TArray<int> NotePitchesChannels;
			TArray<FFrameNumber> NoteTimesChannels;
			//MidiNoteChannels.Add(FMovieSceneIntegerChannel());
			MidiNoteChannels[i].Reset();
			auto& NoteChannel = MidiNoteChannels[i];
			for (const auto& Note : DAWData->LinkedNoteDataMap[i].LinkedNotes)
			{
				const auto& NoteTime = SongsMap->TickToMs(Note.StartTick) * .001f;
				const auto NoteFrameTime = FFrameTime(NoteTime * FrameRate);

				NoteTimesChannels.Add(NoteFrameTime.FrameNumber);
				NotePitchesChannels.Add(Note.Pitch);


			}
			NoteChannel.AddKeys(NoteTimesChannels, NotePitchesChannels);

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

	if (IsValid(DAWData))
	{
		for (int i = 0; i < MidiNoteChannels.Num(); i++)	
		{
			//FMovieSceneChannelMetaData MetaData;
			MetaData.Name = FName(FString::Printf(TEXT("%s %d"), *DAWData->GetTrackMetadata(i).TrackName, i));
			MetaData.DisplayText = FText::FromString(FString::Printf(TEXT("%s %d"), *DAWData->GetTrackMetadata(i).TrackName, i));
			MetaData.Color = DAWData->GetTrackMetadata(i).TrackColor;

			Channels.Add(MidiNoteChannels[i], MetaData, TMovieSceneExternalValue<int>());
		}

	}
	
	
	ChannelProxy = MakeShared<FMovieSceneChannelProxy>(MoveTemp(Channels));
	return EMovieSceneChannelProxyType::Dynamic;
}



#if WITH_EDITOR
void UUndawMidiMovieSceneTrackSection::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName=PropertyChangedEvent.GetPropertyName();

	if (PropertyName == TEXT("PlayRate"))
	{
		UE_LOG(LogTemp, Warning, TEXT("Playrate changed"));

	}

	UE_LOG(LogTemp, Warning, TEXT("Property Changed %s"), *PropertyName.ToString());

	
	
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
