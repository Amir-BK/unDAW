// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Metasound.h"
#include "MetasoundBuilderSubsystem.h"
#include "M2SoundGraphData.h"

#include <Pins/M2Pins.h>
#include "BKMusicCore.h"

#include "M2SoundVertex.generated.h"

BKMUSICCORE_API DECLARE_LOG_CATEGORY_EXTERN(unDAWVertexLogs, Verbose, All);

//class UM2SoundGraphData;

struct FBuilderVertexCompositeData;

class FVertexCreator
{
public:

	template<typename T>
	static inline T* CreateVertex(UDAWSequencerData* InGraphData)
	{
		T* Vertex = NewObject<T>(InGraphData, NAME_None, RF_Transactional);
		Vertex->SequencerData = InGraphData;

		return Vertex;
	}
};

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EM2SoundPinFlags : uint8
{
	None = 0 UMETA(Hidden),
	IsAutoManaged = 1 << 0,
	ExposeAsMetasoundOutput = 1 << 1,
	//ShowInBoth = ShowInGraph | ShowInDetails,
};
ENUM_CLASS_FLAGS(EM2SoundPinFlags)

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EM2SoundPinDisplayFlags : uint8
{
	None = 0 UMETA(Hidden),
	ShowInGraph = 1 << 0,
	ShowInGame = 1 << 1,
};
ENUM_CLASS_FLAGS(EM2SoundPinDisplayFlags)

USTRUCT(BlueprintType)
struct FM2SoundPinData
{
	GENERATED_BODY()

	EEdGraphPinDirection Direction;

	UPROPERTY(EditAnywhere)
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere)
	float MaxValue = 1.0f;

	//to init the slider correctly
	UPROPERTY(VisibleAnywhere)
	float NormalizedValue = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FName PinName;

	//the underlying metasound datatype
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FName DataType;

	UPROPERTY(VisibleAnywhere, meta = (Bitmask, BitmaskEnum = "/Script/BKMusicCore.EM2SoundPinFlags"))
	uint8 PinFlags;

	UPROPERTY(EditAnywhere, meta = (Bitmask, BitmaskEnum = "/Script/BKMusicCore.EM2SoundPinDisplayFlags"))
	uint8 DisplayFlags;

	UPROPERTY()
	FMetasoundFrontendLiteral LiteralValue;

	//not the most elegant
	FMetaSoundBuilderNodeInputHandle InputHandle;

	FMetaSoundBuilderNodeOutputHandle OutputHandle;
};

UCLASS(Abstract, AutoExpandCategories = ("M2Sound"))
class BKMUSICCORE_API UM2SoundVertex : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "M2Sound")
	bool bIsTransient = false;

	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	TObjectPtr<UM2Pins> VertexMetadataProviderPin = nullptr;

	UPROPERTY()
	TOptional<FLinearColor> VertexColor;

	virtual FLinearColor GetVertexColor() const;

	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	TOptional<FString> VertexDisplayName;

	virtual FString GetVertexDisplayName() const;;

	bool bIsRebuilding = false; //we can use this one to know when we also need to update all connected pins rather than just the input as we do when building the graph, only relevant for nodes that expose patches to the user

	void PopulatePinsFromMetasoundData(const TArray<FMetaSoundBuilderNodeInputHandle>& InHandles, const TArray<FMetaSoundBuilderNodeOutputHandle>& OutHandles);

	//for book keeping and tracking the color of the track in the sequencer, can also be used to assosciate groups of vertexes for the MIXER (that is to come)
	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	int TrackId = INDEX_NONE;

	void UpdateValueForPin(FM2SoundPinData& Pin, FMetasoundFrontendLiteral& NewValue);

	UPROPERTY(BlueprintAssignable, Category = "M2Sound")
	FOnVertexUpdated OnVertexUpdated;
protected:
	UPROPERTY()
	TArray<FMetaSoundBuilderNodeOutputHandle> OutPins;

	UPROPERTY()
	TArray<FMetaSoundBuilderNodeInputHandle> InPins;
