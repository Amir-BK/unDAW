// Fill out your copyright notice in the Description page of Project Settings.


#include "SequenceAssetEditor/DAWEditorCommands.h"

#define LOCTEXT_NAMESPACE "unDAWEditorCommands"

inline void FDAWEditorToolbarCommands::RegisterCommands()
{
	UI_COMMAND(TransportPlay, "Play", "Play", EUserInterfaceActionType::Button, FInputChord(EKeys::SpaceBar));
	UI_COMMAND(TransportStop, "Stop", "Stop", EUserInterfaceActionType::Button, FInputChord(EKeys::SpaceBar, EModifierKey::Control));

	UI_COMMAND(ToggleNotePaintingMode, "Toggle Note Painting Mode", "Toggle Note Painting Mode", EUserInterfaceActionType::ToggleButton, FInputChord(EKeys::B));

};

#undef LOCTEXT_NAMESPACE


