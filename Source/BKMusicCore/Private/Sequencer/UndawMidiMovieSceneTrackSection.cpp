#include "Sequencer/UndawMidiMovieSceneTrackSection.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "LevelSequencePlayer.h"
//#include "MidiBroadcasters/MidiBroadcasterPlayHead.h"
//#include "Tracks/MidiAssetSceneTrack.h"


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
	MetaData.DisplayText = NSLOCTEXT("MovieScene", "SoundVolume", "Sound Volume");
	Channels.Add(SoundVolume, MetaData, TMovieSceneExternalValue<float>());
	
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

