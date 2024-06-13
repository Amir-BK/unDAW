// Fill out your copyright notice in the Description page of Project Settings.


#include "SFZ/UnDAWSFZAsset.h"

#include "SFZ/UnDAWSFZPerformer.h"

UFKSFZAsset::UFKSFZAsset()
{

}

UFK_SFZ_Performer* UFKSFZAsset::CreateSFZPerformerFromSampleBank(UFKSFZAsset* inAsset, UMetaSoundSource* metasoundToUse)
{
	auto toReturn = NewObject<UFK_SFZ_Performer>();
	toReturn->ParentSFZBank = inAsset;
	toReturn->PerformanceMetasound = metasoundToUse;
	
	return toReturn;
}

TArray<UFKSFZAsset*> UFKSFZAsset::GetAllSFZAssets()
{
	auto toReturn = TArray<UFKSFZAsset*>();
	
	GetObjectsOfClass<UFKSFZAsset>(toReturn);
	
	return toReturn;
}

TArray<UFusionPatch*> UFKSFZAsset::GetAllFusionPatchAssets()
{
	auto toReturn = TArray<UFusionPatch*>();

	GetObjectsOfClass<UFusionPatch>(toReturn);

	return toReturn;

}

USoundWave* UFKSFZAsset::GetSoundWaveForNote(const int Note, int seqPosition,
											 bool& bFoundSample, int& PitchDistance)
{
	if (!notesToGroupsMap.Contains(Note))
	{
		bFoundSample = false;
		PitchDistance = 0;
		return nullptr;
	}

	const auto Group = *notesToGroupsMap.Find(Note);
	const USFZRegion*  FoundRegion = Group->GetRegionForVelocity(Note, E_SFZ_TRIGGERTYPE::Attack, seqPosition);
	
	if (FoundRegion == nullptr) 
	{
		bFoundSample = false;
		PitchDistance = 0;

		return nullptr;
	}
		PitchDistance = FoundRegion->centerNoteValue - Note;
		bFoundSample = true;
		return FoundRegion->WavAsset;
}

USFZGroupedRegions* UFKSFZAsset::GetRegionGroupForNote(bool& bSuccess, int const Note)
{
	if (!notesToGroupsMap.Contains(Note))
	{
		bSuccess = false;
		return nullptr;
	}

	//*notesToGroupsMap.Find(Note);
	bSuccess = true;
	
	return *notesToGroupsMap.Find(Note);
}

void UFKSFZAsset::MapNotesToRanges()
{
	for (USFZRegion* region : Regions)
	{
		region->InitializeParams();
		for (int i = region->loNote; i <= region->hiNote; i++)
		{
			
			USFZGroupedRegions* groupRef = nullptr;
			//notesToRegionsMap.Add(TTuple<int, USFZRegion*>(i, region));
			if (notesToGroupsMap.Contains(i))
			{
				groupRef = *notesToGroupsMap.Find(i);

			}
			else {
				groupRef = NewObject<USFZGroupedRegions>(this);
				notesToGroupsMap.Add(TTuple<int, USFZGroupedRegions*>(i, groupRef));
			}

			groupRef->RegionsInGroup.Add(region);
		}
	}
}


TSharedPtr<Audio::IProxyData> UFKSFZAsset::CreateProxyData(const Audio::FProxyDataInitParams& InitParams)
{
	if (!DataPtr) DataPtr = MakeShared<TMap<int, USFZGroupedRegions*>>();
	*DataPtr = notesToGroupsMap;
	return MakeShared<F_FK_SFZ_Asset_Proxy>(this);
}

//this is the critical function where the performance data is populated in the runtime struct
void UFK_Region_Runtime_Performance_Data::InitWithRegionObjectAndPitch(const float& inVelocity, const int& Note, USFZRegion* RegionData)
{
	
	RegionDataPtr = RegionData;

	Region_Performance_Parameters = FSFZRegionPerformanceParameters();

	
	Region_Performance_Parameters.Tune = RegionData->GetPitchDistanceFromNote(Note) + (0.01*RegionData->tune);
	Region_Performance_Parameters.Offset = RegionData->Offset_Time;
	Region_Performance_Parameters.LoopMode = RegionData->loopMode;
	Region_Performance_Parameters.Loop_Start = RegionData->Loop_Start_Time;
	Region_Performance_Parameters.Loop_Duration = RegionData->Loop_Duration_Time;
	Region_Performance_Parameters.bOneShot = RegionData->loopMode == One_Shot;
	Region_Performance_Parameters.Note = Note;
	Region_Performance_Parameters.Velocity = inVelocity;

	if(RegionData->SFZNormalizedTimedParamsArray.Contains("Delay")) Region_Performance_Parameters.Delay = RegionData->SFZNormalizedTimedParamsArray["Delay"];
	if(RegionData->SFZNormalizedTimedParamsArray.Contains("delay_random"))
	{
		const float Rand = RegionData->SFZNormalizedTimedParamsArray["delay_random"];
		Region_Performance_Parameters.Delay += FMath::RandRange(-Rand/2, Rand/2);
	}

	if(RegionData->SFZFloatParamsArray.Contains("pitch_random"))
	{
		// pitch is in cents...             
		const float RandPitch = 0.01 * RegionData->SFZFloatParamsArray["pitch_random"];
		Region_Performance_Parameters.Tune += FMath::RandRange(-RandPitch/2, RandPitch/2);
	}

	sample = RegionData->WavAsset;

	
	
	
}

