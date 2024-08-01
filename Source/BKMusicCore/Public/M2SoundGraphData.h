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

#include <Pins/M2Pins.h>
#include "M2SoundGraphData.generated.h"

BKMUSICCORE_API DECLARE_LOG_CATEGORY_EXTERN(unDAWDataLogs, Verbose, All);
class UM2SoundGraphRenderer;
class UM2SoundVertex;
class UM2VariMixerVertex;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVertexUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBuilderReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVertexNeedsBuilderUpdates, UM2SoundVertex*, UpdatedVertex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVertexAdded, UM2SoundVertex*, AddedVertex);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioParameterFromVertex, FAudioParameter, Parameter);

//on timestamp updated dynamic multicast delegate one param please
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeStampUpdated, FMusicTimestamp, NewTimestamp);

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

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UM2AudioTrackPin> AssignedPin;
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
	TransportPlaying = 8,
	TransportPaused = 16
};

UENUM(BlueprintType, Category = "unDAW Sequence")
enum EM2SoundGraphConnectionStatus : uint8
{
	Connected,
	Disconnected,
	Pending
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlaybackStateChanged, EBKPlayState, NewPlaystate);

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

	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data", BlueprintReadOnly)
	uint8 pitch = 0;

	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	int32 StartTick = 0;

	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	int32 EndTick = 0;

	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	int32 TrackId = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data")
	int32 ChannelId = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data", BlueprintReadOnly)
	double Duration = 0.0;
	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data", BlueprintReadOnly)
	double StartTime = 0.0;

	UPROPERTY(VisibleAnywhere, Category = "unDAW|Midi Data", BlueprintReadOnly)
	float NoteVelocity = 0.0f;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FLinkedMidiEvents> LinkedNotes;
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

USTRUCT(BlueprintType, Category = "unDAW Sequence")
struct FMemberInput
{
	GENERATED_BODY()

	//will also serve as the audio parameter name if you want to use it as such
	UPROPERTY(VisibleAnywhere)
	FName Name;

	//the data type of the underlying metasound literal parameter
	UPROPERTY(VisibleAnywhere)
	FName DataType;

	//this is the output handle for the graph input vertex, needs to be assigned when the vertex is created
	UPROPERTY()
	FMetaSoundBuilderNodeOutputHandle MemberInputOutputHandle;

	//A little confusing but this is the input handle for the graph output vertex, if assigned
	UPROPERTY()
	FMetaSoundBuilderNodeInputHandle GraphOutputInputHandle;

	UPROPERTY()
	bool bGraphOutput = false;

	// when rebuilding the graph all member inputs are marked as stale before rebuilding, if after rebuilding they are still stale, they will be removed
	UPROPERTY()
	bool bIsStale = true;

	UPROPERTY(VisibleAnywhere)
	int MetadataIndex = INDEX_NONE;

	void SetMemberInputOutputHandle(FMetaSoundBuilderNodeOutputHandle InHandle, FName InName, FName InDataType)
	{
		MemberInputOutputHandle = InHandle;
		Name = InName;
		DataType = InDataType;
		bIsStale = false;
	}
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnTriggerExecuted, FName, TriggerName);

//for now this will contain handles for some key connections that other nodes may rely on, expected to be populated before the first vertex is being built
USTRUCT()
struct BKMUSICCORE_API FM2SoundCoreNodesComposite
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FMemberInput> MemberInputs;

	UPROPERTY()
	TObjectPtr<UMetaSoundPatch> BusTransmitterPatchClass;

	UPROPERTY(VisibleAnywhere)
	TMap<FName, FMemberInput> MemberInputMap;

	UPROPERTY()
	bool bIsLooping = false;

	UPROPERTY()
	int32 BarLoopDuration = 4;

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

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAudioBus> MasterOutputBus;

	//graph build results and metadata

	UPROPERTY(VisibleAnywhere)
	TMap<FName, EMetaSoundBuilderResult> BuilderResults;


	FAssignableAudioOutput GetFreeMasterMixerAudioOutput();
	void ReleaseMasterMixerAudioOutput(FAssignableAudioOutput Output);

	void MarkAllMemberInputsStale();

	void RemoveAllStaleInputs();

	void CreateOrUpdateMemberInput(FMetaSoundBuilderNodeOutputHandle InHandle, FName InName = NAME_None, int MetadataIndex = INDEX_NONE);

	//Might be nicer user experience to categorize outputs by data types, for now this will only contain MIDI outputs
	UPROPERTY()
	TMap<int32, FMemberInput> MappedOutputs;

