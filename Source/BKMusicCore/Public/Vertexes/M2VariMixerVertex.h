// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Vertexes/M2SoundVertex.h"
#include "M2SoundGraphStatics.h"
#include "Styling/SlateTypes.h"
#include "M2VariMixerVertex.generated.h"

/**
 *
 */
UCLASS()
class BKMUSICCORE_API UM2VariMixerVertex : public UM2SoundVertex
{
	GENERATED_BODY()

	UMetaSoundBuilderSubsystem* BuilderSubsystem;
	UMetaSoundSourceBuilder* BuilderContext;

public:
	uint8 NumConnectedChannels = 0;

	//Need to clear all of these when the vertex is deleted
	TArray<FMetaSoundNodeHandle> MixerNodes;

	UPROPERTY(VisibleAnywhere)
	TMap<UM2SoundVertex*, FAssignableAudioOutput> VertexToChannelMap;

	UPROPERTY(VisibleAnywhere)
	TArray<FAssignableAudioOutput> MixerChannels;

	UPROPERTY()
	bool bSoloActive;

	//TArray<FAssignableAudioOutput*> FreeChannels;

	void UpdateMuteAndSoloStates();

private:
	void UpdateGainParam_Internal(int ChannelIndex, float newGain);

public:

	UFUNCTION()
	void UpdateGainParameter(int ChannelIndex, float newGain);


	UFUNCTION()
	void SetChannelMuteState(int ChannelIndex, ECheckBoxState InState);

	UFUNCTION()
	void SetChannelSoloState(int ChannelIndex, ECheckBoxState InState);


	void BuildVertex() override;

	FLinearColor GetChannelColor(uint8 ChannelIndex);

	FAssignableAudioOutput CreateChannel();

	void ReleaseChannel(FAssignableAudioOutput Channel);

	void ResizeOutputMixer();
};
