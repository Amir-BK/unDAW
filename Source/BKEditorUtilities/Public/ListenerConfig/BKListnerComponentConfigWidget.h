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

	void Test() 
	{
		USelection::SelectObjectEvent.AddUObject(this, &UBKListnerComponentConfigWidget::OnObjectSlected);
	}

	virtual void OnObjectSlected(UObject* SelectedObject)
	{
		UE_LOG(LogTemp, Log, TEXT("Test"))

	};

	
};
