
#pragma once

#include "CoreMinimal.h"
#include "Channels/MovieSceneAudioTriggerChannel.h"
#include "Channels/MovieSceneBoolChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"
#include "Channels/MovieSceneStringChannel.h"
//#include "MidiBroadcasters/MidiBroadcaster.h"
//#include "MidiBroadcasters/MidiBroadcasterPlayHead.h"
//#include "MidiObjects/MidiAsset.h"
#include "Sections/MovieSceneHookSection.h"
#include "UndawMidiMovieSceneTrackSection.generated.h"


class UUndawSequenceMovieSceneTrack;
class UDAWSequencerData;


UCLASS()
class BKMUSICCORE_API UUndawMidiMovieSceneTrackSection : public UMovieSceneSection//, public IMovieSceneEntityProvider //, public IMidiBroadcaster

{
	GENERATED_BODY()

protected:

	UPROPERTY()
	UUndawMidiMovieSceneTrackSection* This = nullptr;

	UPROPERTY(EditAnywhere, Category = "Midi", BlueprintReadWrite)
	float PlayRate = 1.0f;



	double SectionLocalCurrentTime=0;


public:

	UPROPERTY()
	FMovieSceneFloatChannel SoundVolume;

	UPROPERTY()
	TObjectPtr<UDAWSequencerData> DAWSequencerData;

	UPROPERTY(EditAnywhere, Category = "MidiBroadcaster")
	FString BroadcasterName = "SequencerMidiBroadcaster";

	
	UPROPERTY(EditAnywhere, Category = "MidiBroadcaster")
	TMap<FString, float> CustomPlayHeads;


	
public:
	UUndawMidiMovieSceneTrackSection(const FObjectInitializer& ObjInit);


	virtual TOptional<TRange<FFrameNumber> > GetAutoSizeRange() const override { return TRange<FFrameNumber>::Empty(); }
	virtual void TrimSection(FQualifiedFrameTime TrimTime, bool bTrimLeft, bool bDeleteKeys) override {}
	virtual UMovieSceneSection* SplitSection(FQualifiedFrameTime SplitTime, bool bDeleteKeys) override { return nullptr; }
	virtual TOptional<FFrameTime> GetOffsetTime() const override;
	virtual void MigrateFrameTimes(FFrameRate SourceRate, FFrameRate DestinationRate) override {  };
	//virtual void PostEditImport() override 
	virtual EMovieSceneChannelProxyType CacheChannelProxy() override;




	
protected:



#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif


public:




private:

	double GetPlayerTimeAsSeconds(const UE::MovieScene::FEvaluationHookParams& Params) const;


	double UpdateSectionTime(const UE::MovieScene::FEvaluationHookParams& Params) const;


	double GetSectionTimeAhead(const UE::MovieScene::FEvaluationHookParams& Params) const;

	double GetPlayerTimeAhead(const UE::MovieScene::FEvaluationHookParams& Params) const;


	double GetDeltaTimeAsSeconds(const UE::MovieScene::FEvaluationHookParams& Params) const;
};
