// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <SPianoRollGraph.h>
#include <TimeSyncedPanel.h>
#include "Widgets/Colors/SColorPicker.h"
#include "Quartz/QuartzSubsystem.h"
#include "EngravingSubsystem.h"
#include "TrackPlaybackAndDisplayOptions.h"
#include "HarmonixMidi/MidiFile.h"
#include "HarmonixMidi/MusicTimeSpan.h"
#include "Engine/DataAsset.h"
#include "Interfaces/MetasoundOutputFormatInterfaces.h"
#include "MIDIEditorBase.generated.h"

BK_EDITORUTILITIES_API DECLARE_LOG_CATEGORY_EXTERN(BKMidiLogs, Verbose, All);
 
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPanelPopulated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSeekEvent, float, NewSeek);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTrackSelectedEvent, bool, selected, int, trackID);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGridQuantizationUnitChanged, EMusicTimeSpanOffsetUnits, NewQuantizationUnit);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlaybackStateChanged, EBKPlayState, NewPlaystate);

/**
* This is the 
*/

USTRUCT(BlueprintType)
struct FMidiEventBlueprintWrapper {

	GENERATED_BODY()

	FMidiMsg DataPayload;
};




class SColorPicker;

USTRUCT(BlueprintType, Category = "BK Music|MIDI")
struct FTrackPlaybackData
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, Category = "BK Music|MIDI")
	TArray<FTrackDisplayOptions> OptionsStruct;

};




UCLASS(BlueprintType, Category = "BK Music|MIDI", EditInlineNew)
class BK_EDITORUTILITIES_API UMIDITrackCache : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unDAW|Editor Cache", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float MasterVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unDAW|Editor Cache")
	EMetaSoundOutputAudioFormat OutputFormat = EMetaSoundOutputAudioFormat::Stereo;

	UPROPERTY(EditAnywhere, Category = "unDAW|Editor Cache")
	TMap<FName, FTrackPlaybackData> TracksData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unDAW|Editor Cache")
	TObjectPtr<UMidiFile> lastUsedMidiFile;

	UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "unDAW|Editor Cache", meta = (ExposeOnSpawn = "true", EditInLine = "true"))
	TMap<FName, UDAWSequencerData*> CachedSessions;

	


};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrackMidiEvent, FMidiEventBlueprintWrapper, MidiData);

/**
 * The midi editor base is the root for widgets that load a UMIDIAsset and display them via pianoroll graphs
 */
