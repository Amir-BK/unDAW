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
		enum PinDirection : uint8
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

	UM2Pins() {};

	virtual FLinearColor GetPinColor() const { return FLinearColor::White; }

	UPROPERTY(VisibleAnywhere)
	UM2Pins* LinkedPin;

	bool bIsStale = false;

	virtual void BuildCompositePin(const UMetaSoundSourceBuilder& BuilderContext) {};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMetaSoundBuilderResult BuildResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMetaSoundBuilderResult ConnectionResult;

protected:

	friend class UM2SoundVertex;
	friend class UM2SoundEdGraphNode;
	friend class UDAWSequencerData;

	UPROPERTY(VisibleAnywhere)
	UM2SoundVertex* ParentVertex;

	FGuid NodeID;
	FGuid VertexID;

public:


	template <typename T>
	T GetHandle() const
	{
		T Handle = T();
		Handle.NodeID = NodeID;
		Handle.VertexID = VertexID;

		return Handle;
	}

	template <typename T>
	void SetHandle(T Handle)
	{
		NodeID = Handle.NodeID;
		VertexID = Handle.VertexID;

	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName Name;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	M2Sound::Pins::PinDirection Direction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName Category;

};

UCLASS()
class BKMUSICCORE_API UM2AudioTrackPin final : public UM2Pins
{
	GENERATED_BODY()

public:

	
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
	UM2MetasoundLiteralPin* AudioStreamL;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UM2MetasoundLiteralPin* AudioStreamR;

	UM2AudioTrackPin() 
	{
		Category = M2Sound::Pins::AutoDiscovery::AudioTrack;
	};



};

UCLASS()
class BKMUSICCORE_API UM2MetasoundLiteralPin final : public UM2Pins
{
	GENERATED_BODY()
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FName DataType;


	UM2MetasoundLiteralPin() 
	{

		Category = M2Sound::Pins::AutoDiscovery::MetasoundLiteral;
	};

protected:


};



