// Fill out your copyright notice in the Description page of Project Settings.
// This guy shouldn't be in the editor only module....


#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include <HarmonixDsp/FusionSampler/FusionPatch.h>
#include <MetasoundDocumentInterface.h>
#include <HarmonixDsp/Public/HarmonixDsp/FusionSampler/FusionPatch.h>
#include "UObject/ConstructorHelpers.h"
#include "TrackPlaybackAndDisplayOptions.generated.h"


UENUM(BlueprintType)
enum ETrackRendererMode : uint8
{
	FusionPatch,
	CustomPatch,
	NoAudio,

};

// struct that describes some crucial settings used to set up a MIDI track in the render metasound built by the Meta Sound Graph Builder
USTRUCT(BlueprintType, Category = "BK Music|Track Settings")
struct BKMUSICCORE_API FTrackDisplayOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "BK Music|Track Settings")
	bool isVisible;

	UPROPERTY(BlueprintReadWrite, Category = "BK Music|Track Settings")
	bool isSelected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings")
	FLinearColor trackColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings")
	float trackZOrder;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings")
	float TrackVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings")
	FString trackName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BK Music|Track Settings")
	int TrackIndexInParentMidi;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BK Music|Track Settings")
	int ChannelIndexInParentMidi;

	//the desired render mode, if Custom Patch is selected a custom metasound patch will be inserted into the graph by the builder. Make sure this patch can receive a MIDI stream and output audio, consult the example instrument for reference.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings")
	TEnumAsByte<ETrackRendererMode> RenderMode = ETrackRendererMode::FusionPatch;

	UPROPERTY()
	bool CreateMidiOutput = true;

	//the fusion patch to use in case 'Fusion Patch' mode is selected
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings", meta = (EditCondition = "RenderMode==ETrackRendererMode::FusionPatch", EditConditionHides))
	TObjectPtr<UFusionPatch> fusionPatch;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings", meta = (EditCondition = "RenderMode==ETrackRendererMode::CustomPatch", EditConditionHides))
	TScriptInterface<IMetaSoundDocumentInterface> MidiPatchClass;


	//Can be used to visualize the available KeyZones the used fusion patch exposes 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BK Music|Track Settings")
	TMap<int, bool> SampleAvailabilityMap;



	FTrackDisplayOptions()
	{
		isVisible = true;
		trackColor = FLinearColor::Blue;
		trackZOrder = 0.0f;
		isSelected = false;
		fusionPatch = nullptr;
		TrackIndexInParentMidi = INDEX_NONE;
		ChannelIndexInParentMidi = INDEX_NONE;
		trackName = FString(TEXT("INVALID TRACK"));
	}

};