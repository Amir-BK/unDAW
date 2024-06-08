// Fill out your copyright notice in the Description page of Project Settings.



#pragma once

#include "CoreMinimal.h"
#include "HarmonixMidi/BarMap.h"
#include "HarmonixMidi/MidiFile.h"
#include "Sound/SoundWave.h"
#include "Curves/RichCurve.h"

#include "Curves/RealCurve.h"
#include "Engine/DataAsset.h"
#include "Curves/CurveFloat.h"
#include "EdGraph/EdGraph.h"


#include "Metasound.h"
#include "MetasoundBuilderSubsystem.h"


#include "TrackPlaybackAndDisplayOptions.h"

#include "SequencerData.generated.h"


BKMUSICCORE_API DECLARE_LOG_CATEGORY_EXTERN(unDAWDataLogs, Verbose, All);
class UM2SoundGraphRenderer;

DECLARE_DELEGATE_TwoParams(FOnFusionPatchChanged, int, UFusionPatch*);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVertexUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVertexNeedsBuilderUpdates, UM2SoundVertex*, UpdatedVertex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVertexAdded, UM2SoundVertex*, AddedVertex);

class UDAWSequencerData;

USTRUCT(BlueprintType, Category = "unDAW Sequence")
struct FMidiExplicitMidiInstrumentTrack
{
	GENERATED_BODY()
	
	UPROPERTY()
	int32 TrackId = INDEX_NONE;

	UPROPERTY()
	int32 ChannelId = INDEX_NONE;
};

//the output creation logic will generate an array of these structs, each struct will contain the data needed to create an output in the metasound patch
// once not needed these can be returned to the pool
USTRUCT(BlueprintType, Category = "unDAW Sequence")
struct FAssignableAudioOutput
{
	GENERATED_BODY()

	UPROPERTY()
	FName OutputName;

	//this is the name of the parameter in the metasound patch that will be used to control the gain of this output
	UPROPERTY()
	FName GainParameterName;

	UPROPERTY()
	FMetaSoundBuilderNodeInputHandle AudioLeftOutputInputHandle;

	UPROPERTY()
	FMetaSoundBuilderNodeInputHandle AudioRightOutputInputHandle;

};

struct FM2VertexBuilderMessages
{
	FString Message;
	int32 Severity;
};

UENUM(BlueprintType, Category = "unDAW Sequence")
enum EBKTransportCommands : uint8
{
	Init,
	Play,
	Pause,
	Stop,
	Kill,
	TransportBackward,
	TransportForward,
	NextMarker,
	PrevMarker

};



UENUM(BlueprintType, Category = "unDAW Sequence", meta = (Bitflags))
enum EBKPlayState : uint8
{
	NoPerformer = 0,
	NotReady = 1,
	Preparing = 2,
	ReadyToPlay = 4,
	Playing = 8,
	Paused = 16


};

UENUM(BlueprintType, Category = "unDAW Sequence")
enum EM2SoundGraphConnectionStatus : uint8
{
	Connected,
	Disconnected,
	Pending
};

// this struct allows us to ensure a M2Sound vertex has intelligible data to be used by the metasound builder and once initialized, assuming metasound frontend data didn't change, we can use this data to recreate the sequencer
USTRUCT()
struct FM2SoundMetasoundBuilderPinData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FName PinName;

	UPROPERTY(VisibleAnywhere)
	FName DataType;

	UPROPERTY()
	TObjectPtr<UM2SoundVertex> InputVertex;

	UPROPERTY()
	TObjectPtr<UM2SoundVertex> OutputVertex;

};

USTRUCT()
struct FM2SoundGraphConnection
{
	GENERATED_BODY()
	
	FMetaSoundBuilderNodeInputHandle InputHandle;
	FMetaSoundBuilderNodeOutputHandle OutputHandle;

	EM2SoundGraphConnectionStatus Status = EM2SoundGraphConnectionStatus::Pending;

};

USTRUCT(BlueprintType)
struct FLinkedMidiEvents
{

	GENERATED_BODY()

	FLinkedMidiEvents(const FMidiEvent& StartEvent, const FMidiEvent& EndEvent, const int32 inStartIndex, const int32 inEndindex)
		:StartIndex(inStartIndex),
		EndIndex(inEndindex)

