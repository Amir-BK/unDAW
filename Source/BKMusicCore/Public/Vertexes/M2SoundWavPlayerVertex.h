// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Vertexes/M2SoundVertex.h"
#include "M2SoundWavPlayerVertex.generated.h"


namespace UnDAW
{
	namespace SystemPatches
	{
		static const FSoftObjectPath TimestampWavPlayerPath = FSoftObjectPath("/unDAW/Patches/System/unDAW_TimeStampedWavPlayer.unDAW_TimeStampedWavPlayer");
	};
}
/**
 * 
 */
UCLASS()
class BKMUSICCORE_API UM2SoundWavPlayerVertex : public UM2SoundPatch
{
	GENERATED_BODY()
public:

	void BuildVertex() override;

	UPROPERTY(EditAnywhere, Category = "Wav Player")
	TObjectPtr<USoundWave> SoundWave = nullptr;

	UPROPERTY(EditAnywhere, Category = "Wav Player")
	FMusicTimestamp StartTimestamp = FMusicTimestamp();

	UPROPERTY()
	FMusicTimestamp EndTimestamp = FMusicTimestamp();


};
