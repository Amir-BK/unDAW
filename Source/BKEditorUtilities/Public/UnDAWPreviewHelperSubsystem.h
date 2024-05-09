// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "SequencerData.h"
#include "UnDAWPreviewHelperSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class BK_EDITORUTILITIES_API UUnDAWPreviewHelperSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()
	
public:


	UFUNCTION()
	void CreateAndPrimePreviewBuilderForDawSequence(UDAWSequencerData* InSessionToPreview);
	
private:
	bool hasAlreadyPrimed = false;

};