public:

	UFUNCTION()
	UDAWSequencerData* GetSequencerData() const;

	UMetaSoundSourceBuilder& GetBuilderContext() const;

	UPROPERTY()
	TObjectPtr<UDAWSequencerData> SequencerData;

	UPROPERTY()
	FString VertexErrors;

	UPROPERTY()
	bool bHasErrors = false;

	UPROPERTY()
	int32 ErrorType = EMessageSeverity::Info;

	UFUNCTION()
	virtual void VertexNeedsBuilderUpdates();

	UFUNCTION()
	virtual void VertexConnectionsChanged();

	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	TMap<FName, EMetaSoundBuilderResult> BuilderResults;

	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	TMap<FName, EMetaSoundBuilderResult> BuilderConnectionResults;

	void TransmitAudioParameter(FAudioParameter Parameter);

	virtual void CollectAndTransmitAudioParameters() {};

	virtual void TryFindVertexDefaultRangesInCache() {};

	bool bIsAudioOutputVertex = false;

	//This is a key method, should be renamed and probably refactored into being called automatically via a pure virtual method on the
	//patch vertex base class
	virtual void CollectParamsForAutoConnect();

	UPROPERTY()
	bool bHideInGraph = false;

	virtual void BuildVertex() {};

	virtual void UpdateConnections();

	virtual void DestroyVertex() {};

	//should be private or something, called when the node is destroyed, unregisters the vertex from its input before calling the actual destroy method
	void DestroyVertexInternal();

	UPROPERTY(VisibleAnywhere)
	TMap<FName, TObjectPtr<UM2Pins>> InputM2SoundPins;

	UPROPERTY(VisibleAnywhere)
	TMap<FName, TObjectPtr<UM2Pins>>  OutputM2SoundPins;

	void MarkAllPinsStale();

	void RemoveAllStalePins();

	template<typename T>
	inline T* CreatePin()
	{
		T* NewPin = NewObject<T>(this);
		NewPin->ParentVertex = this;
		//Pins.Add(NewPin);

		return NewPin;
	}
public:

	UM2AudioTrackPin* CreateAudioTrackInputPin(FName PinName = NAME_None)
	{
		UM2AudioTrackPin* NewPin = CreatePin<UM2AudioTrackPin>();
		NewPin->Direction = M2Sound::Pins::EPinDirection::Input;
		if (PinName != NAME_None)
		{
			NewPin->Name = PinName;
		}
		else
		{
			NewPin->Name = M2Sound::Pins::AutoDiscovery::AudioTrack;
		}

		//NewPin->CreateCompositePin(GetBuilderContext());

		return NewPin;
	}

	UM2AudioTrackPin* CreateAudioTrackOutputPin()
	{
		UM2AudioTrackPin* NewPin = CreatePin<UM2AudioTrackPin>();
		NewPin->Direction = M2Sound::Pins::EPinDirection::Output;
		NewPin->Name = M2Sound::Pins::AutoDiscovery::AudioTrack;
		//NewPin->CreateCompositePin(GetBuilderContext());

		return NewPin;
	}

	template<typename T>
	T* CreateInputPin(FMetaSoundBuilderNodeInputHandle InHandle)
	{
		T* NewPin = CreatePin<T>();
		NewPin->Direction = M2Sound::Pins::EPinDirection::Input;
		NewPin->SetHandle(InHandle);
		//NewPin->CreateCompositePin(GetBuilderContext());

		return NewPin;
	}

	template<typename T>
	T* CreateOutputPin(FMetaSoundBuilderNodeOutputHandle InHandle)
	{
		T* NewPin = CreatePin<T>();
		NewPin->Direction = M2Sound::Pins::EPinDirection::Output;

		NewPin->SetHandle(InHandle);
		//NewPin->CreateCompositePin(GetBuilderContext());
		return NewPin;
	}

	UM2MetasoundLiteralPin* CreateWildCardInPin()
	{
		UM2MetasoundLiteralPin* NewPin = CreatePin<UM2MetasoundLiteralPin>();
		NewPin->Direction = M2Sound::Pins::EPinDirection::Input;
		NewPin->DataType = M2Sound::Pins::AutoDiscovery::WildCard;
		NewPin->Name = FName(TEXT("DynamicOutput"));
		//NewPin->CreateCompositePin(GetBuilderContext());

		return NewPin;
	}

	UM2MetasoundLiteralPin* CreateWildCardOutPin()
	{
		UM2MetasoundLiteralPin* NewPin = CreatePin<UM2MetasoundLiteralPin>();
		NewPin->Direction = M2Sound::Pins::EPinDirection::Output;
		NewPin->DataType = M2Sound::Pins::AutoDiscovery::WildCard;
		NewPin->Name = FName(TEXT("DynamicInput"));
		//NewPin->CreateCompositePin(GetBuilderContext());

		return NewPin;
	}

	template<>
	UM2MetasoundLiteralPin* CreateOutputPin(FMetaSoundBuilderNodeOutputHandle InHandle)
	{
		UM2MetasoundLiteralPin* NewPin = CreatePin<UM2MetasoundLiteralPin>();
		NewPin->Direction = M2Sound::Pins::EPinDirection::Output;
		GetBuilderContext().GetNodeOutputData(InHandle, NewPin->Name, NewPin->DataType, NewPin->BuildResult);
		//NewPin->Name = InName;

		NewPin->SetHandle(InHandle);

		return NewPin;
	}

	template<>
	UM2MetasoundLiteralPin* CreateInputPin(FMetaSoundBuilderNodeInputHandle InHandle)
	{
		UM2MetasoundLiteralPin* NewPin = CreatePin<UM2MetasoundLiteralPin>();
		NewPin->Direction = M2Sound::Pins::EPinDirection::Input;
		GetBuilderContext().GetNodeInputData(InHandle, NewPin->Name, NewPin->DataType, NewPin->BuildResult);
		NewPin->LiteralValue = GetBuilderContext().GetNodeInputClassDefault(InHandle, NewPin->BuildResult);


		NewPin->SetHandle(InHandle);

		return NewPin;
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};


