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

		UM2SoundGraphStatics::PopulateAssignableOutputsArray(MixerChannels, BuilderContext->FindNodeInputs(NewMixerNode, BuildResult));

		if(VertexToChannelMap.Num() > 0)
		{
			TMap<UM2SoundVertex*, FAssignableAudioOutput> NewMap;
			for (auto& [Vertex, Channel] : VertexToChannelMap)
			{
				if (MixerChannels.Num() > 0)
				{
					NewMap.Add(Vertex, MixerChannels.Pop());
				}
			}
			
			VertexToChannelMap = NewMap;

			UpdateConnections();
		}

	}

	void MakeTrackInputConnection(UM2SoundVertex* InVertex) override
	{
		//EMetaSoundBuilderResult BuildResult;
		UE_LOG(LogTemp, Warning, TEXT("Making Track Input Connection for vari mixer! exciting!"));
		
		
		//const auto NewMixerNode = BuilderContext->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("AudioMixer")), FName(TEXT("Audio Mixer (Stereo, 8)")))
		//	, BuildResult);

		//BuilderContext->ConnectNodes(InVertex->OutPins[0], NewMixerNode, BuildResult);
		if (MixerChannels.Num() > 1)
		{
			auto Channel = MixerChannels.Pop();


			VertexToChannelMap.Add(InVertex, Channel);
			InVertex->RegisterOutputVertex(this);
			UpdateConnections();

		}



	}

	void BreakTrackInputConnection() override
	{
		UE_LOG(LogTemp, Warning, TEXT("Breaking Track Input Connection for vari mixer! exciting!"));
	}

	void CollectParamsForAutoConnect() override
	{
		AutoConnectOutPins.Empty();

		//we only have two outpins, both audio 

		bool bUsedLeft = false;

		for (auto& Pin : OutPins)
		{
			if(bUsedLeft)
			{
				AutoConnectOutPins.Add(EVertexAutoConnectionPinCategory::AudioStreamR, Pin);

			}
			else
			{
				AutoConnectOutPins.Add(EVertexAutoConnectionPinCategory::AudioStreamL, Pin);
				bUsedLeft = true;
			}
		}
	}

	FLinearColor GetChannelColor(uint8 ChannelIndex)
	{
		return GetSequencerData()->GetTracksDisplayOptions(ChannelIndex).trackColor;
	}

	void UpdateConnections() override
	{
		EMetaSoundBuilderResult BuildResult;
		UE_LOG(LogTemp, Warning, TEXT("Updating Connections for Mixer Vertex"));
		for (auto& [Vertex, Channel] : VertexToChannelMap)
		{
			auto InLeft = Vertex->AutoConnectOutPins[EVertexAutoConnectionPinCategory::AudioStreamL];
			auto InRight = Vertex->AutoConnectOutPins[EVertexAutoConnectionPinCategory::AudioStreamR];

			BuilderContext->ConnectNodes(InLeft, Channel.AudioLeftOutputInputHandle, BuildResult);
			BuilderResults.Add("ConnectLeft", BuildResult);
			BuilderContext->ConnectNodes(InRight, Channel.AudioRightOutputInputHandle, BuildResult);
			BuilderResults.Add("ConnectRight", BuildResult);
		}
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
