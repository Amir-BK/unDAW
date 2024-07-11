// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Framework/Commands/Commands.h"
#include "Framework/Commands/UICommandInfo.h"

/**
 *
 */

class BK_EDITORUTILITIES_API FDAWEditorToolbarCommands final : public TCommands<FDAWEditorToolbarCommands>
{
public:
	FDAWEditorToolbarCommands() : TCommands<FDAWEditorToolbarCommands>("unDAW Toolbar", INVTEXT("unDAW Toobar"), NAME_None, TEXT("EditorStyle")) {};

	TSharedPtr<FUICommandInfo> TransportPlay;
	TSharedPtr<FUICommandInfo> TransportStop;

	TSharedPtr<FUICommandInfo> ToggleNotePaintingMode;
	TSharedPtr<FUICommandInfo> TogglePianoTabView;

	virtual void RegisterCommands() override;
};

class BK_EDITORUTILITIES_API FM2SoundNodeCommands final : public TCommands<FM2SoundNodeCommands>
{
public:
	FM2SoundNodeCommands() : TCommands<FM2SoundNodeCommands>("M2SoundNode", INVTEXT("M2SoundNode"), NAME_None, TEXT("EditorStyle")) {};

	TSharedPtr<FUICommandInfo> SetPinAsColorSource;

	virtual void RegisterCommands() override;
};
