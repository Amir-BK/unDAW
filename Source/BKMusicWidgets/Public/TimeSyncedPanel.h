// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HarmonixMidi/MidiEvent.h"
#include "UObject/Interface.h"
#include "TrackPlaybackAndDisplayOptions.h"
#include "Engine/DataAsset.h"
#include "GenericPlatform/GenericPlatform.h"
#include <BK_MusicSceneManagerInterface.h>
#include <HarmonixDsp/FusionSampler/FusionPatch.h>
#include <HarmonixDsp/Public/HarmonixDsp/FusionSampler/FusionPatch.h>
#include <TimeSyncedPanel.Generated.h>

// This class does not need to be modified.

//DECLARE_DELEGATE_OneParam(FOnTransportChanged, EBKPlayState, newTransportState);
//DECLARE_DELEGATE_OneParam(FOnPianoRollSeekChanged, float, newSeekValue)

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNeedReinit);


UENUM(BlueprintType)
/// <summary>
/// Helper enum to map keyboard actions to widget actions 
/// </summary>
enum EBKKeyActions : uint8
{
	NoAction,
	DrawModeSwitch,
	SeekMode,
	SelectMode,
	LockAxis,
	ToggleQuantization,
	IncreaseQuantizationSnap,
	DecreaseQuantizationSnap,
	IncreaseNoteDrawDuration,
	DecreaseNoteDrawDuration,
	MainViewSwitch,
	PlayPause,
	StopPlayback,
	ToggleFollowCursor,
	MuteSelectedTrack,
	SoloSelectedTrack,
	ToggleClick

};

//Data asset class used to make key binds customizable for should probably only be invoked by the editor widget...
UCLASS(BlueprintType)
class BKMUSICWIDGETS_API UBKEditorUtilsKeyboardMappings : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BK Music|Interface")
	TMap<FKey, TEnumAsByte<EBKKeyActions>> KeyMap;

};

//TODO: needs major refactoring 
UENUM(BlueprintType)
enum EPianoRollEditorMouseMode
{
	drawNotes,
	notesSelect,
	pan,
	zoom,
	seek,
	empty

};

// Helper struct used to construct note on note off pairs, I can see how this might evolve to be a unique identifier for each note, later allowing us
// to directly manipulate the linked events within the source midi file.

// Linked notes logic sorting based on the midifile lib by Craig Stuart Sapp 
// Filename:      midifile/include/MidiEventList.h
// Website:       http://midifile.sapp.org

//USTRUCT()
struct FLinkedMidiEvents
{
	FLinkedMidiEvents(const FMidiEvent& StartEvent, const FMidiEvent& EndEvent, const int32 inStartIndex, const int32 inEndindex)
		: StartEvent(StartEvent),
		  EndEvent(EndEvent),
		StartIndex(inStartIndex),
		EndIndex(inEndindex)
		
	{
	}

	//GENERATED_BODY()


	FMidiEvent StartEvent;
	FMidiEvent EndEvent;
	int32 StartIndex;
	int32 EndIndex;
};

//An interface allowing content slate widgets to pass info back to the parent widget which may sync multiple widgets
UINTERFACE(BlueprintType)
class BKMUSICWIDGETS_API UTimeSyncedPanel : public UInterface
{
	GENERATED_BODY()
};


class BKMUSICWIDGETS_API ITimeSyncedPanel
{
	GENERATED_BODY()
protected:
	//~ITimeSyncedPanel() = default;

public:
	
	//UPROPERTY(BlueprintAssignable)
	//FOnTransportChanged TransportStateChanged;

	bool SnapToGrid;
	int CurrentSelectionIndex = 0;

	//virtual FOnTrackMidiEvent GetMidiEventDelegate() {};
	


	float currentTimelineCursorPosition = 0;

	virtual void SetCurrentPosition(float newCursorPosition) {};

	virtual FTrackDisplayOptions& GetTracksDisplayOptions(int ID) = 0;
	virtual void AddHorizontalOffset(float deltaX) {};
	virtual void UpdateTemporalZoom(float newTemporalZoom, const FVector2D& WidgetSpaceZoomOrigin) {};
	virtual void UpdateZoomVector(FVector2D newZoomVector, const FVector2D& WidgetSpaceZoomOrigin) {};
	virtual void ResizePanel(int panelID, int DeltaSize) {};
	virtual void ToggleTrackVisibility(int trackID, bool inIsVisible) {};
	virtual void SelectTrack(int trackID) {};
	virtual void SetInputMode(EPianoRollEditorMouseMode newMode) {};
	virtual void AddDeltaToTimeLine(float inDelta) {};
	virtual TEnumAsByte<EPianoRollEditorMouseMode> getCurrentInputMode() { return EPianoRollEditorMouseMode::empty; };
	

	/// <summary>
	/// Gets the follow cursor.
	/// </summary>
	/// <returns>success</returns>
	virtual bool getFollowCursor() { return false; };
	virtual void setFollowCursor(bool inFollowCursor) {};

};

