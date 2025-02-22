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


public:

	UMetaSoundBuilderSubsystem* BuilderSubsystem;
	UMetaSoundSourceBuilder* BuilderContext;
	//Mixer aliases are the connection point for anything we add to the graph in runtime, the vari mixer will create and remove input channels per request
	UPROPERTY(EditAnywhere, Category = "M2Sound | Mixer")
	FName MixerAlias = NAME_None;

	uint8 NumConnectedChannels = 0;

	//Need to clear all of these when the vertex is deleted
	TArray<FMetaSoundNodeHandle> MixerNodes;

	UPROPERTY(VisibleAnywhere)
	TMap< TObjectPtr<UM2SoundVertex>, FAssignableAudioOutput> VertexToChannelMap;

	UPROPERTY(VisibleAnywhere)
	TArray<FAssignableAudioOutput> MixerChannels;

	UPROPERTY()
	bool bSoloActive;

	//TArray<FAssignableAudioOutput*> FreeChannels;

	UFUNCTION(BlueprintCallable, Category = "M2Sound | Mixer")
	int AttachM2VertexToMixerInput(UM2SoundVertex* InVertex, float InVolume);

	UFUNCTION()
	void SetMixerAlias(FName InAlias);

	void UpdateMuteAndSoloStates();

	void SetExclusiveSoloForPin(UM2AudioTrackPin* InPin);

private:
	void UpdateGainParam_Internal(int ChannelIndex, float newGain);

	void UpdateGainParamForPin_Internal(UM2AudioTrackPin* InPin, float newGain);

	//UPROPERTY()
	//TArray<TObjectPtr<UM2

public:


	UM2AudioTrackPin* CreateMixerInputPin();

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
