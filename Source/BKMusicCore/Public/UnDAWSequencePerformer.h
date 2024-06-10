// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MetasoundBuilderSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
//#include "Harmoni"
//#include "BK_MusicSceneManagerInterface.h"
#include "HarmonixMetasound/DataTypes/MusicTimestamp.h"
#include "Components/AudioComponent.h"
#include "Metasound.h"
#include "MetasoundSource.h"
#include "MetasoundGeneratorHandle.h"
#include "SequencerData.h"
#include "UnDAWSequencePerformer.generated.h"




DECLARE_MULTICAST_DELEGATE(FDAWPerformerReady);
DECLARE_MULTICAST_DELEGATE(FDAWPerformerDeleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMusicTimestampUpdated, FMusicTimestamp, NewTimestamp);

DECLARE_DELEGATE_OneParam(FOnTransportSeekCommand, float)
DECLARE_DELEGATE_OneParam(FOnMusictimestampFromPerformer, FMusicTimestamp)

/**
 * This class is effectively the 'performer' for DAW Sequencer data. It is responsible for creating the necessary nodes and connections to play back the midi data in the sequencer data.
 */
UCLASS(BlueprintType, Blueprintable)
class BKMUSICCORE_API UM2SoundGraphRenderer : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:

	~UM2SoundGraphRenderer();

	//Main entry point for the performer to start building the nodes and connections from the vertexes in the sequencer data
	void InitPerformer();

	//this function should be called on a every Vertex as it ibeing changed or created, will establish pointers from the vertex to the actual metasound node i/os. 
	UFUNCTION(BlueprintCallable, Category = "unDAW")
	void UpdateVertex(UM2SoundVertex* Vertex);

	//After the performer has been created it can be supplied an audio component to audition the output, when called from editor we supply the preview editor component and run some extra to stop other performers
	//@param InComponent The audio component to audition the output, if null the performer will not create an auditionable metasound
	//@param bReceivesLiveUpdates If true the metasound NODEs will be updatable when the metasound is playing, otherwise it will be static
	void CreateAuditionableMetasound(UAudioComponent* InComponent, bool bReceivesLiveUpdates);

	//This function may be called when the performer is initialized and ready to play, it can save the CURRENT STATE of the builder graph into a new metasound asset
	void SaveMetasoundToAsset();


	FOnMetasoundOutputValueChangedNative OnMidiStreamOutputReceived;
	FOnMetasoundOutputValueChangedNative OnMidiClockOutputReceived;

	FOnTransportSeekCommand OnSeekEvent;

	FDAWPerformerDeleted OnDeleted;

	FMusicTimestampUpdated OnTimestampUpdated;

	FOnMusictimestampFromPerformer OnMusicTimestampFromPerformer;

	//IBK_MusicSceneManagerInterface* ParentSceneManager;

	TEnumAsByte<EBKPlayState> PlayState;

	FDAWPerformerReady OnDAWPerformerReady;

	UFUNCTION(BlueprintCallable, Category = "unDAW")
	void SendTransportCommand(EBKTransportCommands Command);

	UPROPERTY()
	UWorld* ParentWorld;

	FMetaSoundBuilderNodeOutputHandle MainMidiStreamOutput;
	
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper")
	FMetaSoundBuilderNodeOutputHandle OnPlayOutputNode;

	TArray<FMetaSoundBuilderNodeInputHandle> AudioOuts;

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper")

	TMap<UM2SoundVertex*, FAssignableAudioOutput> AudioOutsMap;

	//Name of this class TBD but this is the core data structure that is used to build the builder graph
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper", meta = (ExposeOnSpawn = true))
	UDAWSequencerData* SessionData;

	//the array of tracks/channels found in the midi file
	//UPROPERTY(VisibleAnywhere, Category = "unDAW|MetaSound Builder Helper")
	//TMap<int, FTrackDisplayOptions>* MidiTracks;
	
	UFUNCTION(BlueprintImplementableEvent)
	void PerformBpInitialization();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "unDAW|MetaSound Builder Helper")
	TSet<FName> MidiOutputNames;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "unDAW|MetaSound Builder Helper")
	EMetaSoundOutputAudioFormat OutputFormat;

	//don't need this as a variable 
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper")
	UMetaSoundBuilderSubsystem* MSBuilderSystem;

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper")
	UMetaSoundSourceBuilder* CurrentBuilder;


	//The generated metasound can be shown here but it's recommended to never open the graph in the asset editor as further changes via the builder will crash the editor
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper");
	TScriptInterface<IMetaSoundDocumentInterface> GeneratedMetaSound;

	FMusicTimestamp CurrentTimestamp;

	void RemoveFromParent()
	{
		OnDeleted.Broadcast();
	}

	UFUNCTION()
	void SendSeekCommand(float InSeek);


	// binding to generator outputs, used to monitor transport and midi outputs
	UFUNCTION()
	void ReceiveMetaSoundMidiStreamOutput(FName OutputName, const FMetaSoundOutput Value);

	UFUNCTION()
	void ReceiveMetaSoundMidiClockOutput(FName OutputName, const FMetaSoundOutput Value);

	UFUNCTION()
	void ReceiveAudioParameter(FAudioParameter Parameter);


	void PopulateAssignableOutputsArray(TArray<FAssignableAudioOutput>& OutAssignableOutputs, const TArray<FMetaSoundBuilderNodeInputHandle> InMixerNodeInputs);

	UFUNCTION()
	void CreateMixerNodesSpaghettiBlock(); //very ugly

	UPROPERTY(VisibleAnywhere, Category = "unDAW", Transient)
	TMap<UM2SoundVertex*, FMetaSoundNodeHandle> VertexToNodeMap;

	TArray<FMetaSoundNodeHandle> BuilderNodesToRelease;

	void AttachAnotherMasterMixerToOutput();

	FAssignableAudioOutput GetFreeAudioOutputAssignable();

	UPROPERTY()
	TArray<FAssignableAudioOutput> MasterOutputs;

	//TArray<FMetaSoundBuilderNodeInputHandle> MasterOutputsArray;

	UFUNCTION()
	bool CreateMidiPlayerBlock();

	UFUNCTION(CallInEditor, Category = "unDAW|MetaSound Builder Helper")
	void CreateAndAuditionPreviewAudioComponent();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "unDAW|Created Nodes")
	FMetaSoundNodeHandle TriggerToTransportNode;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "unDAW|Created Nodes")
	FMetaSoundNodeHandle MidiPlayerNode;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "unDAW|Audition Component")
	UAudioComponent* AuditionComponentRef;

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|Audition Component")
	UMetasoundGeneratorHandle* GeneratorHandle;

	bool bShouldTick = false;

	//to avoid loading this asset whenever we create an input we will cache it here
	UPROPERTY()
	TScriptInterface<IMetaSoundDocumentInterface> MidiFilterDocument;


	//This call back gets invoked when the metasound is ready to be played
	UFUNCTION()
	void OnMetaSoundGeneratorHandleCreated(UMetasoundGeneratorHandle* Handle);

	void SetupFusionNode(FTrackDisplayOptions& TrackRef);

	void ConnectTransportPinsToInterface(FMetaSoundNodeHandle& TransportNode);

	//the performer will tick to update the transport and receive metasound outputs when the game is NOT running, when the game is running we will use the MetasoundWatchOutput subsystem to receive the outputs.
	void Tick(float DeltaTime) override;

	virtual bool IsTickable() const { return bShouldTick; }

	virtual ETickableTickType GetTickableTickType() const override
	{
		return ETickableTickType::Conditional;
	}
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FMyTickableThing, STATGROUP_Tickables);
	}
	virtual bool IsTickableWhenPaused() const
	{
		return true;
	}
	virtual bool IsTickableInEditor() const
	{
		return true;
	}
};

