// Copyright Epic Games, Inc. All Rights Reserved.

#include "HarmonixMetasound/Nodes/MidiPulseGeneratorNode.h"

#include "MetasoundExecutableOperator.h"
#include "MetasoundFacade.h"
#include "MetasoundNodeRegistrationMacro.h"
#include "MetasoundStandardNodesCategories.h"

#include "HarmonixMetasound/DataTypes/MidiClock.h"
#include "HarmonixMetasound/MidiOps/PulseGenerator.h"

#define LOCTEXT_NAMESPACE "unDAWMetasounds_MidiArpGeneratorNode"

namespace unDAWMetasounds::MidiArpGeneratorNode
{
	const Metasound::FNodeClassName& GetClassName()
	{
		static const Metasound::FNodeClassName ClassName{ "unDAW", "MidiArpGenerator", "" };
		return ClassName;
	}

	int32 GetCurrentMajorVersion()
	{
		return 0;
	}

	namespace Inputs
	{
		DEFINE_INPUT_METASOUND_PARAM(MidiClock, "MidiClock", "MidiClock");
		DEFINE_INPUT_METASOUND_PARAM(Interval, "Interval", "The musical time interval at which to send out the pulse");
		DEFINE_INPUT_METASOUND_PARAM(IntervalMultiplier, "Interval Multiplier", "Multiplies the interval, 1 to use just the value of Interval");
		DEFINE_INPUT_METASOUND_PARAM(Offset, "Offset", "Offsets the pulse by a musical time");
		DEFINE_INPUT_METASOUND_PARAM(OffsetMultiplier, "Offset Multiplier", "Multiplies the offset, 0 for no offset");
		DEFINE_INPUT_METASOUND_PARAM(MidiTrack, "MidiTrack Number", "Track Number");
		DEFINE_INPUT_METASOUND_PARAM(MidiChannel, "MidiChannel", "MidiChannel");
		DEFINE_INPUT_METASOUND_PARAM(MidiNoteNumber, "Note Number", "The note number to play");
		DEFINE_INPUT_METASOUND_PARAM(MidiVelocity, "Velocity", "The velocity at which to play the note");
	}

	namespace Outputs
	{
		DEFINE_OUTPUT_METASOUND_PARAM(MidiStream, "MidiStream", "MidiStream");
	}

	class FMidiArpGeneratorOperator final : public Metasound::TExecutableOperator<FMidiArpGeneratorOperator>
	{
	public:
		static const Metasound::FNodeClassMetadata& GetNodeInfo()
		{
			using namespace Metasound;

			auto InitNodeInfo = []() -> FNodeClassMetadata
				{
					FNodeClassMetadata Info;
					Info.ClassName = GetClassName();
					Info.MajorVersion = 0;
					Info.MinorVersion = 1;
					Info.DisplayName = INVTEXT("MIDI Arp Generator");
					Info.Description = METASOUND_LOCTEXT("MidiPulseGeneratorNode_Description", "Outputs a repeated MIDI note at the specified musical time interval");
					Info.Author = PluginAuthor;
					Info.PromptIfMissing = PluginNodeMissingPrompt;
					Info.DefaultInterface = GetVertexInterface();
					Info.CategoryHierarchy = { INVTEXT("unDAW"), NodeCategories::Music };
					return Info;
				};

			static const FNodeClassMetadata Info = InitNodeInfo();

			return Info;
		}

