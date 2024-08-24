// Fill out your copyright notice in the Description page of Project Settings.


#include "MidiDrivensSequenceEditor/MidiDrivenSequenceFactory.h"
#include "MidiDrivenSequence/MidiDrivenLevelSequence.h"
#include "MovieScene.h"
#include "MovieSceneToolsProjectSettings.h"

UMidiDrivenSequenceFactory::UMidiDrivenSequenceFactory()
{
	SupportedClass = UMidiDrivenLevelSequence::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UMidiDrivenSequenceFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	auto NewLevelSequence = NewObject<UMidiDrivenLevelSequence>(InParent, Class, Name, Flags);
	NewLevelSequence->Initialize();

	// Set up some sensible defaults
	const UMovieSceneToolsProjectSettings* ProjectSettings = GetDefault<UMovieSceneToolsProjectSettings>();

	FFrameRate TickResolution = NewLevelSequence->GetMovieScene()->GetTickResolution();
	NewLevelSequence->GetMovieScene()->SetPlaybackRange((ProjectSettings->DefaultStartTime * TickResolution).FloorToFrame(), (ProjectSettings->DefaultDuration * TickResolution).FloorToFrame().Value);

	return NewLevelSequence;

}

bool UMidiDrivenSequenceFactory::ShouldShowInNewMenu() const
{
	return true;
}


