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
	FLinearColor TrackColor;

	UPROPERTY(BlueprintReadWrite, Category = "BK Music|Track Settings")
	float trackZOrder;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BK Music|Track Settings")
	FString TrackName;

	UPROPERTY(BlueprintReadOnly, Category = "BK Music|Track Settings")
	int TrackIndexInParentMidi;

	UPROPERTY(BlueprintReadOnly, Category = "BK Music|Track Settings")
	int ChannelIndexInParentMidi;

	//I don't fully understand what harmonix are doing with the 'is primary channel' but this leads to different results when
	// sending notes to fusion vs. when receiving notes from the watch subsystem, to account for that, we save both values
	UPROPERTY(BlueprintReadOnly, Category = "BK Music|Track Settings")
	int ChannelIndexRaw;


	UPROPERTY()
	bool CreateMidiOutput = true;



	//Can be used to visualize the available KeyZones the used fusion patch exposes
	UPROPERTY(BlueprintReadOnly, Category = "BK Music|Track Settings")
	TMap<int, bool> SampleAvailabilityMap;

	FTrackDisplayOptions()
	{
		isVisible = true;
		TrackColor = FLinearColor::Gray;
		trackZOrder = 0.0f;
		isSelected = false;
		TrackIndexInParentMidi = INDEX_NONE;
		ChannelIndexInParentMidi = INDEX_NONE;
		TrackName = FString(TEXT("INVALID TRACK"));
	}
};