// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Vertexes/M2SoundVertex.h"
#include "M2SoundGraphStatics.h"
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
	uint8 numConnectedChannels = 0;

	//Need to clear all of these when the vertex is deleted
	TArray<FMetaSoundNodeHandle> MixerNodes;

	UPROPERTY(VisibleAnywhere)
	TMap<UM2SoundVertex*, FAssignableAudioOutput> VertexToChannelMap;

	UPROPERTY(VisibleAnywhere)
	TArray<FAssignableAudioOutput> MixerChannels;

	UPROPERTY()
	bool bSoloActive;

	//TArray<FAssignableAudioOutput*> FreeChannels;

	void UpdateMuteAndSoloStates()
	{
		
		bSoloActive = MixerChannels.ContainsByPredicate([](const FAssignableAudioOutput& Channel) { return Channel.AssignedPin->bSolo; });
		
		for (int i = 0 ; i <  MixerChannels.Num(); i++)
		{
			auto& Channel = MixerChannels[i];
			//Channel.MuteState = ECheckBoxState::Unchecked;
			//Channel.SoloState = ECheckBoxState::Unchecked;
			if(Channel.AssignedPin->bMute && !Channel.AssignedPin->bSolo)
			{
				//Channel.MuteState = ECheckBoxState::Checked;
				UpdateGainParam_Internal(i, 0.0f);
				continue;
			}

			if(bSoloActive && !Channel.AssignedPin->bSolo)
			{
				//Channel.MuteState = ECheckBoxState::Checked;
				UpdateGainParam_Internal(i, 0.0f);
				continue;
			}

			UpdateGainParam_Internal(i, Channel.AssignedPin->GainValue);
		}
	}

private:
	void UpdateGainParam_Internal(int ChannelIndex, float newGain)
	{
		FName FloatName;
		EMetaSoundBuilderResult BuildResult;

		auto NewFloatLiteral = BuilderSubsystem->CreateFloatMetaSoundLiteral(newGain, FloatName);
		BuilderContext->SetNodeInputDefault(MixerChannels[ChannelIndex].GainParameterInputHandle, NewFloatLiteral, BuildResult);
	}