protected:
	friend class UDAWSequencerData;

	void InitCoreNodes(UMetaSoundSourceBuilder* InBuilderContext, UDAWSequencerData* ParentSession, UAudioBus* InMasterOutBus = nullptr);

	UMetaSoundSourceBuilder* BuilderContext;

	FMetaSoundBuilderNodeOutputHandle CreateFilterNodeForTrack(int32 TrackMetadataIndex);

	void CreateBusTransmitterAndStrealMainOutput();

private:

	UPROPERTY();
	UDAWSequencerData* SessionData;
	void CreateMidiPlayerAndMainClock();
	void CreateMainMixer();

	void ResizeOutputMixer();
};

class UMappedVertexCache;

//This is the main data object that holds all the data for the sequencer, the idea is for this class to hold non-transient data that can be used to recreate the sequencer OR just expose the outputs via the saved metasound
//it's probably a bad idea to have the saved metasound option here... we can export to a new asset and then use that asset to recreate the sequencer without the realtime builder.

UCLASS(BlueprintType, EditInlineNew, Category = "unDAW Sequence")
class BKMUSICCORE_API UDAWSequencerData : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
public:

	float MetasoundCpuUtilization = 0.0f;

	bool AttachActionPatchToMixer(FName InMixerAlias, UMetaSoundPatch* Patch, float InVolume, const FOnTriggerExecuted& InDelegate);


	//as metasounds have rather weird support for structs we also need a Parameter Pack to hold data for struct pins
	UPROPERTY()
	TObjectPtr<UMetasoundParameterPack> StructParametersPack;

	UMetaSoundPatchBuilder* PatchBuilder;

	UPROPERTY(VisibleAnywhere)
	UMetaSoundPatch* BuilderPatch;

	UFUNCTION(CallInEditor)
	void CreateNewPatchBuilder();

	void CreateDefaultVertexes();

	void ConnectTransientVertexToMidiClock(UM2SoundVertex* Vertex);

	bool TraverseOutputPins(UM2SoundVertex* Vertex, TFunction<bool(UM2SoundVertex*)> Predicate);

	bool TraverseInputPins(UM2SoundVertex* Vertex, TFunction<bool(UM2SoundVertex*)> Predicate);

	bool WillConnectionCauseLoop(UM2SoundVertex* InInput, UM2SoundVertex* InOutput);

	template<typename T>
	bool BreakPinConnection(T* InInput)
	{
		UE_LOG(unDAWDataLogs, Warning, TEXT("BreakPinConnection not implemented for this type - UNSPECIALIZED"));
		return false;
	};

	template<>
	bool BreakPinConnection<UM2MetasoundLiteralPin>(UM2MetasoundLiteralPin* InInput)
	{
			if (InInput)
			{
				EMetaSoundBuilderResult Result;
				auto* LinkedToOutput = Cast<UM2MetasoundLiteralPin>(InInput->LinkedPin);
				BuilderContext->DisconnectNodes(LinkedToOutput->GetHandle<FMetaSoundBuilderNodeOutputHandle>(), InInput->GetHandle<FMetaSoundBuilderNodeInputHandle>(), Result);

				//turns out the builder always returns 'fail' for disconnections, so we'll have to assume it succeeded...
				LinkedToOutput->LinkedPin = nullptr;
				InInput->LinkedPin = nullptr;

				if(InInput->AutoGeneratedGraphInputName != NAME_None)
				{
					BuilderContext->ConnectNodeInputToGraphInput(InInput->AutoGeneratedGraphInputName, InInput->GetHandle<FMetaSoundBuilderNodeInputHandle>(), Result);
				}

				return true;
			}
		return false;
	};

	template<>
	bool BreakPinConnection<UM2AudioTrackPin>(UM2AudioTrackPin* InInput)
	{
			if (InInput)
			{
				bool bBreakLeft = BreakPinConnection<UM2MetasoundLiteralPin>(InInput->AudioStreamL);
				bool bBreakRight = BreakPinConnection<UM2MetasoundLiteralPin>(InInput->AudioStreamR);
				if (bBreakLeft && bBreakRight)
				{
					InInput->LinkedPin->LinkedPin = nullptr;
					InInput->LinkedPin = nullptr;
					return true;
				}
			}
		return false;
	};

	//this templating is a stupid confusing waste of time doesn't save any effort and makes the code harder to read, can just make these plain ole methods
	template<typename T>
	bool ConnectPins(T* InInput, T* InOutput)
	{
		UE_LOG(unDAWDataLogs, Warning, TEXT("ConnectPins not implemented for this type - UNSPECIALIZED"));

		return false;
	};


	template<>
	bool ConnectPins<UM2MetasoundLiteralPin>(UM2MetasoundLiteralPin* InInput, UM2MetasoundLiteralPin* InOutput);;

	template<>
	bool ConnectPins<UM2AudioTrackPin>(UM2AudioTrackPin* InInput, UM2AudioTrackPin* InOutput)
	{
		if (InInput && InOutput)
		{
			bool bConnectLeft = ConnectPins<UM2MetasoundLiteralPin>(InInput->AudioStreamL, InOutput->AudioStreamL);
			bool bConnectRight = ConnectPins<UM2MetasoundLiteralPin>(InInput->AudioStreamR, InOutput->AudioStreamR);

			if (InInput->ParentVertex == InOutput->ParentVertex)
			{
				
				return false;
			}

			if (bConnectLeft && bConnectRight)
			{
				InInput->LinkedPin = InOutput;
				InOutput->LinkedPin = InInput;
				return true;
			}
		}
	
		return false;
	};

	UFUNCTION(CallInEditor, Category = "unDAW")
	void SaveDebugMidiFileTest();

	UPROPERTY(BlueprintAssignable, Category = "M2Sound")
	FOnTimeStampUpdated OnTimeStampUpdated;

	//this one is called when the vertex needs to be rebuilt, this is used to update the vertexes in the sequencer data
	UPROPERTY(BlueprintAssignable, Category = "M2Sound")
	FOnVertexNeedsBuilderUpdates OnVertexNeedsBuilderNodeUpdates;

	//used for the less 'violent' updates, ones that don't create underlying nodes in the metasound graph but rather only affect the connections between nodes
	UPROPERTY(BlueprintAssignable, Category = "M2Sound")
	FOnVertexNeedsBuilderUpdates OnVertexNeedsBuilderConnectionUpdates;

	UFUNCTION()
	void RebuildVertex(UM2SoundVertex* Vertex);

	UFUNCTION()
	void UpdateVertexConnections(UM2SoundVertex* Vertex);

	UPROPERTY()
	FOnPlaybackStateChanged OnPlaybackStateChanged;

	UFUNCTION()
	void ReceiveAudioParameter(FAudioParameter Parameter);

	UFUNCTION()
	void ExecuteTriggerParameter(FName ParameterName);

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

	//After BeginPlay it is safe to use this audio component to set up additional output watchers or to send audio parameters
	UPROPERTY(Transient, BlueprintReadOnly, Category = "unDAW")
	UAudioComponent* AuditionComponent = nullptr;

	TSet<TTuple<int32, int32>> CurrentlyActiveNotes;

	UPROPERTY()
	FOnBuilderReady OnBuilderReady;

