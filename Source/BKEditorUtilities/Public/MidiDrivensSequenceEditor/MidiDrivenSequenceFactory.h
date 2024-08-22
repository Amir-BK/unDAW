// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "MidiDrivenSequenceFactory.generated.h"

/**
 * 
 */
UCLASS()
class BK_EDITORUTILITIES_API UMidiDrivenSequenceFactory : public UFactory
{
	GENERATED_BODY()
	
	public:
		UMidiDrivenSequenceFactory();
		virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
		virtual bool ShouldShowInNewMenu() const override;
	
	
};
