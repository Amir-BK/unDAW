// Fill out your copyright notice in the Description page of Project Settings.


#include "FK_SFZ_Performer.h"

#include "FKSFZAsset.h"
#include "MetasoundSource.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"


BKMUSICCORE_API DEFINE_LOG_CATEGORY(FK_SFZ_Logs);


#define PARAM_NAME(name) FName(*FString::Printf(TEXT("FlyKick SFZ.%s_%d_%s"), *RegionName, RegionID, TEXT(name)))
#define PARAM_NAME_STRING(name) FName(*FString::Printf(TEXT("%s_%d_%s"), *RegionName, RegionID, name))
#define FK_SET_AUDIO_PARAM_WITH_LITERAL(ParamName, ParamValue) AudioComponent->SetParameter(\
FAudioParameter(PARAM_NAME(ParamName), ParamValue))
#define FK_SET_AUDIO_PARAM(ParamName, ParamValue) AudioComponent->SetParameter(\
FAudioParameter(PARAM_NAME_STRING(ParamName), ParamValue))


void UFK_SFZ_Performer::SetPedalState(const bool& NewState)
{
	PedalStateAttribute.Set(NewState);
}

bool UFK_SFZ_Performer::GetCurrentPedalState() const
{
	return PedalStateAttribute.Get();
}

