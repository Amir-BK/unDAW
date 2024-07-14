// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Vertexes/M2SoundVertex.h"
#include "BaseUnpatchActor.generated.h"

UCLASS(Abstract)
class UNPATCHWORK_API ABaseUnpatchActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseUnpatchActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unpatch")
	FString VertexName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unpatch")
	FLinearColor VertexColor;

	void SetVertex(UM2SoundVertex* InVertex);

protected:

	friend class AAutoPatchActorMapper;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unpatch")
	UM2SoundVertex* M2Vertex;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
