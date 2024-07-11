// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MetasoundSource.h"
#include "UObject/Object.h"
#include "UnDAWSFZPerformer.generated.h"

class USFZRegion;
class UFKSFZAsset;

USTRUCT(BlueprintType)
struct FSFZGroupPerformanceData
{
	GENERATED_BODY()

	UPROPERTY()
	int seq_position = 1;

	UPROPERTY()
	int seq_length = 1;

	UPROPERTY()
	bool note_off_mask = false;

	// the name given to this performance group
	UPROPERTY()
	int group_label = -1;

	//an array of all audio componnents currently managed by this group
	UPROPERTY()
	TArray<UAudioComponent*> ManagedAudioComponents;

	// the regions and the audio components are ultimately different entities
	UPROPERTY()
	TArray<const USFZRegion*> ManagedSFZRegions;
};

/**
 * @brief this object is considered the 'manager' class for a single SFZ instrument, it manages region logics for this instrument and generates new audio components using 'performNote' and 'performNoteOff' method, future versions will also manage passing CC parameters to all existing components.
 */

UCLASS(BlueprintType)
class BKMUSICCORE_API UFK_SFZ_Performer : public UObject
{
public:
	explicit UFK_SFZ_Performer()
		: PedalStateAttribute(false)
	{
	}

private:
	GENERATED_BODY()

	friend class UFKSFZAsset;

	// The SFZ sample bank that was to spawn this performer
	UPROPERTY()
	UFKSFZAsset* ParentSFZBank;

	UPROPERTY()
	UMetaSoundSource* PerformanceMetasound;

	UPROPERTY()
	TArray<UAudioComponent*> ManagedComponents;

	TAttribute<bool> PedalStateAttribute;

	void SetPedalState(const bool& NewState);

	bool GetCurrentPedalState() const;

	/**
	 * This is the data map storing initial mapping from notes to groups of regions
	 */
	UPROPERTY()
	TMap<int, FSFZGroupPerformanceData> GroupsDataMap;

	/**
	*  @brief Call this to generate an Audio Component preconfigured with the appropriate sample data for the desired note/velocity como. TODO: will also trigger note-off logics to mapped audio components conditionally

	* @param Caller the in-world actor to use for spawning the audio component, can be used for attachment and spatialization
	* @param Note the pitch of the note to perform, in standard midi scaling 0-127
	* @param Velocity the velocity of the note, 0-127
	* @param duration duration in seconds, if duration is 0 or lower the note will wait for note off trigger
	* @param AudioComponent result audio component.
	 * @return whether audio component was created succesffuly and the note is considered 'performed' for the purpose of the next note to be performed.
	   **/

public:
	UFUNCTION(BlueprintCallable, meta = (DefaultToself = "caller"), Category = "BK Music")
	bool PerformNote(AActor* Caller, int Note, int Velocity, int duration, UAudioComponent*& AudioComponent);
};