	{
		StartTick = StartEvent.GetTick();
		EndTick = EndEvent.GetTick();
		pitch = StartEvent.GetMsg().Data1;
	}

	FLinkedMidiEvents()
	{

	}

	//
	//FMidiEvent StartEvent;
	//FMidiEvent EndEvent;
	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	int32 StartIndex = 0;


	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	int32 EndIndex = 0;


	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	uint8 pitch = 0;


	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	int32 StartTick = 0;

	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	int32 EndTick = 0;

	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	int32 TrackId = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	int32 ChannelId = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	double Duration = 0.0;
	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	double StartTime = 0.0;

	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	float cornerRadius = 0.0f;

	void CalculateDuration(FSongMaps* SongsMap)
	{
		StartTime = SongsMap->TickToMs(StartTick);
		Duration = SongsMap->TickToMs(EndTick) - StartTime;
		
	}

	FString GetFormmatedString()
	{
		return FString::Printf(TEXT("StartTick: %d, \nEndTick: %d, \nDuration: %f, \nPitch: %d,\nTrackID: %d"), StartTick, EndTick, Duration, pitch, TrackId);
	}

};

USTRUCT(BlueprintType)
struct FLinkedNotesTrack
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TArray<FLinkedMidiEvents> LinkedNotes;
};


USTRUCT(BlueprintType)
struct FBPMidiStruct {

GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="unDAW|Midi|Note Data")
	int StartTick = 0;
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|Midi|Note Data")
	int EndTick = 0;
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|Midi|Note Data")
	int Duration = 0;
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|Midi|Note Data")
	int Pitch = 0;
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|Midi|Note Data")
	int Velocity = 0;
};



//Helper object used by the pianoroll and other visualizers to easily query the end time of a playing note and the next note on a give track/pitch
UCLASS(BlueprintType)
class BKMUSICCORE_API UParsedMidiTrack : public UObject
{
	
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|Midi Data")
	int TrackUniqueIndex = -1;

	
	//This can't be a UPROPERTY, we need to expose functions to interact with this data
	TArray<FLinkedMidiEvents> TrackData;


};



USTRUCT(BlueprintType, Category = "unDAW Sequence")
struct FMasterChannelOutputSettings {
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, Category = "unDAW Sequence", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float MasterVolume = 1.0f;


	UPROPERTY(EditAnywhere, Category = "unDAW Sequence", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float ClickVolume = 1.0f;



	UPROPERTY(EditAnywhere, Category = "unDAW Sequence")
	bool bClickActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unDAW Sequence")
	EMetaSoundOutputAudioFormat OutputFormat = EMetaSoundOutputAudioFormat::Stereo;



};


USTRUCT(BlueprintType, Category = "unDAW Sequence")
struct FTimeStamppedWavContainer {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "unDAW Sequence")
	FMusicTimestamp TimeStamp;

	UPROPERTY(EditAnywhere, Category = "unDAW Sequence")
	TObjectPtr <USoundWave> SoundWave;

};

USTRUCT(BlueprintType, Category = "unDAW Sequence")
struct FTimeStamppedCurveContainer {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW Sequence")
	FMusicTimestamp TimeStamp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW Sequence")
	TObjectPtr <UCurveFloat> Curve;

};

USTRUCT(BlueprintType, Category = "unDAW Sequence")
struct FTimeStamppedMidiContainer {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW Sequence")
	FMusicTimestamp TimeStamp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW Sequence")
	TObjectPtr<UMidiFile> MidiFile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW Sequence")
	bool bIsClockSource;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unDAW Sequence", meta = (TitleProperty = "trackName"))
	TArray<FTrackDisplayOptions> TracksMappings;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW Sequence")
	TMap<int, UParsedMidiTrack*> TrackMidiDataMap;

};

DECLARE_MULTICAST_DELEGATE(FMidiDataChanged)
DECLARE_MULTICAST_DELEGATE(FOnSelectionChanged)

class UM2SoundGraph;

UCLASS()
class BKMUSICCORE_API UM2SoundGraphBase : public UEdGraph
{
	GENERATED_BODY()
public:
	bool IsEditorOnly() const override { return true; }

	virtual void InitializeGraph() {};
};


//nodes

UCLASS(Abstract, AutoExpandCategories = ("M2Sound"))
class BKMUSICCORE_API UM2SoundVertex : public UObject
{
	GENERATED_BODY()

public:



