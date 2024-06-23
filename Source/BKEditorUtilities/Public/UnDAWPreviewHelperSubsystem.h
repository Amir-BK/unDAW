// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "M2SoundGraphData.h"
#include "BK_MusicSceneManagerInterface.h"


#include "UnDAWPreviewHelperSubsystem.generated.h"


struct FSoftClassPreviewHolder
{
	UDAWSequencerData* ActiveSession;

};

/**
 * This is an editor only subsystem that helps with previewing DAW Sequencer data in the editor by generating audio components and tracking the active preview performer.
 */
UCLASS()
class BK_EDITORUTILITIES_API UUnDAWPreviewHelperSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
	
public:

	FSoftClassPreviewHolder ActivePreviewPerformer;


	UFUNCTION()
	void CreateAndPrimePreviewBuilderForDawSequence(UDAWSequencerData* InSessionToPreview);	

	
private:
	bool hasAlreadyPrimed = false;

};