public:

	UFUNCTION()
	void UpdateGainParameter(int ChannelIndex, float newGain)
	{
		MixerChannels[ChannelIndex].AssignedPin->GainValue = newGain;

		if(bSoloActive && !MixerChannels[ChannelIndex].AssignedPin->bSolo)
		{
			return;
		}

		if(MixerChannels[ChannelIndex].AssignedPin->bMute && !MixerChannels[ChannelIndex].AssignedPin->bSolo)
		{
			return;
		}

		UpdateGainParam_Internal(ChannelIndex, newGain);

	}

	UFUNCTION() 
	void SetChannelMuteState(int ChannelIndex, ECheckBoxState InState)
	{

		MixerChannels[ChannelIndex].AssignedPin->bMute = InState == ECheckBoxState::Checked;

		UpdateMuteAndSoloStates();

	}

	UFUNCTION()
	void SetChannelSoloState(int ChannelIndex, ECheckBoxState InState)
	{

		MixerChannels[ChannelIndex].AssignedPin->bSolo = InState == ECheckBoxState::Checked;
		UpdateMuteAndSoloStates();

	}

	void BuildVertex() override
	{
		UE_LOG(LogTemp, Warning, TEXT("Building Mixer Vertex"));
		
		BuilderResults.Empty();
		//VertexToChannelMap.Empty();
		MixerChannels.Empty();
		EMetaSoundBuilderResult BuildResult;
		BuilderSubsystem = SequencerData->MSBuilderSystem;
		BuilderContext = SequencerData->BuilderContext;


		const auto NewMixerNode = BuilderContext->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("AudioMixer")), FName(TEXT("Audio Mixer (Stereo, 8)")))
			, BuildResult);

		BuilderResults.Add("MixerNode", BuildResult);

		OutPins = BuilderContext->FindNodeOutputs(NewMixerNode, BuildResult);

		BuilderResults.Add("MixerNodeOutputs", BuildResult);

		if (OutputM2SoundPins.IsEmpty())
		{
			auto* AudioTrackPin = CreateAudioTrackOutputPin();
			AudioTrackPin->AudioStreamL = CreateOutputPin<UM2MetasoundLiteralPin>(OutPins[0]);
			AudioTrackPin->AudioStreamR = CreateOutputPin<UM2MetasoundLiteralPin>(OutPins[1]);
			OutputM2SoundPins.Add(M2Sound::Pins::AutoDiscovery::AudioTrack, AudioTrackPin);
		}
		else {
			auto AudioTrackPin = Cast<UM2AudioTrackPin>( OutputM2SoundPins[M2Sound::Pins::AutoDiscovery::AudioTrack]);
			AudioTrackPin->AudioStreamL = CreateOutputPin<UM2MetasoundLiteralPin>(OutPins[0]);
			AudioTrackPin->AudioStreamR = CreateOutputPin<UM2MetasoundLiteralPin>(OutPins[1]);
		}


		
		//PopulatePinsFromMetasoundData(InPins, OutPins);

		UM2SoundGraphStatics::PopulateAssignableOutputsArray(MixerChannels, BuilderContext->FindNodeInputs(NewMixerNode, BuildResult));

		int i = 0;
		for(auto& Channel : MixerChannels)
		{
			auto TrackName = FName(FString::Printf(TEXT("Channel %d"), i));
			if(InputM2SoundPins.Contains(TrackName) == false)
			{
				auto* AutoNewInput = CreateAudioTrackInputPin(TrackName);
				AutoNewInput->ChannelIndex = i;
				AutoNewInput->AudioStreamL = CreateInputPin<UM2MetasoundLiteralPin>(Channel.AudioLeftOutputInputHandle);
				AutoNewInput->AudioStreamR = CreateInputPin<UM2MetasoundLiteralPin>(Channel.AudioRightOutputInputHandle);
				InputM2SoundPins.Add(TrackName, AutoNewInput);
				Channel.AssignedPin = AutoNewInput;
			}
			else {
				auto AutoNewInput = Cast<UM2AudioTrackPin>(InputM2SoundPins[TrackName]);
				AutoNewInput->ChannelIndex = i; // I guess this can help avoid mixups? I don't know
				AutoNewInput->AudioStreamL = CreateInputPin<UM2MetasoundLiteralPin>(Channel.AudioLeftOutputInputHandle);
				AutoNewInput->AudioStreamR = CreateInputPin<UM2MetasoundLiteralPin>(Channel.AudioRightOutputInputHandle);
				MixerChannels[i].AssignedPin = AutoNewInput;
				UpdateGainParameter(i, AutoNewInput->GainValue);
				//Channel.GainValue = AutoNewInput->GainValue;
			}

			

			i++;

		}

		//if(VertexToChannelMap.Num() > 0)
		//{
		//	TMap<UM2SoundVertex*, FAssignableAudioOutput> NewMap;
		//	for (auto& [Vertex, Channel] : VertexToChannelMap)
		//	{
		//		if (MixerChannels.Num() > 0)
		//		{
		//			NewMap.Add(Vertex, MixerChannels.Pop());
		//		}
		//	}
		//	
		//	VertexToChannelMap = NewMap;

		//	//UpdateConnections();
		//}

	}



	FLinearColor GetChannelColor(uint8 ChannelIndex)
	{
		return GetSequencerData()->GetTracksDisplayOptions(ChannelIndex).trackColor;
	}



	FAssignableAudioOutput CreateChannel()
	{
		FAssignableAudioOutput NewChannel;
		//MixerChannels.Add(NewChannel);
		numConnectedChannels++;
		return NewChannel;
	}

	void ReleaseChannel(FAssignableAudioOutput Channel)
	{
		//MixerChannels.Remove(Channel);
		numConnectedChannels--;
	}

	void ResizeOutputMixer()
	{
		EMetaSoundBuilderResult BuildResult;
		const auto NewMixerNode = BuilderContext->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("AudioMixer")), FName(TEXT("Audio Mixer (Stereo, 8)")))
			, BuildResult);
	}

};
