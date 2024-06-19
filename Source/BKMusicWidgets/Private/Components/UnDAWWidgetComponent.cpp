// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/UnDAWWidgetComponent.h"

UUnDAWWidgetComponent::UUnDAWWidgetComponent()
{
	bDrawAtDesiredSize = true;
}

void UUnDAWWidgetComponent::InitWidget()
{
	UWidgetComponent::InitWidget();
	DAWWidgetInstance = Cast<UUnDAWWidgetBase>(GetUserWidgetObject());
	if (DAWWidgetInstance)
	{
		UE_LOG(LogTemp, Log, TEXT("we have a DAW widget instance"))
			DAWWidgetInstance->SceneManager = SceneManager;
	}
}


void UUnDAWWidgetComponent::BeginPlay()
{
	if (DAWWidgetClass)
	{
		SetWidgetClass(DAWWidgetClass);
	}
	Super::BeginPlay();

}

#if WITH_EDITOR
void UUnDAWWidgetComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	SetWidgetClass(DAWWidgetClass);
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
