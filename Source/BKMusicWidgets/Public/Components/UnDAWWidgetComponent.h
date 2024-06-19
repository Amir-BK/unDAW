// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "UnDAWWidgetBase.h"
#include <MusicScenePlayerActor.h>
#include "UnDAWWidgetComponent.generated.h"


/**
 * 
 */
UCLASS(ClassGroup = (unDAW), meta = (BlueprintSpawnableComponent), BlueprintType, Blueprintable, HideCategories = "User Interface")
class BKMUSICWIDGETS_API UUnDAWWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
	public:
	

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "unDAW|Widget")
	TSubclassOf<UUnDAWWidgetBase> DAWWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "unDAW|Widget")
	TObjectPtr<UUnDAWWidgetBase> DAWWidgetInstance;



	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "unDAW|Widget")
	AMusicScenePlayerActor* SceneManager;



	UUnDAWWidgetComponent();
	virtual void InitWidget() override; //to cache a reference to the widget instance as an undaw widget
	void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
};
