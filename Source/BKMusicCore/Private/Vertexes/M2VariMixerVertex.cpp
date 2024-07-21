// Fill out your copyright notice in the Description page of Project Settings.

#include "Vertexes/M2VariMixerVertex.h"

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
	bSoloActive = MixerChannels.ContainsByPredicate([](const FAssignableAudioOutput& Channel) { return Channel.AssignedPin->bSolo; });

	for (int i = 0; i < MixerChannels.Num(); i++)
	{
		auto& Channel = MixerChannels[i];
		//Channel.MuteState = ECheckBoxState::Unchecked;
		//Channel.SoloState = ECheckBoxState::Unchecked;
		if (Channel.AssignedPin->bMute && !Channel.AssignedPin->bSolo)
		{
			//Channel.MuteState = ECheckBoxState::Checked;
			UpdateGainParam_Internal(i, 0.0f);
			continue;
		}

		if (bSoloActive && !Channel.AssignedPin->bSolo)
		{
			//Channel.MuteState = ECheckBoxState::Checked;
			UpdateGainParam_Internal(i, 0.0f);
			continue;
		}

		UpdateGainParam_Internal(i, Channel.AssignedPin->GainValue);
	}
}

inline void UM2VariMixerVertex::UpdateGainParam_Internal(int ChannelIndex, float newGain)
{
	FName FloatName;
	EMetaSoundBuilderResult BuildResult;

	auto NewFloatLiteral = BuilderSubsystem->CreateFloatMetaSoundLiteral(newGain, FloatName);
	BuilderContext->SetNodeInputDefault(MixerChannels[ChannelIndex].GainParameterInputHandle, NewFloatLiteral, BuildResult);
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
		auto AudioTrackPin = Cast<UM2AudioTrackPin>(OutputM2SoundPins[M2Sound::Pins::AutoDiscovery::AudioTrack]);
		AudioTrackPin->AudioStreamL = CreateOutputPin<UM2MetasoundLiteralPin>(OutPins[0]);
		AudioTrackPin->AudioStreamR = CreateOutputPin<UM2MetasoundLiteralPin>(OutPins[1]);
	}

	//PopulatePinsFromMetasoundData(InPins, OutPins);

	UM2SoundGraphStatics::PopulateAssignableOutputsArray(MixerChannels, BuilderContext->FindNodeInputs(NewMixerNode, BuildResult));

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

	UpdateMuteAndSoloStates();
}

inline FLinearColor UM2VariMixerVertex::GetChannelColor(uint8 ChannelIndex)
{
	if (!MixerChannels.IsValidIndex(ChannelIndex))
	{
		return FLinearColor::Gray;
	}
	if (MixerChannels[ChannelIndex].AssignedPin->LinkedPin)
	{
		return MixerChannels[ChannelIndex].AssignedPin->LinkedPin->ParentVertex->GetVertexColor();
	}

	return FLinearColor::Gray;
}

inline FAssignableAudioOutput UM2VariMixerVertex::CreateChannel()
{
	FAssignableAudioOutput NewChannel;
	//MixerChannels.Add(NewChannel);
	NumConnectedChannels++;
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
}
