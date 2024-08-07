//The SFZ performer is not currently being used, It was actually written before 5.4 release and it was my failed attempt at making a sampler within metasounds
// I do not delete the performer and the asset as SFZ still has richer functionality than Fusion, extending fusion to support SFZ capabilities is still on the table
// and the performer and the asset still implement a lot of SFZ functionality (and these classes are also a decent example of custom UObject Audio Proxies)

// At the moment the default SFZ factory class is configured such as imported SFZ are converted to Fusion patches, this can be changed in the factory.

#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"
#include "MetasoundSource.h"
#include "IAudioProxyInitializer.h"
#include "Misc/Guid.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/Attribute.h"
#include <HarmonixDsp/FusionSampler/FusionPatch.h>
#include "Components/ActorComponent.h"
#include "UnDAWSFZAsset.generated.h"

BKMUSICCORE_API DECLARE_LOG_CATEGORY_EXTERN(FK_SFZ_Logs, Verbose, All);

class UFK_SFZ_Performer;
class UFKSFZAsset;

//sfz triggers enum
UENUM(BlueprintType, Category = "BK Music|SFZ")
enum E_SFZ_TRIGGERTYPE {
	Attack, Release, First, Legato, Release_Key
};

//sfz loop modes enum
UENUM(BlueprintType, Category = "BK Music|SFZ")
enum E_SFZ_LOOP_MODE
{
	No_Loop, One_Shot, Loop_Continuous, Loop_Sustain, NotSet
};

//these are the performance parameters for the regions, presumably to be consumed by an audio parameter interface
USTRUCT(BlueprintType)
struct FSFZRegionPerformanceParameters
{
	GENERATED_BODY()

	UPROPERTY();
	TEnumAsByte<E_SFZ_LOOP_MODE> LoopMode = One_Shot;

	UPROPERTY();
	float Tune = 0.0f;

	float Loop_Start = 0;

	UPROPERTY();
	float Loop_Duration = 0;

	UPROPERTY();
	float Amplitude = 0;

	UPROPERTY();
	float Offset = 0;

	//the delay value, make sure to include randomness into it
	UPROPERTY();
	float Delay = 0;

	UPROPERTY();
	bool bOneShot = false;

	UPROPERTY();
	int Note;

	UPROPERTY();
	float Velocity;
};

// these are the region mapping parameters, used to check conditions and figure out if the region should play
// these should generally consist of parameters that are of no interest to a 'sample player', they should be used to acquire the sample
// so, given that we currently have no intention of using this data for finding a region within the metasound,
// it can be excluded from the struct we create for the proxy. ergo, that information can remain on the object
// and this struct is superfluous
USTRUCT(BlueprintType, Category = "BK Music|SFZ")
struct FSFZRegionMappingParameters
{
	GENERATED_BODY()

	UPROPERTY();
	TEnumAsByte<E_SFZ_TRIGGERTYPE> triggerType;

	UPROPERTY();
	TObjectPtr<USoundWave> WavAsset;
};

class FFK_SFZ_Region_Performance_Proxy;

UCLASS(Meta = (UsesHierarchy = "true", ShowInnerProperties = "true"), Category = "BK Music|SFZ")
class BKMUSICCORE_API USFZRegion : public UObject
{
	GENERATED_BODY()

public:

	USFZRegion() : centerNoteValue(60), tune(0)
	{
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	int group = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	int off_by = -1;

	// the wav file for this region
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	TObjectPtr<USoundWave> WavAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	TObjectPtr<USoundWave> ObjectPtrWavAsset;

	//this is just for testing, we will import the wave file as a uasset with asset editor tools
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	FString wavFilePath;

	UPROPERTY()
	float Loop_Start = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	float Loop_Start_Time = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	float Loop_Duration_Time = 0;

	UPROPERTY()
	float Loop_End = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	float Loop_End_Time = -1;

	UPROPERTY()
	float Offset = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	float Offset_Time = 0;

	UPROPERTY()
	TArray<int> noteRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	int loNote = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	int hiNote = -1;
	//I'm going to need to parse the note names into pitch numbers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	int centerNoteValue;

	//velocity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	int loVel = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	int hiVel = 127;

	//tune
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	int tune;

	//trigger

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	TEnumAsByte<E_SFZ_TRIGGERTYPE> triggerType = E_SFZ_TRIGGERTYPE::Attack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	TEnumAsByte<E_SFZ_LOOP_MODE> loopMode = E_SFZ_LOOP_MODE::NotSet;

	//sequence - roundrobin
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	int seqPosition = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true", EditInLine = "true"), Category = "BK Music|SFZ")
	int seqLength = 1;

	UPROPERTY(VisibleAnywhere, Category = "BK Music|SFZ")
	TMap<FName, FString> RegionOpCodes;

	// these are in samples as specified in the original SFZ
	UPROPERTY(VisibleAnywhere, Category = "BK Music|SFZ")
	TMap<FName, int> SFZTimedParamsArray;

	// these are normalized to whatever SR the platform will be playing back the wav at, which might not be the sample rate of the metasound, it is perhaps better to use the original value and convert when used.
	UPROPERTY(VisibleAnywhere, Category = "BK Music|SFZ")
	TMap<FName, float> SFZNormalizedTimedParamsArray;

	// used to generically pass float parameters to the performer under their original SFZ name
	UPROPERTY(VisibleAnywhere, Category = "BK Music|SFZ")
	TMap<FName, float> SFZFloatParamsArray;

	// access to the SFZ int parameters, some of them are used for mapping as well, we might also store the note here eventually
	UPROPERTY(VisibleAnywhere, Category = "BK Music|SFZ")
	TMap<FName, int> SFZIntParamsArray;

	// these are our undefined opcodes, one we didn't add to any of the other categories and don't deal with in any sepcial way
	UPROPERTY(VisibleAnywhere, Category = "BK Music|SFZ")
	TMap<FName, FString> SFZStringParamsArray;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BK Music|SFZ")
	int GetPitchDistanceFromNote(int const Note) const;

	// Generally this function is meant to transform params to local 'timing' given sample rate, but as this should probably be done when we play back the sample I'm also gonna stick here some init data
	UFUNCTION()
	void InitializeParams();

	//
	//TSharedPtr<USoundWave> GetSampleSharedPtr()
	//{
	//	if (!SampleSharedPtr.IsValid()) SampleSharedPtr = MakeShareable(*WavAsset);
	//	return SampleSharedPtr;
	//}

private:
	TSharedPtr<USoundWave> SampleSharedPtr;
};

UCLASS()
class BKMUSICCORE_API UFK_Region_Runtime_Performance_Data : public UObject, public IAudioProxyDataFactory
{
	GENERATED_BODY()
public:

	UPROPERTY();
	FSFZRegionPerformanceParameters Region_Performance_Parameters;

	UPROPERTY()
	TObjectPtr<USFZRegion> RegionDataPtr;
private:

	friend class UFK_SFZ_Performer;
	friend class FFK_SFZ_Region_Performance_Proxy;

	void InitWithRegionObjectAndPitch(const float& inVelocity, const int& Note, USFZRegion* RegionData);

	UPROPERTY()
	TObjectPtr<USoundWave> sample;

	TSharedPtr<FSFZRegionPerformanceParameters> PerfStructDataPtr;
	TSharedPtr<USoundWave> SampleDataPtr;
public:
	virtual TSharedPtr<Audio::IProxyData> CreateProxyData(const Audio::FProxyDataInitParams& InitParams) override;
};

class BKMUSICCORE_API FFK_SFZ_Region_Performance_Proxy : public Audio::TProxyData<FFK_SFZ_Region_Performance_Proxy>
{
public:
	IMPL_AUDIOPROXY_CLASS(FFK_SFZ_Region_Performance_Proxy);

	explicit FFK_SFZ_Region_Performance_Proxy(UFK_Region_Runtime_Performance_Data* InRegion)
		: Guid(FGuid::NewGuid()), RawPointerToRegion(InRegion->RegionDataPtr), PerfStructDataPtr(InRegion->PerfStructDataPtr), SampleSimplePtr(InRegion->sample) {}
	FFK_SFZ_Region_Performance_Proxy(FFK_SFZ_Region_Performance_Proxy& Other) = default;

	FGuid Guid;

	FSFZRegionPerformanceParameters GetPerformanceData() const
	{
		return *PerfStructDataPtr.Get();
	}

	USoundWave* GetSoundwaveData() const
	{
		return SampleSimplePtr;
	}

	USFZRegion* GetRegionPointer() const
	{
		return  RawPointerToRegion;
	}

private:

	USFZRegion* RawPointerToRegion;

	TSharedPtr<FSFZRegionPerformanceParameters> PerfStructDataPtr;
	TSharedPtr<USFZRegion> RegionDataPtr;
	TSharedPtr<USoundWave> SampleDataPtr;
	USoundWave* SampleSimplePtr;
};

UCLASS(BlueprintType, Meta = (UsesHierarchy = "true", ShowInnerProperties = "true"))
class BKMUSICCORE_API USFZGroupedRegions : public UObject //, public IAudioProxyDataFactory
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "BK Music|SFZ|Region Data")
	TArray<TObjectPtr<USFZRegion>> RegionsInGroup;

	USFZRegion* GetRegionForVelocity(int velocityIn, E_SFZ_TRIGGERTYPE trigtype, int rrSeqPosition);;

	UFUNCTION(BlueprintCallable, Category = "BK Music|SFZ|Region Data")
	void GetRegionsForCondition(TArray<USFZRegion*>& InArray, bool& foundRegions, int velocityIn, int rrSeqPosition = 1,
		int GroupID = -1);
};