UCLASS()
class BK_EDITORUTILITIES_API UMIDIEditorBase : public UWidget, public ITimeSyncedPanel
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = SetWorldContextObject, Category = "BK Music|MIDI")
	UObject* WorldContextObject;

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category = "BK Music|MIDI")
	void SetWorldContextObject(UObject* InWorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "unDAW|SceneManager")
	void SetSceneManager(TScriptInterface<IBK_MusicSceneManagerInterface> InSceneManager);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BK Music|MIDI")
	TArray<FTrackDisplayOptions> tracksDisplayOptions;

	UPROPERTY(BlueprintReadWrite, BlueprintSetter = SetPerformanceComponent, Category = "BK Music|MIDI")
	UAudioComponent* PerformanceComponent;

	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly, Category = "BK Music|MIDI")
	void SetPerformanceComponent(UAudioComponent* InPerformanceComponent);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Harmonix Midi", Category = "BK Music|MIDI")
	TWeakObjectPtr<UMidiFile> HarmonixMidiFile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "MidiEditorCache", Category = "BK Music|MIDI")
	TObjectPtr<UMIDITrackCache> MidiEditorCache;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Key Map", Category = "BK Music|MIDI|Interface")
	TObjectPtr<UBKEditorUtilsKeyboardMappings> KeyMapDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BK Music|MIDI|Interface")
	TArray<float> panelSlotSizes;

	

	
	UPROPERTY(BlueprintAssignable, Category = "BK Music|MIDI|Internal")
	FOnPanelPopulated donePopulatingDelegate;

	//event delegates

	UPROPERTY(BlueprintAssignable, Category = "BK Music|MIDI|Interface")
	FOnTrackMidiEvent MidiEventDelegate;

	
	UPROPERTY(BlueprintAssignable , Category = "BK Music|MIDI|Interface")
	FOnSeekEvent SeekEventDelegate;

	UPROPERTY(BlueprintAssignable , Category = "BK Music|MIDI|Interface")
	FOnTrackSelectedEvent OnTrackSelectedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "BK Music|Transport")
	FOnGridQuantizationUnitChanged GridChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "BK Music|MIDI|")
	FOnPlaybackStateChanged PlaybackStateDelegate;

	//TSharedPtr<class SPianoRollGraph> PianoRollGraph

	//begin ITimeSyncedPanel interface
	void AddHorizontalOffset(float deltaX) override;
	void UpdateTemporalZoom(float newTemporalZoom, const FVector2D& WidgetSpaceZoomOrigin) override;
	virtual void ResizePanel(int panelID, int DeltaSize) override;
	void ToggleTrackVisibility(int trackID, bool inIsVisible) override;
	virtual void SelectTrack(int trackID) override;
	virtual FTrackDisplayOptions& GetTracksDisplayOptions(int ID) override;

	virtual void SetCurrentPosition(float newCursorPosition) override;

	virtual TEnumAsByte<EPianoRollEditorMouseMode> getCurrentInputMode() override;
	//End ItimeSyncedPanel interface

	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI|Interface")
	void SetInputMode(EPianoRollEditorMouseMode newMode) override;

	

	FLinearColor TrackColorPickerClicked(int indexInTrackOptionsArray);
	FReply TrackVisibilityClicked(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	TMap<int32, UFusionPatch*> GetTracksMap();

	// Must be called after making any changes that require rebuilding the metasound
	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	void InitAudioBlock();

	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	UPARAM(ref) TArray<FTrackDisplayOptions>& GetTrackDisplayOptions();

	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	void ExecuteAudioParamOnPerformanceComponent(FString InName, float inValue);

	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	void UpdateElementInDisplayOptions(int ElementID, UPARAM(ref) FTrackDisplayOptions& InTrackOptions);

	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	void InitFromDataHarmonix();

	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	void UpdateDataAsset();
	
	UPROPERTY()
	bool bFollowCursor = false;

	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	virtual bool getFollowCursor() override;

	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	virtual void setFollowCursor(bool inFollowCursor) override;

	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	void SetTransportPlayState(EBKPlayState newPlayState);




	UPROPERTY(EditAnywhere, Category = "BK Music|MIDI")
	FSlateBrush GridBrush;

	UPROPERTY(EditAnywhere, Category = "BK Music|MIDI")
	FLinearColor gridLineColor;

	UPROPERTY(EditAnywhere, Category = "BK Music|MIDI")
	FLinearColor accidentalGridLineColor;

	UPROPERTY(EditAnywhere, Category = "BK Music|MIDI")
	FLinearColor noteColor;
	
	UPROPERTY(EditAnywhere, Category = "BK Music|MIDI")
	FLinearColor cNoteColor;

	UPROPERTY(EditAnywhere, Category = "BK Music|MIDI")
	bool multiPanelMode = false;

	//public blue print functions should be moved here

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BK Music|MIDI")
	float mainBPM = 60;

	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	void SetCurrentTimelinePosition(float inPosition);
	
	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	void AddDeltaToTimeLine(float inDelta) override;
	
	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	float getCurrentTimelinePosition();

	/** Please add a function description */
	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	void UpdatePatchInTrack(int TrackID, const TScriptInterface<IMetaSoundDocumentInterface> MidiPatchClass);

	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	void UpdateVolumeInTrack(int TrackID, float newGain);

	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	UPARAM(ref) FTrackDisplayOptions& GetTrackOptionsRef(int TrackID);

	//call to set grid quantization
	UFUNCTION(BlueprintCallable, Category = "BK Music|MIDI")
	void SetGridQuantization(EMusicTimeSpanOffsetUnits newQuantization);



	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true), Category = "BK Music|MIDI")
	EMetaSoundOutputAudioFormat OutputFormat;

protected:

	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;


	//~ End UWidget Interface

	//this is a reference to the actor implementing the music scene manager interface, if this is null we'll be playing in preview only
	UPROPERTY()
	TScriptInterface<IBK_MusicSceneManagerInterface> SceneManager;

	TArray<FMidiEvent> TempoEvents;
	TArray<FMidiEvent> TimeSignatureEvents;

	bool bSnapToQuantizationBorder = true;

	TSharedPtr<ITimeSyncedPanel> MidiEditorSharedPtr;
	TWeakObjectPtr<ITimeSyncedPanel> MidiEditorWeakObjPtr;

	//the value used to snap and to display the grid, might be seperated to two values 
	EMusicTimeSpanOffsetUnits GridQuantizationUnit = EMusicTimeSpanOffsetUnits::Ms;


	TSharedPtr<SVerticalBox> tracksVerticalBox;

	TSharedPtr<SScrollBox> panelMainScrollArea;


	//currently this is only used to host the one pianoroll graph so it's kind of pointless but the idea is to host other panels
	TArray<TSharedRef<SPianoRollGraph>> InternalGraphs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BK Music|MIDI|Interface")
	TEnumAsByte<EPianoRollEditorMouseMode> inputMode;

	
	
};
