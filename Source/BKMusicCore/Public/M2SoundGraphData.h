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
#include "MetasoundGeneratorHandle.h"

#include "Metasound.h"
#include "MetasoundBuilderSubsystem.h"

#include "TrackPlaybackAndDisplayOptions.h"

#include "M2SoundGraphData.generated.h"

BKMUSICCORE_API DECLARE_LOG_CATEGORY_EXTERN(unDAWDataLogs, Verbose, All);
class UM2SoundGraphRenderer;
class UM2SoundVertex;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVertexUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVertexNeedsBuilderUpdates, UM2SoundVertex*, UpdatedVertex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVertexAdded, UM2SoundVertex*, AddedVertex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioParameterFromVertex, FAudioParameter, Parameter);

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

	UPROPERTY(VisibleAnywhere)
	FName OutputName;

	//this is the name of the parameter in the metasound patch that will be used to control the gain of this output

	UPROPERTY(VisibleAnywhere)
	FMetaSoundBuilderNodeInputHandle AudioLeftOutputInputHandle;

	UPROPERTY(VisibleAnywhere)
	FMetaSoundBuilderNodeInputHandle AudioRightOutputInputHandle;

	UPROPERTY(VisibleAnywhere)
	FMetaSoundBuilderNodeInputHandle GainParameterInputHandle;
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
	NoBuilder = 0,
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

//for now this will contain handles for some key connections that other nodes may rely on, expected to be populated before the first vertex is being built
USTRUCT()
struct BKMUSICCORE_API FM2SoundCoreNodesComposite
{
	
	GENERATED_BODY()

	//patch references
	UPROPERTY()
	TScriptInterface<IMetaSoundDocumentInterface> MidiFilterDocument;

	//internal node references

	UPROPERTY()
	FMetaSoundNodeHandle MidiPlayerNode;

	UPROPERTY()
	FMetaSoundBuilderNodeOutputHandle MainMidiStreamOutput; //the stream going out of our 'main' player

	UPROPERTY()
	FMetaSoundBuilderNodeOutputHandle OnPlayOutputNode;

	UPROPERTY()
	FMetaSoundBuilderNodeInputHandle OnFinishedNodeInput;
	UPROPERTY()
	TArray<FMetaSoundBuilderNodeInputHandle> AudioOuts;

	UPROPERTY(VisibleAnywhere)
	TArray<FAssignableAudioOutput> MasterOutputs;

	//graph build results and metadata

	UPROPERTY(VisibleAnywhere)
	TMap<FName,EMetaSoundBuilderResult> BuilderResults;

	FAssignableAudioOutput GetFreeMasterMixerAudioOutput(UMetaSoundSourceBuilder* BuilderContext);

protected:
	friend class UDAWSequencerData;

	void InitCoreNodes(UMetaSoundSourceBuilder* BuilderContext, UDAWSequencerData* ParentSession);

private:

	UPROPERTY();
	UDAWSequencerData* SessionData;
	void CreateMidiPlayerAndMainClock(UMetaSoundSourceBuilder* BuilderContext);
	void CreateMainMixer(UMetaSoundSourceBuilder* BuilderContext);

	void ResizeOutputMixer(UMetaSoundSourceBuilder* BuilderContext);

};


//This is the main data object that holds all the data for the sequencer, the idea is for this class to hold non-transient data that can be used to recreate the sequencer OR just expose the outputs via the saved metasound
//it's probably a bad idea to have the saved metasound option here... we can export to a new asset and then use that asset to recreate the sequencer without the realtime builder.

UCLASS(BlueprintType, EditInlineNew, Category = "unDAW Sequence")
class BKMUSICCORE_API UDAWSequencerData : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|Audition Component")
	UMetasoundGeneratorHandle* GeneratorHandle;

	//tickable object interface, neccesary to monitor the sequencer in editor

	bool bShouldTick = false;

	void Tick(float DeltaTime) override;

	virtual bool IsTickable() const;

	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(FUnDawSessionTickInEditor, STATGROUP_Tickables); }
	virtual bool IsTickableWhenPaused() const { return true; }
	virtual bool IsTickableInEditor() const { return true; }

private:
	UPROPERTY(VisibleAnywhere, Transient)
	UAudioComponent* AuditionComponent = nullptr;

