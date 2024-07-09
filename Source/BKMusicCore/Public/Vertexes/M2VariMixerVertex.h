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

	//TArray<FAssignableAudioOutput*> FreeChannels;

	UFUNCTION()
	void UpdateGainParameter(int ChannelIndex, float newGain)
	{
		FName FloatName;
		EMetaSoundBuilderResult BuildResult;
		MixerChannels[ChannelIndex].GainValue = newGain;
		auto NewFloatLiteral = BuilderSubsystem->CreateFloatMetaSoundLiteral(MixerChannels[ChannelIndex].GainValue, FloatName);
		BuilderContext->SetNodeInputDefault(MixerChannels[ChannelIndex].GainParameterInputHandle, NewFloatLiteral, BuildResult);
		//garbage code...
		Cast<UM2AudioTrackPin>(InputM2SoundPins.FindRef(FName(FString::Printf(TEXT("Channel %d"), ChannelIndex))))->GainValue = newGain;
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
			}
			else {
				auto AutoNewInput = Cast<UM2AudioTrackPin>(InputM2SoundPins[TrackName]);
				AutoNewInput->ChannelIndex = i; // I guess this can help avoid mixups? I don't know
				AutoNewInput->AudioStreamL = CreateInputPin<UM2MetasoundLiteralPin>(Channel.AudioLeftOutputInputHandle);
				AutoNewInput->AudioStreamR = CreateInputPin<UM2MetasoundLiteralPin>(Channel.AudioRightOutputInputHandle);
				Channel.GainValue = AutoNewInput->GainValue;
			}

			UpdateGainParameter(i, Channel.GainValue);

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
