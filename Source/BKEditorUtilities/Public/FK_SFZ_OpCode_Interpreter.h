// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "IAudioParameterInterfaceRegistry.h"
#include "IAudioParameterInterfaceRegistry.h"
//#include "Metasounds/FKSFZAudioParameterInterfaces.h"
#include "FK_SFZ_OpCode_Interpreter.generated.h"

//A simple macro to be used only in the interpreter, helps creating the big list below more easily
#define SFZ_DEF_OPCODE(a,b) SFZ_literal(TEXT(a), b),
#define SFZ_DEF_OPCODE_JERRY_SPRINGER(a,b) SFZ_literal(TEXT(#a), b),

enum ESFZ_Code_Value_Types
{
	SFZInteger,
	SFZBoolInt,
	SFZFloat,
	SFZFloatDecibels,
	TriggerEnum,
	LoopEnum,
	Note,
	SFZSample,
	SFZTime,
	SFZNormalizedTime,
	SFZ_Undefined,
	SFZ_LegatoHalfEnum,
	SFZ_Triggers //we'll pass these as trigger params, improving multi layered workflow
};

struct SFZ_literal
{
	SFZ_literal(FString String_Literal, ESFZ_Code_Value_Types Value_Type)
		: String_Literal(String_Literal),
		Value_Type(Value_Type)
	{
	}

	FString String_Literal;
	ESFZ_Code_Value_Types Value_Type;
};

/**
 * A mostly private class, we really don't need the entire class to be part of the gameplay module, probably only the enum, maybe not even that
 */
UCLASS()
class BK_EDITORUTILITIES_API UFK_SFZ_OpCode_Interpreter : public UObject
{
	GENERATED_BODY()

#define REGISTER_INPUT_INTERFACE_PARAMETER(ParamName, LayerID, Description, DataType, Init) \
	FInput SFZ_##ParamName ## { FText::FromString(#ParamName), FText::FromString(Description), DataType, \
	{FName(#ParamName), Init}}; SFZ_Inputs.Add(SFZ_##ParamName);
public:
	//static constexpr TArray<Audio::FParameterInterface::FInput> SFZ_Inputs;

	//REGISTER_INPUT_INTERFACE_PARAMETER(trigger, "LayerID", "OneTwoThree", "DataType", 100);

private:

	//friend class FK_SFZ::Metasounds::FFKSFZAudioParameterInterfaces;

	void static constexpr Add_Literal(char16_t const* String_Literal, ESFZ_Code_Value_Types Value_Type)
	{
		//Categorized_Literals.	 (SFZ_literal());
	}

	/**
	* Hopefully this will be an array of categorized literals we can easily compare to when adding op codes to the audio parameter table
	* TODO: Aliases and binds
	*/
	static inline SFZ_literal Categorized_Literals[] = {
		//special cases, trigger enums, polyphony semi-enum.
		SFZ_DEF_OPCODE("trigger", TriggerEnum)
		SFZ_DEF_OPCODE("loop_mode", LoopEnum)

		//this guy can either be an enum or an int, be careful
		SFZ_DEF_OPCODE("polyphony", SFZ_LegatoHalfEnum)

		//timed params
		SFZ_DEF_OPCODE("offset", SFZTime)
		SFZ_DEF_OPCODE("offset_oncc", SFZTime) //this guy is a hack until we sort binds, it's in order to support softdrums SFZ
		SFZ_DEF_OPCODE("loop_start", SFZTime)
		SFZ_DEF_OPCODE("loop_end", SFZTime)
		SFZ_DEF_OPCODE("loop_start", SFZTime)
		SFZ_DEF_OPCODE("end", SFZTime)
		SFZ_DEF_OPCODE("start", SFZTime)
		SFZ_DEF_OPCODE("delay_samples", SFZTime)

		//normalized time paremeters, these comd in SECONDS
		SFZ_DEF_OPCODE("delay", SFZTime)

		//integers
		SFZ_DEF_OPCODE("seq_position", SFZInteger)
		SFZ_DEF_OPCODE("seq_length", SFZInteger)
		SFZ_DEF_OPCODE("count", SFZInteger)
		SFZ_DEF_OPCODE("group", SFZInteger)
		SFZ_DEF_OPCODE("offby", SFZInteger)
		SFZ_DEF_OPCODE("note_polyphony", SFZInteger)
		//SFZ_DEF_OPCODE("group", SFZInteger)

		//BoolInts (1 == true, 0 == false)
		SFZ_DEF_OPCODE("ampeg_decay_zero", SFZBoolInt)
		SFZ_DEF_OPCODE("ampeg_dynamic", SFZBoolInt)

		//notes special case on their own, we accept either strings or numbers
		SFZ_DEF_OPCODE("pitch_keycenter", Note)
		SFZ_DEF_OPCODE("key", Note)
		SFZ_DEF_OPCODE("hikey", Note)
		SFZ_DEF_OPCODE("lokey", Note)

		//Sample (only one!)
		SFZ_DEF_OPCODE("sample", SFZSample)

		//Floats linear
		SFZ_DEF_OPCODE("ampeg_hold", SFZFloat)
		SFZ_DEF_OPCODE("ampeg_sustain", SFZFloat)
		SFZ_DEF_OPCODE("ampeg_start", SFZFloat)
		SFZ_DEF_OPCODE("ampeg_attack", SFZFloat)
		SFZ_DEF_OPCODE("ampeg_release", SFZFloat)
		SFZ_DEF_OPCODE("ampeg_decay", SFZFloat)
		SFZ_DEF_OPCODE("ampeg_delay", SFZFloat)
		SFZ_DEF_OPCODE("hivel", SFZFloat)
		SFZ_DEF_OPCODE("lovel", SFZFloat)
		SFZ_DEF_OPCODE("tune", SFZFloat)
		SFZ_DEF_OPCODE("volume", SFZFloat)
		SFZ_DEF_OPCODE("global_volume", SFZFloat)
		SFZ_DEF_OPCODE("group_volume", SFZFloat)
		SFZ_DEF_OPCODE("master_volume", SFZFloat)
		SFZ_DEF_OPCODE("amp_veltrack", SFZFloat)
		SFZ_DEF_OPCODE("ampeg_attack_shape", SFZFloat)

		SFZ_DEF_OPCODE("cutoff", SFZFloat)
		SFZ_DEF_OPCODE("cutoff2", SFZFloat)
		SFZ_DEF_OPCODE("amplitude", SFZFloat)
		SFZ_DEF_OPCODE("pitch_random", SFZFloat)

		//float decibels - we might do something different with these later
		SFZ_DEF_OPCODE("amp_random", SFZFloatDecibels)
	};

public:

	static ESFZ_Code_Value_Types Get_Value_Category_For_OpCode(const FString& KeyToProcess)
	{
		//here we iterate over the array, look if the value exist, return the enum
		for (const SFZ_literal Categorized_Literal : Categorized_Literals)
		{
			if (KeyToProcess.Contains(Categorized_Literal.String_Literal)) return Categorized_Literal.Value_Type;
		}
		// if the loop returned nothing we're dealing with an undefined opcode, we, will stick it in the map as the original string
		return SFZ_Undefined;
	}
};