public:

	FOnMetasoundOutputValueChangedNative OnMidiStreamOutputReceived;
	FOnMetasoundOutputValueChangedNative OnMidiClockOutputReceived;

	void SetLoopSettings(const bool& InbIsLooping, const int32& BarDuration);

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

	UFUNCTION(CallInEditor, Category = "unDAW")
	void PrintMidiData();

	//for now I'll mark this call in editor for testing, but we need fancier management of track add and removal, it should probably
	//edit the midi data and be registered in undos
	UFUNCTION(Category = "unDAW")
	void AddTrack();

	//Just for development and debug! don't call unless you want to destroy the session!
	UFUNCTION(CallInEditor, Category = "unDAW")
	void ReinitGraph();

	UPROPERTY(VisibleAnywhere)
	FM2SoundCoreNodesComposite CoreNodes;

	UPROPERTY(BlueprintAssignable, Category = "unDAW")
	FOnVertexAdded OnVertexAdded;

	void AddTransientVertex(UM2SoundVertex* Vertex);

	void AddVertex(UM2SoundVertex* Vertex);

	FMidiDataChanged OnMidiDataChanged;

	FMusicTimestamp CurrentTimestampData;

	FOnSelectionChanged OnSelectionChanged;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "unDAW")
	UMetaSoundSourceBuilder* BuilderContext;

	UPROPERTY(VisibleAnywhere)
	UCurveFloat* TempoCurve;

	UMetaSoundBuilderSubsystem* MSBuilderSystem;

	UPROPERTY(VisibleAnywhere)
	FName BuilderName;

	UPROPERTY()
	int SelectedTrackIndex = INDEX_NONE;

	UPROPERTY()
	TMap<UM2SoundVertex*, FAssignableAudioOutput> AudioOutsMap;

	UPROPERTY(VisibleAnywhere)
	TArray<FTrackDisplayOptions> M2TrackMetadata;

	void InitMetadataFromFoundMidiTracks(TArray<TTuple<int, int>> InTracks);

	UFUNCTION(BlueprintCallable, Category = "unDAW")
	FTrackDisplayOptions& GetTracksDisplayOptions(const int& ID);

	UFUNCTION()
	void CalculateSequenceDuration();

	UPROPERTY()
	TArray<FLinkedMidiEvents> PendingLinkedMidiNotesMap;

	//this is actually how we add notes to the midi files, it creates a new 'editable midi file' which is a copy of the harmonix midi file
	//and not really editable at all, the next time we make a change a copy of this file will be created
	//can thing of some improvements to this whole process
	UFUNCTION(BlueprintAuthorityOnly, Category = "unDAW")
	void AddLinkedMidiEvent(FLinkedMidiEvents PendingNote);

	UFUNCTION(BlueprintAuthorityOnly, Category = "unDAW")
	void DeleteLinkedMidiEvent(FLinkedMidiEvents PendingNote);

	UFUNCTION()
	void PopulateFromMidiFile(UMidiFile* inMidiFile);

	//this method cleans the pending notes map and populates the main linked notes map with the data from the midi file, which should already contain the pending notes
	//optionally returns the discovered channels/tracks from the midi file
	void UpdateNoteDataFromMidiFile(TArray<TTuple<int, int>>& OutDiscoveredChannels);

	UFUNCTION()
	void FindOrCreateBuilderForAsset(bool bResetBuilder = false, UAudioBus* MasterOutputBus = nullptr);

	UFUNCTION()
	void ApplyParameterPack();

	//void ApplyParameterViaParameterPack

	const TSet<UM2SoundVertex*>& GetVertexes() const { return Vertexes; }

	void RemoveVertex(UM2SoundVertex* Vertex);