	UPROPERTY(BlueprintAssignable, Category = "M2Sound")
	FOnVertexUpdated OnVertexUpdated;

	UPROPERTY(BlueprintAssignable, Category = "M2Sound")
	FOnVertexNeedsBuilderUpdates OnVertexNeedsBuilderUpdates;



	//Probably should only allow a single input (multi outputs), these are not the proper node i/os but rather the 'track' binding. 
	UPROPERTY()
	TArray<UM2SoundVertex*> Inputs;

	UPROPERTY()
	TArray<UM2SoundVertex*> Outputs;

	UFUNCTION()
	UDAWSequencerData* GetSequencerData() const;

	UPROPERTY()
	TObjectPtr<UDAWSequencerData> SequencerData;

	bool bBuiltSuccessfully = false;
	bool bIsInput = false;
	bool bIsOutput = false;

	UPROPERTY()
	FString VertexErrors;

	UPROPERTY()
	bool bHasErrors = false;

	UPROPERTY()
	int32 ErrorType = EMessageSeverity::Info;

	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	TArray<FM2SoundMetasoundBuilderPinData> MetasoundInputs;

	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	TArray<FM2SoundMetasoundBuilderPinData> MetasoundOutputs;

	UFUNCTION(BlueprintCallable)
	virtual void VertexNeedsBuilderUpdates();

	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	TMap<FName, EMetaSoundBuilderResult> BuilderResults;

#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override {};
#endif
};

UCLASS()
//this is a literal node that can be used to pass a value to the metasound graph
class BKMUSICCORE_API UM2SoundLiteralNodeVertex : public UM2SoundVertex
{
	GENERATED_BODY()

public:

};

// this vertex represends a midi output that can be queried from blueprint using the Listeners
UCLASS(AutoExpandCategories = ("M2Sound"))
class BKMUSICCORE_API UM2SoundMidiOutput : public UM2SoundVertex
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "M2Sound")
	FName OutputName;
	//FText GetTooltip() const override
	//{
	//	return INVTEXT("An output that can be queried from Blueprint.");
	//}

	//TArray<FInputInfo> GetInputInfo() const override
	//{
	//	return
	//	{
	//		{ {}, {}, INVTEXT("Output"), "0" }
	//	};
	//}

};

UCLASS()
class BKMUSICCORE_API UM2SoundAudioOutput : public UM2SoundVertex
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "M2Sound")
	float Gain = 1.0f;

	UPROPERTY(VisibleAnywhere)
	FName GainParameterName = NAME_None;

	UPROPERTY(VisibleAnywhere)
	FAssignableAudioOutput AssignedOutput;

};


UCLASS()
class BKMUSICCORE_API UM2SoundTrackInput : public UM2SoundVertex
{
	GENERATED_BODY()

public:

	//Represents the index of the track in the sequencer data, to get the actual midi metadata use this index to get the track from the sequencer data
	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	int TrackId = INDEX_NONE;


	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	FString TrackPrefix;

	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	FMetaSoundBuilderNodeOutputHandle MidiStreamOutput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "M2Sound")
	bool bOutputToBlueprints = true;

};

UCLASS()
class BKMUSICCORE_API UM2SoundPatch : public UM2SoundVertex
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, Category = "M2Sound")
	UMetaSoundPatch* Patch;

	//Audio::FParameterInterfacePtr interface = nullptr;


	//post import property edit
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif


};

UCLASS()
class BKMUSICCORE_API UM2SoundAudioInsert : public UM2SoundVertex
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, Category = "M2Sound")
	UMetaSoundPatch* Patch;

	//post import property edit
#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// yada

};


//This is the main data object that holds all the data for the sequencer, the idea is for this class to hold non-transient data that can be used to recreate the sequencer OR just expose the outputs via the saved metasound
//it's probably a bad idea to have the saved metasound option here... we can export to a new asset and then use that asset to recreate the sequencer without the realtime builder.

