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
DECLARE_MULTICAST_DELEGATE_OneParam(FMusicTimestampUpdated, FMusicTimestamp);

DECLARE_DELEGATE_OneParam(FOnTransportSeekCommand, float);

/**
 * This class is effectively the 'performer' for DAW Sequencer data. It is responsible for creating the necessary nodes and connections to play back the midi data in the sequencer data.
 */
UCLASS(BlueprintType, Blueprintable)
class BKMUSICCORE_API UDAWSequencerPerformer : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:

	FOnMetasoundOutputValueChangedNative OnMidiStreamOutputReceived;
	FOnMetasoundOutputValueChangedNative OnMidiClockOutputReceived;

	FOnTransportSeekCommand OnSeekEvent;

	FDAWPerformerDeleted OnDeleted;

	FMusicTimestampUpdated OnTimestampUpdated;

	//IBK_MusicSceneManagerInterface* ParentSceneManager;

	TEnumAsByte<EBKPlayState> PlayState;

	FDAWPerformerReady OnDAWPerformerReady;

	UFUNCTION(BlueprintCallable, Category = "unDAW")
	void SendTransportCommand(EBKTransportCommands Command);

	UPROPERTY()
	UWorld* ParentWorld;

	FMetaSoundBuilderNodeOutputHandle MainMidiStreamOutput;

	UPROPERTY(BlueprintReadWrite, Category = "unDAW|MetaSound Builder Helper")
	UAudioComponent* AuditionComponent;
	
	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper")
	FMetaSoundBuilderNodeOutputHandle OnPlayOutputNode;

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper")

	TArray<FMetaSoundBuilderNodeInputHandle> AudioOuts;

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper", meta = (ExposeOnSpawn = true))
	UDAWSequencerData* SessionData;

	//the array of tracks/channels found in the midi file
	//UPROPERTY(VisibleAnywhere, Category = "unDAW|MetaSound Builder Helper")
	TMap<int, FTrackDisplayOptions>* MidiTracks;
	
	UFUNCTION(BlueprintImplementableEvent)
	void PerformBpInitialization();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "unDAW|MetaSound Builder Helper")
	TSet<FName> MidiOutputNames;

	//this is the main entry point for the performer to start building the nodes and connections
	UFUNCTION(BlueprintCallable, Category = "unDAW|MetaSound Builder Helper", CallInEditor)
	void InitBuilderHelper(FString BuilderName, UAudioComponent* InAuditionComponent);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "unDAW|MetaSound Builder Helper")
	EMetaSoundOutputAudioFormat OutputFormat;

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper")
	UMetaSoundBuilderSubsystem* MSBuilderSystem;

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper")
	UMetaSoundSourceBuilder* CurrentBuilder;

	//can be used to get all assets of certain class
	template<typename T>
	static void GetObjectsOfClass(TArray<T*>& OutArray);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "unDAW|MetaSound Builder Helper")
	static TArray<UMetaSoundSource*> GetAllMetasoundSourcesWithInstrumentInterface();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "unDAW|MetaSound Builder Helper")
	static TArray<UMetaSoundPatch*> GetAllMetasoundPatchesWithInstrumentInterface();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "unDAW|MetaSound Builder Helper")
	static TArray<UMetaSoundPatch*> GetAllMetasoundPatchesWithInsertInterface();


	UPROPERTY(BlueprintReadOnly, Category = "unDAW|MetaSound Builder Helper");
	TScriptInterface<IMetaSoundDocumentInterface> GeneratedMetaSound;

	FMusicTimestamp CurrentTimestamp;


	UFUNCTION()
	void CreateAndRegisterMidiOutput(FTrackDisplayOptions& TrackRef);

	UFUNCTION()
	void CreateFusionPlayerForMidiTrack();
	
	UFUNCTION()
	void GenerateMidiPlayerAndTransport();

	UFUNCTION()
	void CreateCustomPatchPlayerForMidiTrack();

	void RemoveFromParent()
	{
		OnDeleted.Broadcast();
	}

	UFUNCTION()
	void ChangeFusionPatchInTrack(int TrackIndex, UFusionPatch* NewPatch);

	UFUNCTION()
	void SendSeekCommand(float InSeek);

	UFUNCTION()
	void ReceiveMetaSoundMidiStreamOutput(FName OutputName, const FMetaSoundOutput Value);

	UFUNCTION()
	void ReceiveMetaSoundMidiClockOutput(FName OutputName, const FMetaSoundOutput Value);



//#define WITH_METABUILDERHELPER_TESTS
#ifdef WITH_METABUILDERHELPER_TESTS


	void CreateTestWavPlayerBlock();

#endif //WITH_TESTS





	UFUNCTION()
	void CreateInputsFromMidiTracks();


	UFUNCTION()
	void CreateMixerPatchBlock(); //doesn't work

	UFUNCTION()
	void CreateMixerNodesSpaghettiBlock(); //very ugly




	void AttachAnotherMasterMixerToOutput();
	//returns an array of two free audio outputs, should create ones if we're missing
	TArray<FMetaSoundBuilderNodeInputHandle> GetFreeAudioOutput();

	TArray<FMetaSoundBuilderNodeInputHandle> MasterOutputsArray;

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

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "unDAW|Audition Component")
	UMetasoundGeneratorHandle* GeneratorHandle;

	bool bShouldTick = false;

	UFUNCTION()
	void OnMetaSoundGeneratorHandleCreated(UMetasoundGeneratorHandle* Handle);

	void SetupFusionNode(FTrackDisplayOptions& TrackRef);

	void ConnectTransportPinsToInterface(FMetaSoundNodeHandle& TransportNode);

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

template<typename T>
inline void UDAWSequencerPerformer::GetObjectsOfClass(TArray<T*>& OutArray)
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	AssetRegistryModule.Get().GetAssetsByClass(T::StaticClass()->GetClassPathName(), AssetData);
	for (int i = 0; i < AssetData.Num(); i++) {
		T* Object = Cast<T>(AssetData[i].GetAsset());
		OutArray.Add(Object);
	}
}