#if WITH_EDITORONLY_DATA

	UPROPERTY()
	UM2SoundGraphBase* M2SoundGraph;

	UPROPERTY()
	FVector2D GraphPosition;

	UPROPERTY()
	FVector2D GraphZoom;

#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unDAW")
	TObjectPtr<UMetaSoundSource> SavedMetaSound;

	void BeginDestroy() override;

	UPROPERTY(VisibleAnywhere, Category = "unDAW")
	float BeatsPerMinute = 120.0f;

	UPROPERTY(BlueprintReadOnly, Category = "unDAW")
	float SequenceDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "unDAW", meta = (ShowInnerProperties = "true", DisplayPriority = "0", ExposeOnSpawn = "true", EditInLine = "true"))
	FMasterChannelOutputSettings MasterOptions;

	TEnumAsByte<EBKPlayState> PlayState;

	UFUNCTION(BlueprintCallable, Category = "unDAW")
	void AuditionBuilder(UAudioComponent* InAuditionComponent, bool bForceRebuild = false, UAudioBus* MasterOutBus = nullptr);

	//for now this has to be set, although switching midi files is possible it deletes the entire graph
	//I'll make this thus read only, and change the facotry so that it creates a new empty midi file when we create a new session
	UPROPERTY(VisibleAnywhere, Category = "unDAW", BlueprintReadOnly)
	UMidiFile* HarmonixMidiFile;

	//we probably don't need all of these copies... 
	UPROPERTY()
	TArray<FMidiEvent> TempoEvents;

	//should remember time signature can only come on bar start
	UPROPERTY()
	TArray<FMidiEvent> TimeSignatureEvents;

	UPROPERTY(VisibleAnywhere, Category = "unDAW")
	TMap<int, FVector2f> TimeSignatureMap;

	UPROPERTY(VisibleAnywhere, Category = "unDAW")
	TMap<float, double> TempoEventsMap;

	//this is a map that sorts the midi events by track and links start/end events with each other, needed for the pianoroll and other visualizers
	//Adding or removing notes should be done with the provided methods, objects that use this data should also bind to OnMidiDataChanged
	// (if not passed as ref)

	UPROPERTY(BlueprintReadOnly)
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

	UPROPERTY(Transient, VisibleAnywhere)
	TSet<UM2SoundVertex*> TransientVertexes;


	// as the sequener should contain a 'recipe' it effectively needs several maps to store the data, mapping the different types of vertexes, the data in these, coupled with the metadata extracted from the midi file should suffice to create a static performer
	// in the future we will add a curvetable to the sequencer data, this will allow us to store the data for the curves in the sequencer, this will be used to create the curves in the performer

