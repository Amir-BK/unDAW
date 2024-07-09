// Fill out your copyright notice in the Description page of Project Settings.


#include "SequenceAssetEditor/DAWEditorCommands.h"

#define LOCTEXT_NAMESPACE "BKEditorUtilitiesModule"

void FDAWEditorToolbarCommands::RegisterCommands()
{
	UI_COMMAND(TransportPlay, "Play", "Play", EUserInterfaceActionType::Button, FInputChord(EKeys::SpaceBar));
	UI_COMMAND(TransportStop, "Stop", "Stop", EUserInterfaceActionType::Button, FInputChord(EKeys::SpaceBar, EModifierKey::Control));

	UI_COMMAND(ToggleNotePaintingMode, "Toggle Note Painting Mode", "Toggle Note Painting Mode", EUserInterfaceActionType::ToggleButton, FInputChord(EKeys::B));
	UI_COMMAND(TogglePianoTabView, "Toggle Piano Tab View", "Toggle Piano Tab View", EUserInterfaceActionType::ToggleButton, FInputChord(EKeys::P));
};



void FM2SoundNodeCommands::RegisterCommands()
{
	UI_COMMAND(SetPinAsColorSource, "Color Source", "Color Source", EUserInterfaceActionType::ToggleButton, FInputChord());
};

#undef LOCTEXT_NAMESPACE
