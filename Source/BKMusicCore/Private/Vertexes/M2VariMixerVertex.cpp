// Fill out your copyright notice in the Description page of Project Settings.

#include "Vertexes/M2VariMixerVertex.h"

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
