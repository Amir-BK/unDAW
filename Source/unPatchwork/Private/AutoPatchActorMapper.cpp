// Fill out your copyright notice in the Description page of Project Settings.

#include "AutoPatchActorMapper.h"
#include "Engine/Texture2D.h"

// Sets default values
AAutoPatchActorMapper::AAutoPatchActorMapper()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Attachment Root"));

# if WITH_EDITORONLY_DATA

	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (SpriteComponent)
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> TriggerTextureObject;
			FName ID_Triggers;
			FText NAME_Triggers;
			FConstructorStatics()
				: TriggerTextureObject(TEXT("/Engine/EditorResources/S_Trigger"))
				, ID_Triggers(TEXT("Triggers"))
				, NAME_Triggers(NSLOCTEXT("SpriteCategory", "Triggers", "Triggers"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		SpriteComponent->Sprite = ConstructorStatics.TriggerTextureObject.Get();
		SpriteComponent->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
		SpriteComponent->bHiddenInGame = false;
		SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Triggers;
		SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Triggers;
		SpriteComponent->bIsScreenSizeScaled = true;
	}

# endif
}

void AAutoPatchActorMapper::PlaceVertexes()
{
	if (MusicScenePlayer && UnpatchActorClass && MusicScenePlayer->GetDAWSequencerData())
	{
		auto* SessionData = MusicScenePlayer->GetDAWSequencerData();

		auto Vertexes = SessionData->GetVertexes();

		int PlaceMentX = 500;

		for (auto Vertex : Vertexes)
		{
			auto NewVertexActor = GetWorld()->SpawnActor<ABaseUnpatchActor>(UnpatchActorClass, StartTransform);
			NewVertexActor->SetVertex(Vertex);

			StartTransform.AddToTranslation(FVector(PlaceMentX, 0, 0));
		}
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("No Music Scene Player or Unpatch Actor Class set on Auto Patch Actor Mapper"))
	}
}

// Called when the game starts or when spawned
void AAutoPatchActorMapper::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AAutoPatchActorMapper::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}