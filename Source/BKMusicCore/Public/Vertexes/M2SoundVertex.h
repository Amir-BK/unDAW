// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Metasound.h"
#include "MetasoundBuilderSubsystem.h"
#include "M2SoundGraphData.h"



#include "M2SoundVertex.generated.h"

BKMUSICCORE_API DECLARE_LOG_CATEGORY_EXTERN(unDAWVertexLogs, Verbose, All);

//class UM2SoundGraphData;
class UM2SoundGraphRenderer;
struct FBuilderVertexCompositeData;

UENUM()
enum class EVertexAutoConnectionPinCategory :  uint8
{
	MidiTrackStream, //pairs a midistream with a tracknum int - really we wouldn't need the tracknum but the fusion sampler requires it, other custom metasound instruments don't need it as the stream is already filtered for chan/track
	MidiTrackTrackNum, //pairs a tracknum with a tracknum int - used to filter the midi stream for a specific track
	AudioStreamL, //right now this is just a convenient wrapper for two audio channels, in the future we'd like to use this to allow multichannel audio
	AudioStreamR,
};

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EM2SoundPinFlags : uint8
{
	None = 0 UMETA(Hidden),
	IsAutoManaged = 1 << 0,
	IsConnectedToGraphParam = 1 << 1,
	//ShowInBoth = ShowInGraph | ShowInDetails,
};
ENUM_CLASS_FLAGS(EM2SoundPinFlags)

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EM2SoundPinDisplayFlags : uint8
{
	None = 0 UMETA(Hidden),
	ShowInGraph = 1 << 0,
	ShowInGame = 1 << 1,
};
ENUM_CLASS_FLAGS(EM2SoundPinDisplayFlags)

USTRUCT(BlueprintType)
struct FM2SoundPinData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere)
	float MaxValue = 1.0f;

	//to init the slider correctly
	UPROPERTY(VisibleAnywhere)
	float NormalizedValue = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FName PinName;

	//the underlying metasound datatype
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FName DataType;

	UPROPERTY(VisibleAnywhere, meta = (Bitmask, BitmaskEnum = "/Script/BKMusicCore.EM2SoundPinFlags"))
	uint8 PinFlags;

	UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = "/Script/BKMusicCore.EM2SoundPinDisplayFlags"))
	uint8 DisplayFlags;

	UPROPERTY()
	FMetasoundFrontendLiteral LiteralValue;

	FMetaSoundBuilderNodeInputHandle InputHandle;

};

UCLASS(Abstract, AutoExpandCategories = ("M2Sound"))
class BKMUSICCORE_API UM2SoundVertex : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "M2Sound", meta=(EditFixedSize))
	TMap<FName, FM2SoundPinData> InPinsNew;

	UPROPERTY(EditAnywhere, Category = "M2Sound", meta = (EditFixedSize))
	TMap<FName, FM2SoundPinData> OutPinsNew;


	

	//for book keeping and tracking the color of the track in the sequencer, can also be used to assosciate groups of vertexes for the MIXER (that is to come)
	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	int TrackId = INDEX_NONE;

	//as we only allow ony input and multiple outputs, we can use this function to break the connection between the input and the output
	void BreakTrackInputConnection();

	void MakeTrackInputConnection(UM2SoundVertex* InputVertex);

	void BreakTrackOutputConnection(UM2SoundVertex* OutputVertex);

	void UpdateValueForPin(FM2SoundPinData& Pin, FMetasoundFrontendLiteral& NewValue);

	UPROPERTY(BlueprintAssignable, Category = "M2Sound")
	FOnVertexUpdated OnVertexUpdated;

	void RegisterOutputVertex(UM2SoundVertex* OutputVertex);

	bool UnregisterOutputVertex(UM2SoundVertex* OutputVertex);

	//main input represents the 'track' binding, used to make auto connections to the data from the midi file and allow us to keep
	// track of the flow of controls and assignments within the actual metasound graph, generally, the 'Track' pins represent
	// metasound i/os that are exposed via our metasound parameter interfaces
	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	UM2SoundVertex* MainInput;

	//Probably should only allow a single input (multi outputs), these are not the proper node i/os but rather the 'track' binding.
	//UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	//TArray<UM2SoundVertex*> Inputs;

	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	TSet<UM2SoundVertex*> Outputs;

	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	TMap<EVertexAutoConnectionPinCategory, FMetaSoundBuilderNodeOutputHandle> AutoConnectOutPins;

	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	TMap<EVertexAutoConnectionPinCategory, FMetaSoundBuilderNodeInputHandle> AutoConnectInPins;

	UPROPERTY()
	TArray<FMetaSoundBuilderNodeOutputHandle> OutPins;

	UPROPERTY()
	TArray<FMetaSoundBuilderNodeInputHandle> InPins;


	UFUNCTION()
	UDAWSequencerData* GetSequencerData() const;

	UPROPERTY()
	TObjectPtr<UDAWSequencerData> SequencerData;


	UPROPERTY()
	FString VertexErrors;

	UPROPERTY()
	bool bHasErrors = false;

	UPROPERTY()
	int32 ErrorType = EMessageSeverity::Info;

	UPROPERTY()
	TArray<FM2SoundMetasoundBuilderPinData> MetasoundInputs;

	UPROPERTY()
	TArray<FM2SoundMetasoundBuilderPinData> MetasoundOutputs;

	UFUNCTION()
	virtual void VertexNeedsBuilderUpdates();

	UFUNCTION()
	virtual void VertexConnectionsChanged();

	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	TMap<FName, EMetaSoundBuilderResult> BuilderResults;

	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	TMap<FName, EMetaSoundBuilderResult> BuilderConnectionResults;


	void TransmitAudioParameter(FAudioParameter Parameter);

	virtual void CollectAndTransmitAudioParameters() {}


	bool bIsAudioOutputVertex = false;

	//This is a key method, should be renamed and probably refactored into being called automatically via a pure virtual method on the
	//patch vertex base class
	void CollectParamsForAutoConnect();

	UPROPERTY()
	bool bHideInGraph = false;


	virtual void BuildVertex() {};

	virtual void UpdateConnections() {};

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	#endif

};

