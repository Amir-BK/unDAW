// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "BK_MusicSceneManagerInterface.h"
#include "SceneManagerTransport.generated.h"

/**
 * 
 */
UCLASS()
class BKMUSICWIDGETS_API USceneManagerTransportWidget : public UWidget
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable, Category = "unDAW|Transport")
	FOnTransportCommand TransportCalled;
	
};