bool UFK_SFZ_Performer::PerformNote(AActor* Caller, int Note, int Velocity, int duration, UAudioComponent*& AudioComponent)
{
	
    //we should actually check if we have valid audio to return before spawning the component and adding it to the map
    
    // wrap component in FSFZGroupPerformanceData first and add it to the map as such, etc.
	
    AudioComponent =  UGameplayStatics::CreateSound2D(Caller, PerformanceMetasound);
	
	if (PerformanceMetasound != nullptr)
		{
		bool foundParam = PerformanceMetasound->IsParameterValid(FAudioParameter(FName(TEXT("Layer_0_Region"))));
            AudioComponent->Sound = PerformanceMetasound;
		AudioComponent->SetParameter(FAudioParameter(FName("FlyKick SFZ.Internals.PedalState"), PedalStateAttribute.Get()));
		}

	if (!IsValid(AudioComponent)) return false;

	FSFZGroupPerformanceData PerformanceGroupData;

	//we really should flip the order here, don't spawn an audio component and group unless we have a note to actually play, etc. for now this will work
	if (GroupsDataMap.Contains(Note))
	{
		// TODO [$65cfdef41013620009101dd8]: Implement 'if region is already playing logics'
        // perform logic if group already exists,
		// retrieve current round robin sequence position, issue 'stop' (if we didn't receive note off), yada yada
		PerformanceGroupData = GroupsDataMap[Note];
		for (auto component : PerformanceGroupData.ManagedAudioComponents)
		{
			//Sending the note off trigger to the previous component is a bit superfluous as Aubrey is already managing that,
			//as we decouple from MidiEngine this will become more relevant
			//test implementation of hard note off for polyphony management. 
			if(IsValid(component))		component->SetTriggerParameter(FName("FlyKick SFZ.Internals.HardNoteOff"));
		}

		PerformanceGroupData.ManagedAudioComponents.Empty();
		PerformanceGroupData.ManagedAudioComponents.Add(AudioComponent);

	}	else
	{
		PerformanceGroupData = FSFZGroupPerformanceData();
		//add audio component to managed components array
		PerformanceGroupData.ManagedAudioComponents.Add(AudioComponent);
	}
	bool bFoundGroup;
	USFZGroupedRegions* ReturnedGroups = ParentSFZBank->GetRegionGroupForNote(bFoundGroup, Note);

	if (bFoundGroup)
	{
		bool foundAnyRegion = false;
		//this functionality is buggy with the way SFZ works, we're going to need to do groupings for kalimba to work...
		TArray<USFZRegion*> ReturnedRegions;

		//this is the heart of roundrobin and polyphony handling, needs some more meat. 
		if(PerformanceGroupData.ManagedSFZRegions.IsEmpty())
		{
			bool foundRegion;
			//a bit of magic... groupID - 2 returns all groups, group ID -1 is just the default group. we reserve 0 and above for groups from the file
			ReturnedGroups->GetRegionsForCondition(ReturnedRegions, foundRegion, Velocity, 1, -2);
			UE_LOG(FK_SFZ_Logs, VeryVerbose, TEXT("Sample: %s, SeqPosition: %d, GroupID: %d"),
					TEXT("Starting from scratch"), 1, -2);
			foundAnyRegion = foundAnyRegion || foundRegion;
		}else
		{
			for(const auto ManagedRegion : PerformanceGroupData.ManagedSFZRegions)
			{
				
				bool foundRegion;
				const int NextSeqPosition = (ManagedRegion->seqPosition + 1) % ManagedRegion->seqLength;
				UE_LOG(FK_SFZ_Logs, VeryVerbose, TEXT("Sample: %s, SeqPosition: %d, GroupID: %d"),
					IsValid(ManagedRegion->WavAsset) ? *ManagedRegion->WavAsset->GetName() : TEXT("silence or something"), NextSeqPosition, ManagedRegion->group);
				ReturnedGroups->GetRegionsForCondition(ReturnedRegions, foundRegion, Velocity, NextSeqPosition, ManagedRegion->group);
				foundAnyRegion = foundAnyRegion || foundRegion;
			}
		}
		//for now we clear the managed regions, but more complex group logic will require better clearing, can we clear while iterating?
		PerformanceGroupData.ManagedSFZRegions.Empty();
		
		if (foundAnyRegion)
		{
			//PerformanceGroupData.seq_length = ReturnedRegions[0]->seqLength;

			int NumAttackLayers = 0;
			int NumReleaseLayers = 0;

			AudioComponent->SetParameter(FAudioParameter(FName(TEXT("NumLayers")), ReturnedRegions.Num()));
            AudioComponent->SetParameter(FAudioParameter(FName(TEXT("Vel")), Velocity));
			
			for (auto region : ReturnedRegions)
			{
				PerformanceGroupData.ManagedSFZRegions.Add(region);
				FString RegionName = TEXT("Layer");
				int RegionID = 0;
				// assign region to correct group
				switch (region->triggerType)
				{
					//hacking First and Attack together here, do note lack of 'break' statement, this is probably fine 
					case E_SFZ_TRIGGERTYPE::First:
					case E_SFZ_TRIGGERTYPE::Legato:
					case E_SFZ_TRIGGERTYPE::Attack:
					default:
					RegionID = NumAttackLayers++;
					break;

				//also mangling release and release key together, this might cause issues as Release_key ignores releasing the pedal, though this can be handled by group via not sending the trigger
				case E_SFZ_TRIGGERTYPE::Release:
				case E_SFZ_TRIGGERTYPE::Release_Key:
					RegionID = NumReleaseLayers++;
					RegionName = TEXT("Release");
					break;
				};
				const bool bLoopMode = region->loopMode == E_SFZ_LOOP_MODE::Loop_Continuous || region->loopMode == E_SFZ_LOOP_MODE::Loop_Sustain;

				UFK_Region_Runtime_Performance_Data* PerfRegion;

				//some ugly copy paste ahead

				PerfRegion = NewObject<UFK_Region_Runtime_Performance_Data>();
				PerfRegion->InitWithRegionObjectAndPitch(Velocity, Note, region);
				//AudioComponent->SetParameter(FAudioParameter(FName("FlyKick SFZ.Layer_0_Region"),PerfRegion));


				FK_SET_AUDIO_PARAM_WITH_LITERAL("Region", PerfRegion);
				FK_SET_AUDIO_PARAM_WITH_LITERAL("Sample", region->WavAsset);
				FK_SET_AUDIO_PARAM_WITH_LITERAL("Loop",bLoopMode);
				FK_SET_AUDIO_PARAM_WITH_LITERAL("Pitchshift",region->GetPitchDistanceFromNote(Note));

				for(const auto[key, value] : region->SFZNormalizedTimedParamsArray)
				{
					//UE_LOG(FK_SFZ_Logs, Log, TEXT("The Param Name Is %s"), *key.ToString())
					FK_SET_AUDIO_PARAM(*key.ToString(), value);
				}
				for(const auto[key, value] : region->SFZFloatParamsArray)
				{
					FK_SET_AUDIO_PARAM(*key.ToString(), value);
				}
			}
		}

			
			
		GroupsDataMap.Add(Note,PerformanceGroupData);

	}


	return true;
}
