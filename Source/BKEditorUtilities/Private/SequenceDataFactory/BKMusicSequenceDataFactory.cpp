// Fill out your copyright notice in the Description page of Project Settings.


#include "SequenceDataFactory/BKMusicSequenceDataFactory.h"

UObject* UBKMusicSequenceDataFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	return NewObject<UDAWSequencerData>(InParent, InClass, InName, Flags);
}

bool UBKMusicSequenceDataFactory::ShouldShowInNewMenu() const
{
	return true;
}

UBKMusicSequenceDataFactory::UBKMusicSequenceDataFactory()
{
	SupportedClass = UDAWSequencerData::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}
