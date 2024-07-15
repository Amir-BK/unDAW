// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MetasoundBuilderSubsystem.h"
#include "MetasoundDataReference.h"
#include "MetasoundLiteral.h"
#include "MetasoundTrigger.h"
#include "MetasoundWaveTable.h"
#include "MetasoundTime.h"

#include "MetasoundAudioBuffer.h"
#include "MetasoundAssetBase.h"
#include "MetasoundFrontendDataTypeRegistry.h"
#include "WaveTable.h"
#include "M2Pins.generated.h"

UENUM()
enum class EVertexAutoConnectionPinCategory : uint8
{
	MidiTrackStream, //pairs a midistream with a tracknum int - really we wouldn't need the tracknum but the fusion sampler requires it, other custom metasound instruments don't need it as the stream is already filtered for chan/track
	MidiTrackTrackNum, //pairs a tracknum with a tracknum int - used to filter the midi stream for a specific track
	AudioStreamL, //right now this is just a convenient wrapper for two audio channels, in the future we'd like to use this to allow multichannel audio
	AudioStreamR,
};

namespace M2Sound
{
	namespace Pins
	{
		namespace AutoDiscovery
		{
			static const FName Default("Default");
			static const FName AudioTrack("AudioTrack");
			static const FName MetasoundLiteral("MetasoundLiteral");
			//unreal interface outputs, for now stereo only
			static const FName StereoLeft("UE.OutputFormat.Stereo.Audio:0");
			static const FName StereoRight("UE.OutputFormat.Stereo.Audio:1");
		}

		UENUM(BlueprintType)
			enum EPinDirection : uint8
		{
			Input,
			Output
		};

		namespace PinCategories
		{
			using namespace Metasound;
			const FName PinSubCategoryObject = "object"; // Basket for all UObject proxy types (corresponds to multiple DataTypes)
			const FName PinSubCategoryBoolean = GetMetasoundDataTypeName<bool>();
			const FName PinSubCategoryFloat = GetMetasoundDataTypeName<float>();
			const FName PinSubCategoryInt32 = GetMetasoundDataTypeName<int32>();
			const FName PinSubCategoryString = GetMetasoundDataTypeName<FString>();

			// Categories corresponding with MetaSound DataTypes with custom visualization
			//const FName PinCategoryAudio = GetMetasoundDataTypeName<FAudioBuffer>();
			//const FName PinCategoryTime = GetMetasoundDataTypeName<FTime>();
			//const FName PinCategoryTimeArray = GetMetasoundDataTypeName<TArray<FTime>>();
			//const FName PinCategoryTrigger = GetMetasoundDataTypeName<FTrigger>();
			//const FName PinCategoryWaveTable = GetMetasoundDataTypeName<WaveTable::FWaveTable>();
		}

		static const TMap<FName, EVertexAutoConnectionPinCategory> PinCategoryMap = {
			{FName(TEXT("unDAW Instrument.Audio L")), EVertexAutoConnectionPinCategory::AudioStreamL},
			{FName(TEXT("unDAW Instrument.Audio R")), EVertexAutoConnectionPinCategory::AudioStreamR},
			{FName(TEXT("unDAW Insert.Audio L")), EVertexAutoConnectionPinCategory::AudioStreamL},
			{FName(TEXT("unDAW Insert.Audio R")), EVertexAutoConnectionPinCategory::AudioStreamR},
			{FName(TEXT("unDAW Insert.Audio In L")), EVertexAutoConnectionPinCategory::AudioStreamL},
			{FName(TEXT("unDAW Insert.Audio In R")), EVertexAutoConnectionPinCategory::AudioStreamR},
			{AutoDiscovery::StereoLeft, EVertexAutoConnectionPinCategory::AudioStreamL},
			{AutoDiscovery::StereoRight, EVertexAutoConnectionPinCategory::AudioStreamR}
		};

		struct FVariWidthAudioOutput
		{
			FName PlaceHolderData;
		};
	}
}
/**
 *
 */
 //using namespace M2Sound::Pins;

UCLASS(DefaultToInstanced)
class BKMUSICCORE_API UM2Pins : public UObject
{
	GENERATED_BODY()

public:

	UM2Pins() : NodeId(), VertexId(), Name()  {};

	virtual FLinearColor GetPinColor() const { return FLinearColor::White; }


	template <typename T>
	T GetHandle() const
	{
		T Handle = T();
		Handle.NodeID = NodeId;
		Handle.VertexID = VertexId;

		return Handle;
	}

	template <typename T>
	void SetHandle(T Handle)
	{
		NodeId = Handle.NodeID;
		VertexId = Handle.VertexID;
	}

	virtual void BuildCompositePin(const UMetaSoundSourceBuilder& BuilderContext) {};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMetaSoundBuilderResult BuildResult = EMetaSoundBuilderResult::Succeeded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMetaSoundBuilderResult ConnectionResult = EMetaSoundBuilderResult::Succeeded;

protected:

	friend class UM2SoundVertex;
	friend class UM2SoundEdGraphNode;
	friend class UDAWSequencerData;


	FGuid NodeId;
	FGuid VertexId;

public:
	UPROPERTY(VisibleAnywhere)
	UM2SoundVertex* ParentVertex = nullptr;

	UPROPERTY(VisibleAnywhere)
	UM2Pins* LinkedPin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UM2Pins>> LinkedPins;

	bool bIsStale = false;

	UPROPERTY()
	bool bIsColorSource = false;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName Name;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	M2Sound::Pins::EPinDirection Direction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName Category;

	//allows user to cofigure which pin will determine the color of the vertex
	UPROPERTY()
	bool  bPinIsColorSource;
};

UCLASS()
class BKMUSICCORE_API UM2AudioTrackPin final : public UM2Pins
{
	GENERATED_BODY()

public:


	UM2AudioTrackPin()
	{
		Category = M2Sound::Pins::AutoDiscovery::AudioTrack;
	};

	//convenience parameter for nodes that may accept more than one audio track, such as the vari mixer
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int ChannelIndex = INDEX_NONE;

	//really a convenience, does nothing if not specifically implemented in the node, such as the varimixer
	//perhaps I'll make more generic use of this in the future with crossfade nodes and other smaller mixer nodes
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float GainValue = 1.0f;

	UPROPERTY()
	bool bMute = false;

	UPROPERTY()
	bool bSolo = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UM2MetasoundLiteralPin* AudioStreamL = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UM2MetasoundLiteralPin* AudioStreamR = nullptr;

};

UCLASS()
class BKMUSICCORE_API UM2MetasoundLiteralPin final : public UM2Pins
{
	GENERATED_BODY()
public:

	UM2MetasoundLiteralPin()
	{
		Category = M2Sound::Pins::AutoDiscovery::MetasoundLiteral;
	};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName DataType;

	// mostly used for trigger pins so that they can be used and simulated without a user created input
	UPROPERTY()
	FName AutoGeneratedGraphInputName = NAME_None;

};
