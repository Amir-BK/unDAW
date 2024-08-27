
#pragma once

#include "CoreMinimal.h"
#include "Channels/MovieSceneAudioTriggerChannel.h"
#include "Channels/MovieSceneChannelEditorData.h"
#include "Channels/MovieSceneCurveChannelCommon.h"
#include "Channels/MovieSceneBoolChannel.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Channels/MovieSceneIntegerChannel.h"
#include "Channels/MovieSceneStringChannel.h"
//#include "MidiBroadcasters/MidiBroadcaster.h"
//#include "MidiBroadcasters/MidiBroadcasterPlayHead.h"
//#include "MidiObjects/MidiAsset.h"
#include "Sections/MovieSceneHookSection.h"
#include "M2SoundGraphData.h"
#include "UndawMidiMovieSceneTrackSection.generated.h"


class UUndawSequenceMovieSceneTrack;
class UDAWSequencerData;

template<>
struct TMovieSceneExternalValue<FLinkedMidiEvents>
{

};

USTRUCT()
struct FMovieSceneMidiTrackChannel : public FMovieSceneChannel
{
	GENERATED_BODY()

	typedef FLinkedMidiEvents CurveValueType;
	typedef FLinkedMidiEvents ChannelValueType;

	friend struct TMovieSceneChannelTraits<FMovieSceneMidiTrackChannel>;
	using FMovieSceneMidiTrackChannelImp = TMovieSceneCurveChannelImpl<FMovieSceneMidiTrackChannel>;

	bool SerializeFromMismatchedTag(const FPropertyTag& Tag, FStructuredArchive::FSlot Slot) {return false; }
	bool Serialize(FArchive& Ar);
#if WITH_EDITORONLY_DATA
	void PostSerialize(const FArchive& Ar);
#endif

};

template<>
struct TStructOpsTypeTraits<FMovieSceneMidiTrackChannel> : public TStructOpsTypeTraitsBase2<FMovieSceneMidiTrackChannel>
{
	enum
	{
		WithStructuredSerializeFromMismatchedTag = true,
		WithSerializer = true,
#if WITH_EDITORONLY_DATA
		WithPostSerialize = true,
#endif
	};
	static constexpr EPropertyObjectReferenceType WithSerializerObjectReferences = EPropertyObjectReferenceType::None;

};

template<>
struct TMovieSceneChannelTraits<FMovieSceneMidiTrackChannel> : TMovieSceneChannelTraitsBase<FMovieSceneMidiTrackChannel>
{
#if WITH_EDITOR

	/** Float channels can have external values (ie, they can get their values from external objects for UI purposes) */
	typedef TMovieSceneExternalValue<FLinkedMidiEvents> ExtendedEditorDataType;

#endif
};

UCLASS()
class BKMUSICCORE_API UUndawMidiMovieSceneTrackSection : public UMovieSceneSection//, public IMovieSceneEntityProvider //, public IMidiBroadcaster

{
	GENERATED_BODY()

protected:

	UFUNCTION(CallInEditor, Category = "Midi")
	void CreateMarksOnBars();

	UFUNCTION(CallInEditor, Category = "Midi")
	void CreateMarksOnBeats();

	UFUNCTION(CallInEditor, Category = "Midi")
	void CreateMarksForNotesInRange();

	UPROPERTY()
	UUndawMidiMovieSceneTrackSection* This = nullptr;

	UPROPERTY(EditAnywhere, Category = "Midi", BlueprintReadWrite)
	float PlayRate = 1.0f;



	double SectionLocalCurrentTime=0;


public:

	UPROPERTY()
	int TrackIndexInParentSession = INDEX_NONE;

	UPROPERTY()
	FMovieSceneFloatChannel SoundVolume;

	UPROPERTY()
	FMovieSceneFloatChannel PitchBend;

	UPROPERTY()
	TObjectPtr<UDAWSequencerData> DAWSequencerData;

	UPROPERTY()
	FMovieSceneMidiTrackChannel MidiNotes;

	UPROPERTY(EditAnywhere, Category = "unDAW")
	FString TrackName = "Midi Track Name";




	
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