UCLASS()
class BKMUSICCORE_API UM2SoundCompositeVertex : public UM2SoundVertex
{
	GENERATED_BODY()


};

UCLASS(Abstract)
//this is a literal node that can be used to pass a value to the metasound graph
class BKMUSICCORE_API UM2SoundLiteralNodeVertex : public UM2SoundVertex
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FMetaSoundNodeHandle NodeHandle;
};

//the audio output is a bit of a special case due to the way the output graph works, we need to add another channel to the mixer for each one
// for now it can remain as a separate node, we'll perhaps expand it once we support sends

//to start making sense of things here, the audio output is NOT a literal node handle - why? because it represents a just some output pins and not a complete node, this is why we need to create a separate class for it
UCLASS()
class BKMUSICCORE_API UM2SoundAudioOutput : public UM2SoundVertex
{
	GENERATED_BODY()

public:

	UM2SoundAudioOutput() { bIsAudioOutputVertex = true; }

	UPROPERTY(EditAnywhere, Category = "M2Sound")
	float Gain = 1.0f;

	UPROPERTY(VisibleAnywhere)
	FName GainParameterName = NAME_None;

	UPROPERTY(VisibleAnywhere)
	FAssignableAudioOutput AudioOutput;

	// Inherited via UM2SoundVertex

	void BuildVertex() override;

	void UpdateConnections() override;

	void CollectAndTransmitAudioParameters() override;
	

};

//All vertex that from the perspective of the metasound builder can be reduced to a single FMetaSoundBuilderNodeOutputHandle - such as the midi input vertexes
// these do not expose the actual metasound node, but rather the output handle that can be used to connect to other nodes - this abtracts the midifilters and other similar nodes from the m2sound graph 
// by our paradigm we probably don't need to implement 'update connections' for these, but it might be useful for the 'expose to outputs' function. 
UCLASS()

class BKMUSICCORE_API UM2SoundBuilderInputHandleNode : public UM2SoundVertex
{
	GENERATED_BODY()

	//virtual FString GetUniqueParameterName() = ;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "M2Sound")
	bool bOutputToBlueprints = true;

	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	FString TrackPrefix;

	// Inherited via UM2SoundVertex

	void BuildVertex() override;
};

UCLASS()
class BKMUSICCORE_API UM2SoundMidiInputVertex : public UM2SoundBuilderInputHandleNode
{
	GENERATED_BODY()

public:

	//Represents the index of the track in the sequencer data, to get the actual midi metadata use this index to get the track from the sequencer data



};

//vertex container for user created metasound patch assets, of course this one is a literal node.
UCLASS()
class BKMUSICCORE_API UM2SoundPatch : public UM2SoundLiteralNodeVertex
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, Category = "M2Sound")
	UMetaSoundPatch* Patch;

	UFUNCTION(CallInEditor, Category = "M2Sound")
	void SaveDefaultsToVertexCache();

	//for now can be used to rebuild the node manually if the user changes the metasound patch
	bool bForceRebuild = false;


	// Inherited via UM2SoundVertex

	void BuildVertex() override;

	void UpdateConnections() override;

};