public:

	FOnMetasoundOutputValueChangedNative OnMidiStreamOutputReceived;
	FOnMetasoundOutputValueChangedNative OnMidiClockOutputReceived;

	// binding to generator outputs, used to monitor transport and midi outputs
	UFUNCTION()
	void ReceiveMetaSoundMidiStreamOutput(FName OutputName, const FMetaSoundOutput Value);

	UFUNCTION()
	void ReceiveMetaSoundMidiClockOutput(FName OutputName, const FMetaSoundOutput Value);

	UFUNCTION()
	void OnMetaSoundGeneratorHandleCreated(UMetasoundGeneratorHandle* Handle);

	void SendTransportCommand(EBKTransportCommands Command);

	void SendSeekCommand(float InSeek);

	bool isInEditorPreview = false;

	UPROPERTY(VisibleAnywhere)
	FM2SoundCoreNodesComposite CoreNodes;

	UPROPERTY(BlueprintAssignable, Category = "M2Sound")
	FOnVertexAdded OnVertexAdded;

	void AddVertex(UM2SoundVertex* Vertex);

	FMidiDataChanged OnMidiDataChanged;

	FMusicTimestamp CurrentTimestampData;

	FOnSelectionChanged OnSelectionChanged;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "unDAW Sequence")
	UMetaSoundSourceBuilder* BuilderContext;

	UMetaSoundBuilderSubsystem* MSBuilderSystem;

	UPROPERTY(VisibleAnywhere)
	FName BuilderName;

	UPROPERTY()
	int SelectedTrackIndex = INDEX_NONE;

	UPROPERTY()
	TMap<UM2SoundVertex*, FAssignableAudioOutput> AudioOutsMap;

	UPROPERTY()
	TArray<FTrackDisplayOptions> M2TrackMetadata;

	void InitVertexesFromFoundMidiTracks(TArray<TTuple<int, int>> InTracks);

	FTrackDisplayOptions& GetTracksDisplayOptions(const int& ID);

	UFUNCTION()
	void CalculateSequenceDuration();

	TMap<int, FLinkedMidiEvents> LinkedMidiNotesMap;

	UPROPERTY()
	TArray<FLinkedMidiEvents> PendingLinkedMidiNotesMap;

	void AddLinkedMidiEvent(FLinkedMidiEvents PendingNote);

	UFUNCTION()
	void PopulateFromMidiFile(UMidiFile* inMidiFile);

	UFUNCTION()
	void FindOrCreateBuilderForAsset(bool bResetBuilder = false);

	const TSet<UM2SoundVertex*>& GetVertexes() const { return Vertexes; }

#if WITH_EDITORONLY_DATA

	UPROPERTY()
	UM2SoundGraphBase* M2SoundGraph;

	UPROPERTY()
	FVector2D GraphPosition;

	UPROPERTY()
	FVector2D GraphZoom;

#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unDAW Sequence|Meta Sound")
	TObjectPtr<UMetaSoundSource> SavedMetaSound;

	void BeginDestroy() override;

	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "unDAW Sequence")
	float SequenceDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW Sequence", meta = (ShowInnerProperties = "true", DisplayPriority = "0", ExposeOnSpawn = "true", EditInLine = "true"))
	FMasterChannelOutputSettings MasterOptions;


	TEnumAsByte<EBKPlayState> PlayState;



	UFUNCTION()
	bool AuditionBuilder(UAudioComponent* InAuditionComponent, bool bForceRebuild = false);

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

	UPROPERTY(VisibleAnywhere)
	TSet<UM2SoundVertex*> Vertexes;

	// as the sequener should contain a 'recipe' it effectively needs several maps to store the data, mapping the different types of vertexes, the data in these, coupled with the metadata extracted from the midi file should suffice to create a static performer
	// in the future we will add a curvetable to the sequencer data, this will allow us to store the data for the curves in the sequencer, this will be used to create the curves in the performer

public:

	UPROPERTY(VisibleAnywhere)
	TArray<FAssignableAudioOutput> AudioOutputs;


	//This delegate is listened to by the performer and is fired by the vertexes
	UPROPERTY(BlueprintAssignable, Category = "M2Sound")
	FOnAudioParameterFromVertex OnAudioParameterFromVertex;

	bool IsRecreatingMidiFile = false;

	UFUNCTION(CallInEditor)
	void PushPendingNotesToNewMidiFile();
};