UCLASS(Abstract)
//this is a literal node that can be used to pass a value to the metasound graph
class BKMUSICCORE_API UM2SoundLiteralNodeVertex : public UM2SoundVertex
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FMetaSoundNodeHandle NodeHandle;

	void DestroyVertex() override;
};

//the audio output is a bit of a special case due to the way the output graph works, we need to add another channel to the mixer for each one
// for now it can remain as a separate node, we'll perhaps expand it once we support sends

//to start making sense of things here, the audio output is NOT a literal node handle - why? because it represents a just some output pins and not a complete node, this is why we need to create a separate class for it
UCLASS()
class BKMUSICCORE_API UM2SoundAudioOutput : public UM2SoundVertex
{
	GENERATED_BODY()

public:

	UM2SoundAudioOutput() { bIsAudioOutputVertex = true; }

	UPROPERTY(EditAnywhere, Category = "M2Sound")
	float Gain = 1.0f;

	UPROPERTY(VisibleAnywhere)
	FName GainParameterName = NAME_None;

	UPROPERTY(VisibleAnywhere)
	FAssignableAudioOutput AudioOutput;

	// Inherited via UM2SoundVertex

	void BuildVertex() override;

	//void UpdateConnections() override;

	void DestroyVertex() override;

	void CollectAndTransmitAudioParameters() override;
};

//All vertex that from the perspective of the metasound builder can be reduced to a single FMetaSoundBuilderNodeOutputHandle - such as the midi input vertexes
// these do not expose the actual metasound node, but rather the output handle that can be used to connect to other nodes - this abtracts the midifilters and other similar nodes from the m2sound graph
// by our paradigm we probably don't need to implement 'update connections' for these, but it might be useful for the 'expose to outputs' function.
UCLASS()

class BKMUSICCORE_API UM2SoundBuilderInputHandleVertex : public UM2SoundVertex
{
	GENERATED_BODY()

	//virtual FString GetUniqueParameterName() = ;
	FMetaSoundBuilderNodeOutputHandle OutputHandle;

public:
	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	FName MemberName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "M2Sound")
	bool bOutputToBlueprints = true;

	UPROPERTY(VisibleAnywhere, Category = "M2Sound")
	FString TrackPrefix;

	// Inherited via UM2SoundVertex

	void BuildVertex() override;

	FLinearColor GetVertexColor() const override;

	//anvoid InitFromMemberName(FName InMemberName);
};

UCLASS()

class BKMUSICCORE_API UM2SoundDynamicGraphInputVertex : public UM2SoundBuilderInputHandleVertex
{
	GENERATED_BODY()

public:
	//UM2SoundDynamicGraphInputVertex() { CreateWildCardOutPin(); }
	//UPROPERTY()
	//FName MemberName;
	void RenameInput(FName InMemberName);

	virtual void BuildVertex() override;

	bool bIsSet = false;
};

UCLASS()
class BKMUSICCORE_API UM2SoundMidiInputVertex : public UM2SoundBuilderInputHandleVertex
{
	GENERATED_BODY()

public:


	//Represents the index of the track in the sequencer data, to get the actual midi metadata use this index to get the track from the sequencer data
};

//vertex container for user created metasound patch assets, of course this one is a literal node.
UCLASS()
class BKMUSICCORE_API UM2SoundPatch : public UM2SoundLiteralNodeVertex, public UnDAW::IMetasoundAssetListener
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "M2Sound")
	TObjectPtr <UMetaSoundPatch> Patch;

	UFUNCTION(CallInEditor, Category = "M2Sound")
	void SaveDefaultsToVertexCache();

	//for now can be used to rebuild the node manually if the user changes the metasound patch
	bool bForceRebuild = false;

	// Inherited via UM2SoundVertex

	void BuildVertex() override;

	//void UpdateConnections() override;

	void TryFindVertexDefaultRangesInCache() override;

	~UM2SoundPatch();

	// Inherited via IMetasoundAssetListener
	virtual void MetasoundDocumentUpdated() override;



};
