// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Selection.h"
#include "BKListnerComponentConfigWidget.generated.h"

/**
 * 
 */
UCLASS()
class BK_EDITORUTILITIES_API UBKListnerComponentConfigWidget : public UUserWidget
{
	GENERATED_BODY()


	virtual void OnObjectSlected(UObject* SelectedObject);

	virtual void OnSelectNone();

	FString SelectedObjectName = "";


protected:

	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	//virtual void NativeConstruct() override; 

	//~ End UWidget Interface
	
};