UCLASS(BlueprintType, EditInlineNew, Category = "unDAW Sequence")
class BKMUSICCORE_API UDAWSequencerData : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable, Category = "M2Sound")
	FOnVertexAdded OnVertexAdded;

	void AddVertex(UM2SoundVertex* Vertex);

	FOnFusionPatchChanged OnFusionPatchChangedInTrack;

	void ChangeFusionPatchInTrack(int TrackID, UFusionPatch* NewPatch);

	FMidiDataChanged OnMidiDataChanged;

	FOnSelectionChanged OnSelectionChanged;
	
	TSharedPtr<UDAWSequencerData, ESPMode::ThreadSafe> SelfSharedPtr;

	TSharedPtr<UDAWSequencerData> GetSelfSharedPtr()
	{
		if (SelfSharedPtr.IsValid() == false)
		{
			SelfSharedPtr = TSharedPtr<UDAWSequencerData>(this);
		}
		
		return SelfSharedPtr;
	}


	UPROPERTY()
	TArray<FTrackDisplayOptions> M2TrackMetadata;

	void InitVertexesFromFoundMidiTracks(TArray<TTuple<int, int>> InTracks);

	FTrackDisplayOptions& GetTracksDisplayOptions(const int& ID);


#if WITH_EDITORONLY_DATA

	UPROPERTY(VisibleAnywhere, Category = "unDAW", Transient)
	UM2SoundGraphRenderer* EditorPreviewPerformer;

	UPROPERTY()
	UM2SoundGraphBase* M2SoundGraph;

	UPROPERTY()
	FVector2D GraphPosition;

	UPROPERTY()
	FVector2D GraphZoom;


#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unDAW Sequence|Meta Sound")
	TObjectPtr<UMetaSoundSource> SavedMetaSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "unDAW Sequence")
	float SequenceDuration = 100.0f;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW Sequence", meta = (ShowInnerProperties = "true", DisplayPriority = "0", ExposeOnSpawn = "true", EditInLine = "true"))
	FMasterChannelOutputSettings MasterOptions;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW Sequence", meta = (ShowInnerProperties = "true", DisplayPriority = "2", ExposeOnSpawn = "true", EditInLine = "true"))
	TArray<FTimeStamppedCurveContainer> TimeStampedCurves;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW Sequence", meta = (ShowInnerProperties = "true", DisplayPriority = "3", ExposeOnSpawn = "true", EditInLine = "true"))
	TArray<FTimeStamppedWavContainer> TimeStampedWavs;

	UFUNCTION()
	void CalculateSequenceDuration();


	TMap<int, FLinkedMidiEvents> LinkedMidiNotesMap;


	UFUNCTION()
	void PopulateFromMidiFile(UMidiFile* inMidiFile);

	UFUNCTION()
	UM2SoundGraphRenderer* CreatePerformer(UAudioComponent* AuditionComponent);

	UPROPERTY(EditAnywhere, Category = "unDAW")
	UMidiFile* HarmonixMidiFile;

	

	UPROPERTY()
	TArray<FMidiEvent> TempoEvents;

	UPROPERTY()
	TArray<FMidiEvent> TimeSignatureEvents;

	//this is a map that sorts the midi events by track and links start/end events with each other, needed for the pianoroll and other visualizers

	UPROPERTY()
	TMap<int, FLinkedNotesTrack> LinkedNoteDataMap;

	//UPROPERTY()
	//TMap <FMidiExplicitMidiInstrumentTrack, FLinkedMidiEvents> SievedLinkedMidiEventsMap;

	

#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	private:

		FTrackDisplayOptions InvalidTrackRef;



// as the sequener should contain a 'recipe' it effectively needs several maps to store the data, mapping the different types of vertexes, the data in these, coupled with the metadata extracted from the midi file should suffice to create a static performer
// in the future we will add a curvetable to the sequencer data, this will allow us to store the data for the curves in the sequencer, this will be used to create the curves in the performer
	
		public:
		//the outputs map should be used by Listeners in the scene to easily get MIDI outputs and other outputs, MIDI is the priority, we might create multiple maps for other data types.
		UPROPERTY(VisibleAnywhere)
		TMap<FName, UM2SoundMidiOutput*> Outputs;

		UPROPERTY(VisibleAnywhere)
		TMap<FName, UM2SoundPatch*> Patches;

		UPROPERTY(VisibleAnywhere)
		TArray<FAssignableAudioOutput> AudioOutputs;

		UPROPERTY(VisibleAnywhere)
		TMap<int, UM2SoundTrackInput*> TrackInputs;

};

