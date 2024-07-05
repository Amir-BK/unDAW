// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MetasoundBuilderSubsystem.h"
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
		namespace Categories
		{
			static const FName Default("Default");
			static const FName AudioTrack("AudioTrack");
			static const FName MetasoundLiteral("MetasoundLiteral");
		}

		UENUM(BlueprintType)
		enum PinDirection : uint8
		{
			Input,
			Output
		};



		static const TMap<FName, EVertexAutoConnectionPinCategory> PinCategoryMap = {
			{FName(TEXT("unDAW Instrument.Audio L")), EVertexAutoConnectionPinCategory::AudioStreamL},
			{FName(TEXT("unDAW Instrument.Audio R")), EVertexAutoConnectionPinCategory::AudioStreamR},
			{FName(TEXT("unDAW Insert.Audio L")), EVertexAutoConnectionPinCategory::AudioStreamL},
			{FName(TEXT("unDAW Insert.Audio R")), EVertexAutoConnectionPinCategory::AudioStreamR},
			{FName(TEXT("unDAW Insert.Audio In L")), EVertexAutoConnectionPinCategory::AudioStreamL},
			{FName(TEXT("unDAW Insert.Audio In R")), EVertexAutoConnectionPinCategory::AudioStreamR}
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

UCLASS()
class BKMUSICCORE_API UM2Pins : public UObject
{
	GENERATED_BODY()
	
public:

	UM2Pins() {};

	bool bIsStale = false;

	virtual void BuildCompositePin(const UMetaSoundSourceBuilder& BuilderContext) {};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMetaSoundBuilderResult BuildResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EMetaSoundBuilderResult ConnectionResult;

protected:

	friend class UM2SoundVertex;

	UPROPERTY()
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

	void BuildCompositePin(const UMetaSoundSourceBuilder& BuilderContext) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UM2MetasoundLiteralPin* AudioStreamL;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UM2MetasoundLiteralPin* AudioStreamR;

	UM2AudioTrackPin() 
	{
		Category = M2Sound::Pins::Categories::AudioTrack;
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

		Category = M2Sound::Pins::Categories::MetasoundLiteral;
	};

protected:


};



