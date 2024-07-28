// Fill out your copyright notice in the Description page of Project Settings.

#include "Vertexes/M2VariMixerVertex.h"

int UM2VariMixerVertex::AttachM2VertexToMixerInput(UM2SoundVertex* InVertex, float InVolume)
{
	//find first free channel, so we'll just create one as needed
	for (int i = 0; i < MixerChannels.Num(); i++)
	{
		if (MixerChannels[i].AssignedPin->LinkedPin != nullptr)
		{
			UM2AudioTrackPin* VertexOutput = Cast< UM2AudioTrackPin>(InVertex->OutputM2SoundPins[M2Sound::Pins::AutoDiscovery::AudioTrack]);
			if (VertexOutput == nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("Vertex does not have an audio track output pin"));
				return INDEX_NONE;
			}

			UM2AudioTrackPin* MixerInput = Cast<UM2AudioTrackPin>(MixerChannels[i].AssignedPin);

			bool bConnectionSuccess = GetSequencerData()->ConnectPins<UM2AudioTrackPin>(MixerInput, VertexOutput);

			UE_CLOG(!bConnectionSuccess, LogTemp, Warning, TEXT("Connection failed"));

			return i;

		}
	}

	
	
	return INDEX_NONE;
}

void UM2VariMixerVertex::SetMixerAlias(FName InAlias)
{
	//master mixer cannot be renamed
	if (MixerAlias == FName("Master"))
	{
		UE_LOG(LogTemp, Warning, TEXT("Master Mixer cannot be renamed"));
		return;
	}
	
	//check that mixer alias is unique
	if (GetSequencerData()->Mixers.Contains(InAlias))
	{
		UE_LOG(LogTemp, Warning, TEXT("Mixer Alias already exists"));
		return;
	}

	//if we have a mixer alias, remove it from the sequencer data
	if (MixerAlias != NAME_None)
	{
		GetSequencerData()->Mixers.Remove(MixerAlias);
	}

	MixerAlias = InAlias;

	//add the mixer alias to the sequencer data
	GetSequencerData()->Mixers.Add(MixerAlias, this);
}

inline void UM2VariMixerVertex::UpdateMuteAndSoloStates()
{

	//probably a bit costly, check if ANY channel has solo active, we should probably only do this when changing solo states
	auto AnySoloArray = InputM2SoundPins.FilterByPredicate([](const TPair<FName, UM2Pins*>& Pin) { return Cast<UM2AudioTrackPin>(Pin.Value)->bSolo; });

	bSoloActive = AnySoloArray.Num() > 0;

	for (auto& [Name, Pin] : InputM2SoundPins)
	{
		auto* AudioTrackPin = Cast<UM2AudioTrackPin>(Pin);
		if (AudioTrackPin == nullptr)
		{
			continue;
		}
	

		if(AudioTrackPin->bMute && !AudioTrackPin->bSolo)
		{
			//UpdateGainParam_Internal(AudioTrackPin->ChannelIndex, 0.0f);
			UpdateGainParamForPin_Internal(AudioTrackPin, 0.0f);
			continue;
		}

		if (bSoloActive && !AudioTrackPin->bSolo)
		{
			//UpdateGainParam_Internal(AudioTrackPin->ChannelIndex, 0.0f);
			UpdateGainParamForPin_Internal(AudioTrackPin, 0.0f);
			continue;
		}

		UpdateGainParamForPin_Internal(AudioTrackPin, AudioTrackPin->GainValue);

	}

}

inline void UM2VariMixerVertex::UpdateGainParam_Internal(int ChannelIndex, float newGain)
{
	FName FloatName;
	EMetaSoundBuilderResult BuildResult;

	auto NewFloatLiteral = BuilderSubsystem->CreateFloatMetaSoundLiteral(newGain, FloatName);
	BuilderContext->SetNodeInputDefault(MixerChannels[ChannelIndex].GainParameterInputHandle, NewFloatLiteral, BuildResult);
}

