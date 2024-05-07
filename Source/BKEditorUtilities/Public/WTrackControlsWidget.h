// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"

#include "WTrackControlsWidget.generated.h"


/**
 * 
 */
UCLASS(Category = "BK Music|Track Settings")
class BK_EDITORUTILITIES_API UWTrackControlsWidget : public UWidget
{
	GENERATED_BODY()


	

	//void SetMidiEditorParentWidget(UMIDIEditorBase* inEditor);

	UFUNCTION(BlueprintCallable, Category = "BK Music|Track Settings")
	void InitFromData();
	
protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
		//~ End UWidget Interface

	TSharedPtr<SVerticalBox> tracksVerticalBox;

	//TWeakObjectPtr<UMIDIEditorBase> MidiEditorSharedPtr;
	//UMIDIEditorBase* ParentMidiEditor;


	
};
