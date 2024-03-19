// Fill out your copyright notice in the Description page of Project Settings.
// This guy shouldn't be in the editor only module....


#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <HarmonixDsp/FusionSampler/FusionPatch.h>
#include <MetasoundDocumentInterface.h>
#include <HarmonixDsp/Public/HarmonixDsp/FusionSampler/FusionPatch.h>
#include "TrackPlaybackAndDisplayOptions.generated.h"


UENUM(BlueprintType)
enum ETrackRendererMode : uint8
{
	FusionPatch,
	CustomPatch,
	NoAudio
};

// struct that describes some crucial settings used to set up a MIDI track in the render metasound built by the Meta Sound Graph Builder
USTRUCT(BlueprintType, Category = "BK Music|Track Settings")
struct FTrackDisplayOptions
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings")
	bool isVisible;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings")
	bool isSelected;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings")
	FLinearColor trackColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings")
	float trackZOrder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings")
	float TrackVolume = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings")
	FString trackName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BK Music|Track Settings")
	int TrackIndexInParentMidi;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BK Music|Track Settings")
	int ChannelIndexInParentMidi;

	//Can be used to visualize the available KeyZones the used fusion patch exposes 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BK Music|Track Settings")
	TMap<int, bool> SampleAvailabilityMap;

	//the fusion patch to use in case 'Fusion Patch' mode is selected
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings")
	TObjectPtr<UFusionPatch> fusionPatch;

	/** Please add a function description */
	//UFUNCTION(BlueprintCallable)
	//void BK Meta Builder(UPARAM(ref) TArray<FTrackDisplayOptions>& Tracks, const TScriptInterface<IMetaSoundDocumentInterface>& MidiPatchClass, UObject* MidiToPlay, UObject* __WorldContext, bool& Success, UMetaSoundSourceBuilder*& Source Builder, FString& Result, UMetaSoundSourceBuilder* Builder, FMetaSoundBuilderNodeOutputHandle MidiOutputHandle, FMetaSoundBuilderNodeOutputHandle PlayInputHandle, FMetaSoundBuilderNodeOutputHandle OnPlayInput, TArray<FMetaSoundBuilderNodeInputHandle> AudioOutputs, FMetaSoundNodeHandle CurrentTrackNode, FMetaSoundBuilderNodeOutputHandle PrepareInput);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings")
	TScriptInterface<IMetaSoundDocumentInterface> MidiPatchClass;

	//the desired render mode, if Custom Patch is selected a custom metasound patch will be inserted into the graph by the builder. Make sure this patch can receive a MIDI stream and output audio, consult the example instrument for reference.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings")
	TEnumAsByte<ETrackRendererMode> RenderMode = ETrackRendererMode::FusionPatch;

	FTrackDisplayOptions()
	{
		isVisible = true;
		trackColor = FLinearColor::Blue;
		trackZOrder = 0.0f;
		isSelected = false;
		fusionPatch = nullptr;
		TrackIndexInParentMidi = -1;
		ChannelIndexInParentMidi = 0;
		trackName = FString(TEXT("No Track Name Given"));
	}

};