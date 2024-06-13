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
#include "M2SoundGraphData.h"
#include "M2SoundGraphRenderer.generated.h"

DECLARE_MULTICAST_DELEGATE(FDAWPerformerReady);
DECLARE_MULTICAST_DELEGATE(FDAWPerformerDeleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMusicTimestampUpdated, FMusicTimestamp, NewTimestamp);

DECLARE_DELEGATE_OneParam(FOnTransportSeekCommand, float)
DECLARE_DELEGATE_OneParam(FOnMusictimestampFromPerformer, FMusicTimestamp)



//struct container for runtime vertex data
//this is used to keep track of the vertexes and their assosciated builder nodes in a flexible manner
USTRUCT()
struct FBuilderVertexCompositeData
{
	GENERATED_BODY()

	UPROPERTY()
	FMetaSoundNodeHandle NodeHandle;

	UPROPERTY()
	TArray<FMetaSoundBuilderNodeOutputHandle> OutputHandles;

	UPROPERTY()
	TArray<FMetaSoundBuilderNodeInputHandle> InputHandles;

	UPROPERTY()
	FAssignableAudioOutput AudioOutputs;

};


// think about what actually gets BP exposes, probably this class should be hidden from the user


/**
 * This class is effectively the 'performer' for DAW Sequencer data. It is responsible for creating the necessary nodes and connections to play back the midi data in the sequencer data.
 */
UCLASS(BlueprintType, Blueprintable, Deprecated)
class BKMUSICCORE_API UDEPRECATED_M2SoundGraphRenderer : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:


	//~UM2SoundGraphRenderer();



	//Main entry point for the performer to start building the nodes and connections from the vertexes in the sequencer data
	void InitPerformer();

	//This function may be called when the performer is initialized and ready to play, it can save the CURRENT STATE of the builder graph into a new metasound asset
	void SaveMetasoundToAsset();



	//this function should be called on a every Vertex as it ibeing changed or created, will establish pointers from the vertex to the actual metasound node i/os.
	UFUNCTION(BlueprintCallable, Category = "unDAW")
	void UpdateVertex(UM2SoundVertex* Vertex);

	UFUNCTION(BlueprintCallable, Category = "unDAW")
	void UpdateVertexConnections(UM2SoundVertex* Vertex);

	//UFUNCTION(BlueprintCallable, Category = "unDAW")
	//void SendTransportCommand(EBKTransportCommands Command);

	UFUNCTION(BlueprintImplementableEvent)
	void PerformBpInitialization();

	void RemoveFromParent();

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
	void CreateMainMixer(); //very ugly

	//This is used to connect an audio output to a mixer node, right now we only have the master mixer node but soon we'll support busses
	void AttachAnotherMasterMixerToOutput();

	//
	FAssignableAudioOutput GetFreeAudioOutputAssignable();

	//FAssignableAudioOutput GetFreeAudioOutputFromBus();


	UFUNCTION(CallInEditor, Category = "unDAW|MetaSound Builder Helper")
	void CreateAndAuditionPreviewAudioComponent();

 
	// meta and hidden blocks creation - the m2sound graph relies on the existance of a few 'underlying nodes' that are hidden from the user, these are created on init
	bool CreateMidiPlayerBlock();


	// vertex book keeping - the renderer needs to keep track of vertexes and their assosciated builder nodes, which may use additional abstractions
	void AddVertex(UM2SoundVertex* Vertex);



	//This call back gets invoked when the metasound is ready to be played
	UFUNCTION()
	void OnMetaSoundGeneratorHandleCreated(UMetasoundGeneratorHandle* Handle);


	//After the performer has been created it can be supplied an audio component to audition the output, when called from editor we supply the preview editor component and run some extra to stop other performers
	//@param InComponent The audio component to audition the output, if null the performer will not create an auditionable metasound
	//@param bReceivesLiveUpdates If true the metasound NODEs will be updatable when the metasound is playing, otherwise it will be static
	void CreateAuditionableMetasound(UAudioComponent* InComponent, bool bReceivesLiveUpdates);

	void CreateVertex(UM2SoundVertex* Vertex);

	//inherited from FTickableGameObject