/**
 * @brief a parsed SFZ file referenced imported wav assets within unreal that can be used to perform an SFZ instrument via metasounds or quartz.
 */
UCLASS(BlueprintType, Category = "BK Music|SFZ")
class BKMUSICCORE_API UFKSFZAsset : public UObject
{
	GENERATED_BODY()

public:

	UFKSFZAsset();

	UPROPERTY()
	TArray<TObjectPtr<USFZRegion>> Regions;

	/**
	 * @brief Spawns a new instance of an FK SFZ Instrument performer, this object is considered the 'manager' class for a single SFZ instrument, it manages region logics for this instrument and generates new audio components using 'performNote' and 'performNoteOff' method, future versions will also manage passing CC parameters to all existing components.
	 *
	 * @param inAsset The SFZ Asset this performer will use, determines what 'musical instrument' we're creating.
	 * @param metasoundToUse the metasound used to perform the SFZ regions in all their complexities, given that only very specifically designed metasounds that receive all the parameters sent by the performers will sound correctly this parameter probably shouldn't be exposed to end users and the specific required metasound should be set in code.
	 * @return new instance of the performer.
	 */

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BK Music|SFZ")
	static TArray<UFKSFZAsset*> GetAllSFZAssets();

	//retrieves all Fusion Patches, probably shouldn't be in the FK SFZ Asset header.

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BK Music|SFZ")
	static TArray<UFusionPatch*> GetAllFusionPatchAssets();

	//can be used to get all assets of certain class
	template<typename T>
	static void GetObjectsOfClass(TArray<T*>& OutArray);

	//gets value of property within struct
	template<typename T>
	static T* GetPropertyValueWithinStruct(UObject* Target, const FName& StructPropertyName, const FName& PropertyNameWithinStruct);

	UFUNCTION(BlueprintCallable, Category = "BK Music|SFZ")
	USoundWave* GetSoundWaveForNote(const int Note, int seqPosition,
		bool& bFoundSample, int& PitchDistance);

	/**
	 *  @brief blueprint callable function that returns true or false if the key is mapped, returning the full pointer might be excessive
	 * @param bSuccess    was a mapped region found for this note
	 * @param Note       pitch of the note.
	 * @return   Region group pointer, TODO: should probably be removed or a different function should be exposed to bp.
	 */
	UFUNCTION(BlueprintCallable, Category = "BK Music|SFZ")
	USFZGroupedRegions* GetRegionGroupForNote(bool& bSuccess, int const Note);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BK Music|SFZ")
	TMap<FName, FString> ControlOpCodes;

	UPROPERTY(VisibleAnywhere, Category = "BK Music|SFZ")
	TMap<FString, FString> AssetDefines;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BK Music|SFZ")
	TMap<int, FString> ccLabelsMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BK Music|SFZ")
	TMap<int, int> ccDefaultValMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BK Music|SFZ")
	TMap<int, TObjectPtr<USFZGroupedRegions>> notesToGroupsMap;

	//TUniquePtr<Audio::IProxyData> CreateNewProxyData(const Audio::FProxyDataInitParams& InitParams) override;

	

private:
	friend class F_FK_SFZ_Asset_Proxy;
	friend class USFZAssetFactory;
	/**
	 * This needs to be called by the factory when all regions are done importing so that we actually populate the map
	 */
	void MapNotesToRanges();
	TSharedPtr<TMap<int, USFZGroupedRegions*>> DataPtr;
};

template<typename T>
inline void UFKSFZAsset::GetObjectsOfClass(TArray<T*>& OutArray)
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetData;
	AssetRegistryModule.Get().GetAssetsByClass(T::StaticClass()->GetClassPathName(), AssetData);
	for (int i = 0; i < AssetData.Num(); i++) {
		T* Object = Cast<T>(AssetData[i].GetAsset());
		OutArray.Add(Object);
	}
}

template<typename T>
inline T* UFKSFZAsset::GetPropertyValueWithinStruct(UObject* Target, const FName& StructPropertyName, const FName& PropertyNameWithinStruct)
{
	// Get the reflected struct property
	FStructProperty* StructProp = (FStructProperty*)Target->GetClass()->FindPropertyByName(StructPropertyName);

	// Get a pointer to the struct instance
	void* StructPtr = StructProp->ContainerPtrToValuePtr<void>(Target);

	// Get the struct reflection data
	UScriptStruct* StructReflectionData = StructProp->Struct;

	// Get the reflected property within struct
	FProperty* PropertyWithinStruct = StructReflectionData->FindPropertyByName(PropertyNameWithinStruct);

	// Get value of property within struct
	return PropertyWithinStruct->ContainerPtrToValuePtr<T>(StructPtr);
}