void UM2VariMixerVertex::UpdateGainParamForPin_Internal(UM2AudioTrackPin* InPin, float newGain)
{
	FName FloatName;
	EMetaSoundBuilderResult BuildResult;

	auto NewFloatLiteral = BuilderSubsystem->CreateFloatMetaSoundLiteral(newGain, FloatName);
	BuilderContext->SetNodeInputDefault(InPin->GainParameter->GetHandle<FMetaSoundBuilderNodeInputHandle>(), NewFloatLiteral, BuildResult);
}

UM2AudioTrackPin* UM2VariMixerVertex::CreateMixerInputPin()
{
	if(MixerChannels.Num() == 1)
	{
		//FAssignableAudioOutput NewChannel;
		//MixerChannels.Add(NewChannel);
		ResizeOutputMixer();
	}

	auto AvailableOutput = MixerChannels.Pop();

	auto TrackName = FName(FString::Printf(TEXT("Channel %d"), NumConnectedChannels++));
	if (InputM2SoundPins.Contains(TrackName) == false)
	{
		auto* AutoNewInput = CreateAudioTrackInputPin(TrackName);
		AutoNewInput->ChannelIndex = NumConnectedChannels;
		AutoNewInput->AudioStreamL = CreateInputPin<UM2MetasoundLiteralPin>(AvailableOutput.AudioLeftOutputInputHandle);
		AutoNewInput->AudioStreamR = CreateInputPin<UM2MetasoundLiteralPin>(AvailableOutput.AudioRightOutputInputHandle);
		InputM2SoundPins.Add(TrackName, AutoNewInput);
		AvailableOutput.AssignedPin = AutoNewInput;

		return AutoNewInput;
	}


	return nullptr;

}

void UM2VariMixerVertex::UpdateGainParameter(int ChannelIndex, float newGain)
{
	MixerChannels[ChannelIndex].AssignedPin->GainValue = newGain;

	if (bSoloActive && !MixerChannels[ChannelIndex].AssignedPin->bSolo)
	{
		return;
	}

	if (MixerChannels[ChannelIndex].AssignedPin->bMute && !MixerChannels[ChannelIndex].AssignedPin->bSolo)
	{
		return;
	}

	UpdateGainParam_Internal(ChannelIndex, newGain);
}

void UM2VariMixerVertex::SetChannelMuteState(int ChannelIndex, ECheckBoxState InState)
{
	MixerChannels[ChannelIndex].AssignedPin->bMute = InState == ECheckBoxState::Checked;

	UpdateMuteAndSoloStates();
}


void UM2VariMixerVertex::SetChannelSoloState(int ChannelIndex, ECheckBoxState InState)
{
	MixerChannels[ChannelIndex].AssignedPin->bSolo = InState == ECheckBoxState::Checked;
	UpdateMuteAndSoloStates();
}