public:

	UPROPERTY()
	TMap<FName, UM2VariMixerVertex*> Mixers;

	UPROPERTY()
	TMap<FName, UM2MetasoundLiteralPin*> NamedInputs;

	UFUNCTION()
	bool RenameNamedInput(FName OldName, FName NewName);

	UPROPERTY()
	TMap<FName, UM2MetasoundLiteralPin*> NamedOutputs;

	//TArray<FName> GetMixerNames();

	UPROPERTY()
	TArray<FAssignableAudioOutput> AudioOutputs;

	//This delegate is listened to by the performer and is fired by the vertexes
	UPROPERTY()
	FOnAudioParameterFromVertex OnAudioParameterFromVertex;

	bool IsRecreatingMidiFile = false;

	//not used, we push notes individually and create a new file for each one, kind of a waste if we're not actually playing back
	//on the other hand, we don't currently 'manage' the pending notes, we just add to them and the pianoroll uses them to display the notes
	//alongside the original notes from the midi file; we can get the best of both worlds by sending partial 'chunks'
	//further optimizations can be achieved by tracking the transport position and state and using that to push notes on time
	UFUNCTION()
	void PushPendingNotesToNewMidiFile();
};

template<>
inline bool UDAWSequencerData::ConnectPins(UM2MetasoundLiteralPin* InInput, UM2MetasoundLiteralPin* InOutput)
{

	//so we need to check if output is wildcard, if it is wildcard we check if the builder already has a graph input for this pin, if it does we connect it,
	// if not we create it and assign the handle to the pin.

	if (InOutput->DataType == M2Sound::Pins::AutoDiscovery::WildCard)
	{
		EMetaSoundBuilderResult Result;
		//auto* VertexAsDynamicInput = Cast<UM2SoundDynamicGraphInputVertex>(InOutput->ParentVertex);
		FName MemberName = InOutput->AutoGeneratedGraphInputName;
		if (InOutput->bIsSet)
		{
			BuilderContext->ConnectNodeInputToGraphInput(MemberName, InInput->GetHandle<FMetaSoundBuilderNodeInputHandle>(), Result);
		}
		else
		{
			// this is problematic, we need a more robust mechanism for settings defaults, this will assign the default by the first connection which is unpredictable and undesirable
			InOutput->SetHandle<FMetaSoundBuilderNodeOutputHandle>(BuilderContext->AddGraphInputNode(MemberName, InInput->DataType, InInput->LiteralValue, Result));
			BuilderContext->ConnectNodeInputToGraphInput(MemberName, InInput->GetHandle<FMetaSoundBuilderNodeInputHandle>(), Result);
			InOutput->bIsSet = true;
		}
		//auto GraphInputMember = BuilderContext->ConnectNodeInputToGraphInput
		return Result == EMetaSoundBuilderResult::Succeeded;
	}


	if (InInput && InOutput)
	{
		EMetaSoundBuilderResult Result;
		BuilderContext->ConnectNodes(InOutput->GetHandle<FMetaSoundBuilderNodeOutputHandle>(), InInput->GetHandle<FMetaSoundBuilderNodeInputHandle>(), Result);

		//print all data from the pins so we debug what's going on
		//print connection result
		if (Result == EMetaSoundBuilderResult::Succeeded)
		{
			InInput->LinkedPin = InOutput;
			InOutput->LinkedPin = InInput;
		}
		else
		{
			UE_LOG(unDAWDataLogs, Warning, TEXT("Connection Failed!"));
		}

		InInput->ConnectionResult = Result;


		return Result == EMetaSoundBuilderResult::Succeeded;
	}
	return false;
}