		static const Metasound::FVertexInterface& GetVertexInterface()
		{
			using namespace Metasound;

			static const Harmonix::Midi::Ops::FPulseGenerator PulseGeneratorForDefaults;
			const auto DefaultInterval = PulseGeneratorForDefaults.GetInterval();

			static const FVertexInterface Interface
			{
				FInputVertexInterface {
					TInputDataVertex<HarmonixMetasound::FMidiClock>(METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MidiClock)),
					TInputDataVertex<FEnumMidiClockSubdivisionQuantizationType>(
						METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::Interval), static_cast<int32>(DefaultInterval.Interval)),
					TInputDataVertex<int32>(
						METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::IntervalMultiplier), static_cast<int32>(DefaultInterval.IntervalMultiplier)),
					TInputDataVertex<FEnumMidiClockSubdivisionQuantizationType>(
						METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::Offset), static_cast<int32>(DefaultInterval.Offset)),
					TInputDataVertex<int32>(
						METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::OffsetMultiplier), static_cast<int32>(DefaultInterval.OffsetMultiplier)),
					TInputDataVertex<int32>(
						METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MidiTrack), static_cast<int32>(PulseGeneratorForDefaults.Track)),
					TInputDataVertex<int32>(
						METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MidiChannel), static_cast<int32>(PulseGeneratorForDefaults.Channel)),
					TInputDataVertex<int32>(
						METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MidiNoteNumber), static_cast<int32>(PulseGeneratorForDefaults.NoteNumber)),
					TInputDataVertex<int32>(
						METASOUND_GET_PARAM_NAME_AND_METADATA(Inputs::MidiVelocity), static_cast<int32>(PulseGeneratorForDefaults.Velocity))
				},
				FOutputVertexInterface
				{
					TOutputDataVertex<HarmonixMetasound::FMidiStream>(METASOUND_GET_PARAM_NAME_AND_METADATA(Outputs::MidiStream))
				}
			};

			return Interface;
		}

		struct FInputs
		{
			HarmonixMetasound::FMidiClockReadRef Clock;
			Metasound::FEnumMidiClockSubdivisionQuantizationReadRef Interval;
			Metasound::FInt32ReadRef IntervalMultiplier;
			Metasound::FEnumMidiClockSubdivisionQuantizationReadRef Offset;
			Metasound::FInt32ReadRef OffsetMultiplier;
			Metasound::FInt32ReadRef Track;
			Metasound::FInt32ReadRef Channel;
			Metasound::FInt32ReadRef NoteNumber;
			Metasound::FInt32ReadRef Velocity;
		};

		struct FOutputs
		{
			HarmonixMetasound::FMidiStreamWriteRef MidiStream;
		};

		static TUniquePtr<IOperator> CreateOperator(const Metasound::FBuildOperatorParams& InParams, Metasound::FBuildResults& OutResults)
		{
			FInputs Inputs
			{
				InParams.InputData.GetOrConstructDataReadReference<HarmonixMetasound::FMidiClock>(Inputs::MidiClockName, InParams.OperatorSettings),
				InParams.InputData.GetOrCreateDefaultDataReadReference<Metasound::FEnumMidiClockSubdivisionQuantizationType>(
					Inputs::IntervalName, InParams.OperatorSettings),
				InParams.InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::IntervalMultiplierName, InParams.OperatorSettings),
				InParams.InputData.GetOrCreateDefaultDataReadReference<Metasound::FEnumMidiClockSubdivisionQuantizationType>(
					Inputs::OffsetName, InParams.OperatorSettings),
				InParams.InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::OffsetMultiplierName, InParams.OperatorSettings),
				InParams.InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::MidiTrackName, InParams.OperatorSettings),
				InParams.InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::MidiChannelName, InParams.OperatorSettings),
				InParams.InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::MidiNoteNumberName, InParams.OperatorSettings),
				InParams.InputData.GetOrCreateDefaultDataReadReference<int32>(Inputs::MidiVelocityName, InParams.OperatorSettings),
			};

			FOutputs Outputs
			{
				HarmonixMetasound::FMidiStreamWriteRef::CreateNew()
			};

			return MakeUnique<FMidiArpGeneratorOperator>(InParams, MoveTemp(Inputs), MoveTemp(Outputs));
		}

		FMidiArpGeneratorOperator(const Metasound::FBuildOperatorParams& InParams, FInputs&& InInputs, FOutputs&& InOutputs)
			: Inputs(MoveTemp(InInputs))
			, Outputs(MoveTemp(InOutputs))
		{
			Reset(InParams);
		}

		void Reset(const FResetParams&)
		{
			ScaleDegrees = { 0, 2, 4, 5, 7, 9, 11 };
			PulseGenerator.SetClock(Inputs.Clock->AsShared());
			ApplyParameters();
		}

		virtual void BindInputs(Metasound::FInputVertexInterfaceData& InVertexData) override
		{
			InVertexData.BindReadVertex(Inputs::MidiClockName, Inputs.Clock);
			InVertexData.BindReadVertex(Inputs::IntervalName, Inputs.Interval);
			InVertexData.BindReadVertex(Inputs::IntervalMultiplierName, Inputs.IntervalMultiplier);
			InVertexData.BindReadVertex(Inputs::OffsetName, Inputs.Offset);
			InVertexData.BindReadVertex(Inputs::OffsetMultiplierName, Inputs.OffsetMultiplier);
			InVertexData.BindReadVertex(Inputs::MidiTrackName, Inputs.Track);
			InVertexData.BindReadVertex(Inputs::MidiChannelName, Inputs.Channel);
			InVertexData.BindReadVertex(Inputs::MidiNoteNumberName, Inputs.NoteNumber);
			InVertexData.BindReadVertex(Inputs::MidiVelocityName, Inputs.Velocity);

			PulseGenerator.SetClock(Inputs.Clock->AsShared());
			ApplyParameters();
		}

		virtual void BindOutputs(Metasound::FOutputVertexInterfaceData& InVertexData) override
		{
			InVertexData.BindReadVertex(Outputs::MidiStreamName, Outputs.MidiStream);
		}

		void Execute()
		{
			ApplyParameters();

			PulseGenerator.Process(*Outputs.MidiStream);
		}
	private:
		void ApplyParameters()
		{
			int RandomIndex = FMath::RandRange(0, ScaleDegrees.Num() - 1);
			int32 ArpVal = this->ScaleDegrees[RandomIndex];

			PulseGenerator.Track = *Inputs.Track;
			PulseGenerator.Channel = *Inputs.Channel;
			PulseGenerator.NoteNumber = *Inputs.NoteNumber + ArpVal;
			PulseGenerator.Velocity = *Inputs.Velocity;
			PulseGenerator.SetInterval(
				{
					*Inputs.Interval,
					*Inputs.Offset,
					static_cast<uint16>(*Inputs.IntervalMultiplier),
					static_cast<uint16>(*Inputs.OffsetMultiplier)
				});
		}

		TArray<int32> ScaleDegrees;
		int32 OctaveRange;
		FInputs Inputs;
		FOutputs Outputs;
		Harmonix::Midi::Ops::FPulseGenerator PulseGenerator;
	};

	class FMidiArpGeneratorNode final : public Metasound::FNodeFacade
	{
	public:
		explicit FMidiArpGeneratorNode(const Metasound::FNodeInitData& InInitData)
			: FNodeFacade(InInitData.InstanceName, InInitData.InstanceID, Metasound::TFacadeOperatorClass<FMidiArpGeneratorOperator>())
		{}
	};

	METASOUND_REGISTER_NODE(FMidiArpGeneratorNode)
}

#undef LOCTEXT_NAMESPACE