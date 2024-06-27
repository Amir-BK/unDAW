// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseUnpatchActor.h"


// Sets default values
ABaseUnpatchActor::ABaseUnpatchActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseUnpatchActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseUnpatchActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