inline void UM2VariMixerVertex::BuildVertex()
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

	auto MixerOutPins = BuilderContext->FindNodeOutputs(NewMixerNode, BuildResult);

	BuilderResults.Add("MixerNodeOutputs", BuildResult);

	if (OutputM2SoundPins.IsEmpty())
	{
		auto* AudioTrackPin = CreateAudioTrackOutputPin();
		AudioTrackPin->AudioStreamL = CreateOutputPin<UM2MetasoundLiteralPin>(MixerOutPins[0]);
		AudioTrackPin->AudioStreamR = CreateOutputPin<UM2MetasoundLiteralPin>(MixerOutPins[1]);
		OutputM2SoundPins.Add(M2Sound::Pins::AutoDiscovery::AudioTrack, AudioTrackPin);
	}
	else {
		auto AudioTrackPin = Cast<UM2AudioTrackPin>(OutputM2SoundPins[M2Sound::Pins::AutoDiscovery::AudioTrack]);
		AudioTrackPin->AudioStreamL = CreateOutputPin<UM2MetasoundLiteralPin>(MixerOutPins[0]);
		AudioTrackPin->AudioStreamR = CreateOutputPin<UM2MetasoundLiteralPin>(MixerOutPins[1]);
	}

	//PopulatePinsFromMetasoundData(InPins, OutPins);

	UM2SoundGraphStatics::PopulateAssignableOutputsArray(MixerChannels, BuilderContext->FindNodeInputs(NewMixerNode, BuildResult));

	//create one input channel
	//CreateChannel();

	// if input pins are empty this means this is a new vertex, create initial pin,
	// otherwise we are updating an existing vertex, so we need to update the pins

	if (InputM2SoundPins.IsEmpty())
	{
		CreateMixerInputPin();
		return;
	}

	for (auto& InputPin : InputM2SoundPins)
	{
		if (MixerChannels.Num() == 1)
		{
			//FAssignableAudioOutput NewChannel;
			//MixerChannels.Add(NewChannel);
			ResizeOutputMixer();
		}

		auto AvailableOutput = MixerChannels.Pop();
		
		auto* AutoNewInput = Cast<UM2AudioTrackPin>(InputPin.Value);
		AutoNewInput->AudioStreamL = CreateInputPin<UM2MetasoundLiteralPin>(AvailableOutput.AudioLeftOutputInputHandle);
		AutoNewInput->AudioStreamR = CreateInputPin<UM2MetasoundLiteralPin>(AvailableOutput.AudioRightOutputInputHandle);
		AutoNewInput->GainParameter = CreateInputPin<UM2MetasoundLiteralPin>(AvailableOutput.GainParameterInputHandle);
		AvailableOutput.AssignedPin = AutoNewInput;
	}



	/*
	int i = 0;
	for (auto& Channel : MixerChannels)
	{
		auto TrackName = FName(FString::Printf(TEXT("Channel %d"), i));
		if (InputM2SoundPins.Contains(TrackName) == false)
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
		}

		i++;
	}
	*/
	//UpdateMuteAndSoloStates();
}

inline FLinearColor UM2VariMixerVertex::GetChannelColor(uint8 ChannelIndex)
{
	//if (!MixerChannels.IsValidIndex(ChannelIndex))
	//{
	//	return FLinearColor::Gray;
	//}
	//if (MixerChannels[ChannelIndex].AssignedPin->LinkedPin)
	//{
	//	return MixerChannels[ChannelIndex].AssignedPin->LinkedPin->ParentVertex->GetVertexColor();
	//}

	return FLinearColor::Gray;
}

inline FAssignableAudioOutput UM2VariMixerVertex::CreateChannel()
{
	FAssignableAudioOutput NewChannel;
	//MixerChannels.Add(NewChannel);
	//NumConnectedChannels++;
	return NewChannel;
}

inline void UM2VariMixerVertex::ReleaseChannel(FAssignableAudioOutput Channel)
{
	//MixerChannels.Remove(Channel);
	NumConnectedChannels--;
}

inline void UM2VariMixerVertex::ResizeOutputMixer()
{
	EMetaSoundBuilderResult BuildResult;
	const auto NewMixerNode = BuilderContext->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("AudioMixer")), FName(TEXT("Audio Mixer (Stereo, 8)")))
		, BuildResult);

	//add the new mixer to the array so that we can remove it when destroying the node
	MixerNodes.Add(NewMixerNode);

	auto LastRemainingChannel = MixerChannels.Pop();

	//populate the channels with the new mixer node
	UM2SoundGraphStatics::PopulateAssignableOutputsArray(MixerChannels, BuilderContext->FindNodeInputs(NewMixerNode, BuildResult));

	//we need the new mixer node's outputs
	auto NewOutputs = BuilderContext->FindNodeOutputs(NewMixerNode, BuildResult);

	BuilderResults.Add("MixerNode", BuildResult);

	//connect output audio channels of new mixer to the last remaining channel
	BuilderContext->ConnectNodes(NewOutputs[0], LastRemainingChannel.AudioLeftOutputInputHandle, BuildResult);
	BuilderContext->ConnectNodes(NewOutputs[1], LastRemainingChannel.AudioRightOutputInputHandle, BuildResult);

}
