// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Framework/Commands/Commands.h"
#include "Framework/Commands/UICommandInfo.h"



/**
 * 
 */

class BK_EDITORUTILITIES_API FDAWEditorToolbarCommands : public TCommands<FDAWEditorToolbarCommands>
{

	public:
		FDAWEditorToolbarCommands() : TCommands<FDAWEditorToolbarCommands>("unDAW Toolbar", INVTEXT("unDAW Toobar"), NAME_None, TEXT("EditorStyle")) {};

		TSharedPtr<FUICommandInfo> TransportPlay;

		virtual void RegisterCommands() override;
	
};