//the performer will tick to update the transport and receive metasound outputs when the game is NOT running, when the game is running we will use the MetasoundWatchOutput subsystem to receive the outputs.
	void Tick(float DeltaTime) override;

	virtual bool IsTickable() const;

	virtual ETickableTickType GetTickableTickType() const override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickableWhenPaused() const;
	virtual bool IsTickableInEditor() const;

	//end of inherited from FTickableGameObject


	//member delegates
	FOnMetasoundOutputValueChangedNative OnMidiStreamOutputReceived;
	FOnMetasoundOutputValueChangedNative OnMidiClockOutputReceived;

	FOnTransportSeekCommand OnSeekEvent;

	FDAWPerformerDeleted OnDeleted;

	FMusicTimestampUpdated OnTimestampUpdated;

	FOnMusictimestampFromPerformer OnMusicTimestampFromPerformer;

	//IBK_MusicSceneManagerInterface* ParentSceneManager;

	TEnumAsByte<EBKPlayState> PlayState;

	FDAWPerformerReady OnDAWPerformerReady;



	FMetaSoundBuilderNodeOutputHandle MainMidiStreamOutput;

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper")
	FMetaSoundBuilderNodeOutputHandle OnPlayOutputNode;

	TArray<FMetaSoundBuilderNodeInputHandle> AudioOuts;

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper")

	TMap<UM2SoundVertex*, FAssignableAudioOutput> AudioOutsMap;

	//Name of this class TBD but this is the core data structure that is used to build the builder graph
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper", meta = (ExposeOnSpawn = true))
	UDAWSequencerData* SessionData;



	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "unDAW|MetaSound Builder Helper")
	TSet<FName> MidiOutputNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "unDAW|MetaSound Builder Helper")
	EMetaSoundOutputAudioFormat OutputFormat;

	//don't need this as a variable
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper")
	UMetaSoundBuilderSubsystem* MSBuilderSystem;

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper")
	UMetaSoundSourceBuilder* BuilderContext;

	//The generated metasound can be shown here but it's recommended to never open the graph in the asset editor as further changes via the builder will crash the editor
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper");
	TScriptInterface<IMetaSoundDocumentInterface> GeneratedMetaSound;

	FMusicTimestamp CurrentTimestamp;


	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "unDAW|Audition Component")
	UAudioComponent* AuditionComponentRef;


public:

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|Audition Component")
	UMetasoundGeneratorHandle* GeneratorHandle;

	UPROPERTY()
	TScriptInterface<IMetaSoundDocumentInterface> MidiFilterDocument;

	UPROPERTY(VisibleAnywhere)
	TMap<UM2SoundVertex*, FBuilderVertexCompositeData> VertexToBuilderDataMap;

private:
	TSet<UM2SoundVertex*> InitializedVertexes;



	TMap<UM2SoundVertex*, FMetaSoundNodeHandle> VertexToNodeMap;

	TArray<FMetaSoundNodeHandle> BuilderNodesToRelease;




	TArray<FAssignableAudioOutput> MasterOutputs;

	//TArray<FMetaSoundBuilderNodeInputHandle> MasterOutputsArray;



	FMetaSoundNodeHandle TriggerToTransportNode;


	FMetaSoundNodeHandle MidiPlayerNode;


	bool bShouldTick = false;

	//to avoid loading this asset whenever we create an input we will cache it here



	//member variables

	//Special case for now, we want to easily find the midi stream outputs but this is not flexible 
	TMap<UM2SoundVertex*, FMetaSoundBuilderNodeOutputHandle> MidiStreamTracksNodeOutputHandles;


};
