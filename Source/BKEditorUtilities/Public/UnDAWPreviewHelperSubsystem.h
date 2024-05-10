// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "SequencerData.h"
#include "UnDAWPreviewHelperSubsystem.generated.h"


struct FSoftClassPreviewHolder
{
	UDAWSequencerData* ActiveSession;
	UDAWSequencerPerformer* PreviewPerformer;
};

/**
 * 
 */
UCLASS()
class BK_EDITORUTILITIES_API UUnDAWPreviewHelperSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
	
public:

	FSoftClassPreviewHolder ActivePreviewPerformer;

	TMap<FSoftObjectPath, UDAWSequencerPerformer*> PreviewBuilders;

	UFUNCTION()
	void CreateAndPrimePreviewBuilderForDawSequence(UDAWSequencerData* InSessionToPreview);
	
private:
	bool hasAlreadyPrimed = false;

};
