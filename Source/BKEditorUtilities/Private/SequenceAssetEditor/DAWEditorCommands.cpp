// Fill out your copyright notice in the Description page of Project Settings.


#include "SequenceAssetEditor/DAWEditorCommands.h"

#define LOCTEXT_NAMESPACE "unDAWEditorCommands"

inline void FDAWEditorToolbarCommands::RegisterCommands()
{
	UI_COMMAND(TransportPlay, "Play", "Play", EUserInterfaceActionType::Button, FInputChord(EKeys::SpaceBar));
}