TSharedPtr<Audio::IProxyData> UFK_Region_Runtime_Performance_Data::CreateProxyData(const Audio::FProxyDataInitParams& InitParams)
{
	if (!PerfStructDataPtr) PerfStructDataPtr = MakeShared<FSFZRegionPerformanceParameters>();
	*PerfStructDataPtr = Region_Performance_Parameters;
	
	//if (!SampleDataPtr) 

	//if (!SampleDataPtr) SampleDataPtr = MakeShareable(sample);
	//SampleDataPtr = sample;
	
	
	return MakeShared<FFK_SFZ_Region_Performance_Proxy>(this);
}

int USFZRegion::GetPitchDistanceFromNote(int const Note) const
{
	return Note - centerNoteValue;
}

//this is a critical function that happens when importing the file
void USFZRegion::InitializeParams()
{
	// call this only after wav file has done importing. TODO: or not
	if(SFZIntParamsArray.Contains("seq_position")) seqPosition = SFZIntParamsArray["seq_position"];
	if(SFZIntParamsArray.Contains("seq_length")) seqLength = SFZIntParamsArray["seq_length"];

	for (auto&[key, value] : SFZTimedParamsArray)
	{
		//WavAsset->

		
		if(IsValid(WavAsset))
		{
			const auto& SampleRate = WavAsset->GetSampleRateForCurrentPlatform(); 
			UE_LOG(FK_SFZ_Logs, Verbose, TEXT("Platform SR: %f "), SampleRate)
			SFZNormalizedTimedParamsArray.Add(key, static_cast<float>(value)/ SampleRate);

		}else
		{
			//assume 48k
			SFZNormalizedTimedParamsArray.Add(key, static_cast<float>(value/ 48000));
		}
	
	}

	if(IsValid(WavAsset) && loopMode == NotSet)
	{
		if(WavAsset->bLooping)		loopMode = Loop_Continuous;
		if(WavAsset->GetLoopRegions().Num() > 0)
		{
			loopMode = Loop_Continuous;
			const auto& SampleRate = WavAsset->GetSampleRateForCurrentPlatform(); 
			for(const auto& LoopRegion : WavAsset->GetLoopRegions())
			{
				Loop_Start_Time = static_cast<float>(LoopRegion.FramePosition) / SampleRate;
				UE_LOG(FK_SFZ_Logs, Verbose, TEXT("Detected Loop Cue Point In wave file with no loop data set start: %d, %f "), LoopRegion.FramePosition, Loop_Start_Time)
				Loop_Duration_Time = static_cast<float>(LoopRegion.FrameLength) / SampleRate;
			
				UE_LOG(FK_SFZ_Logs, Verbose, TEXT("Detected Loop Cue Point In ave File with no loop data set duration: %d, %f "), LoopRegion.FrameLength, Loop_Duration_Time)
			}
		}
	}


	//so maybe we get to do some stuff here as well, init the proxy struct, it's a bit of double work which we'll sort in the future,

}

USFZRegion* USFZGroupedRegions::GetRegionForVelocity(int velocityIn, E_SFZ_TRIGGERTYPE trigtype, int rrSeqPosition)
{
	for (auto region : RegionsInGroup)
	{
		auto velRange = TRange<int>(region->loVel, region->hiVel);
		if (velRange.Contains(velocityIn) && region->triggerType == trigtype && region->seqPosition == rrSeqPosition) return region;

	}

	return nullptr;
}

// mmm -1 == no group so maybe -2 all groups?
// group ID -1 sent in this function returns all groups in note/velocity combo regardless of polyphonic group settings, it's for testing
void USFZGroupedRegions::GetRegionsForCondition(TArray<USFZRegion*>& InArray, bool& foundRegions, int velocityIn,
												int rrSeqPosition, int GroupID)
{
	foundRegions = false;
	if (!RegionsInGroup.IsEmpty()) {
		for (auto region : RegionsInGroup)
		{
			auto velRange = TRange<int>().Inclusive(region->loVel, region->hiVel);
			//we will try to accomodate for disrepencies in velocity mappings, i.e cases where one velocity layer lacks a round robin position, a bit hacky
			if (velRange.Contains(velocityIn) && region->seqPosition == FMath::Clamp(rrSeqPosition, 1, region->seqLength))
			{
				if(GroupID != -2)
				{
					if(region->group == GroupID)
					{
						InArray.AddUnique(region);
						foundRegions = true;
					}
				}else
				{
					InArray.AddUnique(region);
					foundRegions = true;
				}

			}
		}
	}

}



USFZGroupedRegions* F_FK_SFZ_Asset_Proxy::GetRegionGroupForNote(bool& success, int note) const
{
	if (!DataPtr.Get()->Contains(note))
	{
		success = false;

		return nullptr;
	}
	const auto Group = DataPtr.Get()->Find(note);
	success = true;

	return *Group;
}


