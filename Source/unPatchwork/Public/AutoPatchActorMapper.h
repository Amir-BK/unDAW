// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseUnpatchActor.h"
#include "M2SoundGraphData.h"
#include "Components/BillboardComponent.h"
#include "MusicScenePlayerActor.h"
#if WITH_EDITOR
#include "Editor/UnrealEd/Public/Kismet2/BlueprintEditorUtils.h"
#include "Editor/UnrealEd/Public/Kismet2/KismetEditorUtilities.h"
#endif

#include "AutoPatchActorMapper.generated.h"

UCLASS()
class UNPATCHWORK_API AAutoPatchActorMapper : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAutoPatchActorMapper();

# if WITH_EDITORONLY_DATA

	/** Billboard used to see the trigger in the editor */
	UPROPERTY(Category = TriggerBase, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBillboardComponent> SpriteComponent;

# endif

	//probably better to force the scene to be synced to an actor rather than to the session
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoPatch")
	TObjectPtr<AMusicScenePlayerActor> MusicScenePlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoPatch")
	TSubclassOf<ABaseUnpatchActor> UnpatchActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoPatch")
	TObjectPtr <UDAWSequencerData> SequncerData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoPatch", meta = (MakeEditWidget))
	FTransform StartTransform;

	UFUNCTION(CallInEditor, Category = "AutoPatch")
	void PlaceVertexes();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
