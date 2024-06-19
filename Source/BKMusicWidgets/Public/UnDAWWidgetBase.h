// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "MusicScenePlayerActor.h"
#include "UnDAWWidgetBase.generated.h"

/**
 * 
 */
UCLASS()
class BKMUSICWIDGETS_API UUnDAWWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()
	
	public:
	//Scene manager reference
		UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "unDAW|Widget")
		AMusicScenePlayerActor* SceneManager;
	
	
};
