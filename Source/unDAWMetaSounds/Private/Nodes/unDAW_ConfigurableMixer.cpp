// Copyright Epic Games, Inc. All Rights Reserved.


#include "DSP/Dsp.h"
#include "DSP/BufferVectorOperations.h"
#include "DSP/FloatArrayMath.h"

#include "MetasoundAudioBuffer.h"
#include "MetasoundExecutableOperator.h"
#include "MetasoundFacade.h"
#include "MetasoundNodeRegistrationMacro.h"
#include "MetasoundOperatorSettings.h"
#include "MetasoundPrimitives.h"
#include "MetasoundStandardNodesNames.h"
#include "MetasoundTrigger.h"
#include "MetasoundVertex.h"
#include "MetasoundFrontendDocument.h"
#include "MetasoundOperatorData.h"
#include "MetasoundStandardNodesCategories.h"
#include "Nodes/unDAW_ConfigurableMixer.h"

#define LOCTEXT_NAMESPACE "unDAWMetasounds_ConfigurableMixerNode"

namespace Metasound::ConfigurableMixerPrivate
{
	const FLazyName InputBaseName{ "In" };
	const FLazyName InputLeftBaseName{ "InL" };
	const FLazyName InputRightBaseName{ "InR" };
	const FLazyName GainInputBaseName{ "Gain" };
	const FLazyName PanInputBaseName{ "Pan" };
	const FLazyName OutputLeftBaseName{ "OutLeft" };
	const FLazyName OutputRightBaseName{ "OutRight" };
#if WITH_EDITOR
	const FText InputTooltip = LOCTEXT("InCh_Tooltip", "Channel");
	const FText GainInputTooltip = LOCTEXT("Gain_Tooltip", "Gain");
	const FText PanInputTooltip = LOCTEXT("Pan_Tooltip", "Pan");
	const FText OutputTooltip = LOCTEXT("OutCh_Tooltip", "Channel");

#else
	const FText InputTooltip = FText::GetEmpty();
#endif
	

	FVertexInterface GetVertexInterface(int32 NumInputs)
	{
		FInputVertexInterface InputInterface;
		FOutputVertexInterface OutputInterface;
		for (int32 i = 0; i < NumInputs; i++)
		{
		/*	InputInterface.Add(MakeInputDataVertex(i));*/
			FName LeftInputName = InputLeftBaseName;
			LeftInputName.SetNumber(i);

#if WITH_EDITOR
			const FText InputLeftDisplayName = FText::Format(LOCTEXT("InCh_Name", "In {0} L"), i);
#else
			const FText InputLeftDisplayName = FText::GetEmpty();
#endif
			InputInterface.Add(TInputDataVertex<FAudioBuffer>{LeftInputName, FDataVertexMetadata{ InputTooltip, InputLeftDisplayName }});

			FName RightInputName = InputRightBaseName;
			RightInputName.SetNumber(i);
#if WITH_EDITOR
			const FText InputRightDisplayName = FText::Format(LOCTEXT("InCh_Name", "In {0} R"), i);
#else
			const FText InputRightDisplayName = FText::GetEmpty();
#endif
			InputInterface.Add(TInputDataVertex<FAudioBuffer>{RightInputName, FDataVertexMetadata{ InputTooltip, InputRightDisplayName }});

			//gain, pan
			FName GainInputName = GainInputBaseName;
			GainInputName.SetNumber(i);

#if WITH_EDITOR
			const FText GainInputDisplayName = FText::Format(LOCTEXT("Gain_Name", "Gain {0}"), i);
#else
			const FText GainInputDisplayName = FText::GetEmpty();
#endif
			InputInterface.Add(TInputDataVertex<float>{GainInputName, FDataVertexMetadata{ InputTooltip, GainInputDisplayName }, 1.0f});
			FName PanInputName = PanInputBaseName;
			PanInputName.SetNumber(i);
#if WITH_EDITOR
			const FText PanInputDisplayName = FText::Format(LOCTEXT("Pan_Name", "Pan {0}"), i);
#else
			const FText PanInputDisplayName = FText::GetEmpty();
#endif

			InputInterface.Add(TInputDataVertex<float>{PanInputName, FDataVertexMetadata{ InputTooltip, PanInputDisplayName }, 0.0f});

		}

		//build the output interface, for now we only have two outputs as this is a stereo mixer
	 //we already have names just need the tooltip and the display name

#if WITH_EDITOR
		const FText OutputLeftDisplayName = LOCTEXT("OutL_Name", "Out L");
		const FText OutputRightDisplayName = LOCTEXT("OutR_Name", "Out R");
#else
		const FText OutputLeftDisplayName = FText::GetEmpty();
		const FText OutputRightDisplayName = FText::GetEmpty();
#endif

		OutputInterface.Add(TOutputDataVertex<FAudioBuffer>{OutputLeftBaseName, FDataVertexMetadata{ OutputTooltip, OutputLeftDisplayName }});
		OutputInterface.Add(TOutputDataVertex<FAudioBuffer>{OutputRightBaseName, FDataVertexMetadata{ OutputTooltip, OutputRightDisplayName }});



		return FVertexInterface
		{
			MoveTemp(InputInterface),
			MoveTemp(OutputInterface)
		};
	}

	class FConfigurableMixerOperatorData : public TOperatorData<FConfigurableMixerOperatorData>
	{
	public:
		static const FLazyName OperatorDataTypeName;

		FConfigurableMixerOperatorData(const int& InNumInputs, const bool& bInEqualPower)
			: NumInputs(InNumInputs),
			bEqualPower(bInEqualPower)
		{
		}

		// Returns the number of inputs.
		const int32& GetNumInputs() const
		{
			return NumInputs;
		}

		const bool& GetEqualPower() const
		{
			return bEqualPower;
		}

	private:
		int32 NumInputs;
		bool bEqualPower = false;

	};

	const FLazyName FConfigurableMixerOperatorData::OperatorDataTypeName = "ConfigurableMixerOperatorData";

	class FDAWConfigurableMixerNodeOperator : public TExecutableOperator<FDAWConfigurableMixerNodeOperator>
	{
	public:
		// Constructor: Set up input/output buffers.
		FDAWConfigurableMixerNodeOperator(const int& InNumInputs, const bool& bInEqualPower, const FBuildOperatorParams& InParams,
			const TArray<FAudioBufferReadRef>&& InInputBuffers,
			const TArray<FFloatReadRef>&& InGainValues,
			const TArray<FFloatReadRef>&& InPanValues)
			: Gains(InGainValues)
			, Pans(InPanValues)
			, Inputs(InInputBuffers)
			, bEqualPower(bInEqualPower)
		{
			// Create output write references.
			for (int32 i = 0; i < 2; ++i)
			{
				Outputs.Add(FAudioBufferWriteRef::CreateNew(InParams.OperatorSettings));
			}

			Reset(InParams);
		}

		virtual ~FDAWConfigurableMixerNodeOperator() = default;


		static FNodeClassMetadata GetNodeInfo()
		{
			using namespace ConfigurableMixerPrivate;
			return FNodeClassMetadata
			{

				FNodeClassName{ "unDAW", "ConfigurableStereoMixer", "" },
				0, // Major version
				1, // Minor version
				LOCTEXT("ConfigurableMixerNodeName", "Configurable Stereo Mixer"),
				LOCTEXT("ConfigurableMixerNodeDescription", "A node that mixes audio with configurable gain and pan controls."),
				TEXT("Amir Ben-Kiki"), // Author
				LOCTEXT("ConfigurableMixerPromptIfMissing", "Enable the unDAW Plugin"), // Prompt if missing
				ConfigurableMixerPrivate::GetVertexInterface(1),
				{}
			};
		
		}

		// Creates an operator instance from input data.
		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			const FInputVertexInterfaceData& InputData = InParams.InputData;

			int32 ConfiguredNumInputs = INDEX_NONE;
			if (const FConfigurableMixerOperatorData* ConfigData = CastOperatorData<const FConfigurableMixerOperatorData>(InParams.Node.GetOperatorData().Get()))
			{
				ConfiguredNumInputs = ConfigData->GetNumInputs();
			}

			bool bConfiguredEqualPower = false;
			if (const FConfigurableMixerOperatorData* ConfigData = CastOperatorData<const FConfigurableMixerOperatorData>(InParams.Node.GetOperatorData().Get()))
			{
				bConfiguredEqualPower = ConfigData->GetEqualPower();
			}


			TArray<FAudioBufferReadRef> InputBuffers;
			TArray<FFloatReadRef> InputGains;
			TArray<FFloatReadRef> InputPans;

			for (int32 i = 0; i < ConfiguredNumInputs; ++i)
			{

				FName LeftInputName = InputLeftBaseName;
				LeftInputName.SetNumber(i);
				InputBuffers.Add(InputData.GetOrCreateDefaultDataReadReference<FAudioBuffer>(LeftInputName, InParams.OperatorSettings));

				FName RightInputName = InputRightBaseName;
				RightInputName.SetNumber(i);
				InputBuffers.Add(InputData.GetOrCreateDefaultDataReadReference<FAudioBuffer>(RightInputName, InParams.OperatorSettings));

				//gain, pan
				FName GainInputName = GainInputBaseName;
				GainInputName.SetNumber(i);
				InputGains.Add(InputData.GetOrCreateDefaultDataReadReference<float>(GainInputName, InParams.OperatorSettings));

				FName PanInputName = PanInputBaseName;
				PanInputName.SetNumber(i);



				InputPans.Add(InputData.GetOrCreateDefaultDataReadReference<float>(PanInputName, InParams.OperatorSettings));
			}

			return MakeUnique<FDAWConfigurableMixerNodeOperator>(ConfiguredNumInputs, bConfiguredEqualPower, InParams, MoveTemp(InputBuffers), MoveTemp(InputGains), MoveTemp(InputPans));
		}

		// Bind input references to the vertex data.
		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override
		{
			for (uint32 i = 0; i < static_cast<uint32>(Gains.Num()); ++i)
			{
			
				FName LeftInputName = InputLeftBaseName;
				LeftInputName.SetNumber(i);
				
				InOutVertexData.BindReadVertex(LeftInputName, Inputs[i * 2]);

				//right
				FName RightInputName = InputRightBaseName;
				RightInputName.SetNumber(i);
				InOutVertexData.BindReadVertex(RightInputName, Inputs[i * 2 + 1]);

				//gain, pan

				FName GainInputName = GainInputBaseName;
				GainInputName.SetNumber(i);


				InOutVertexData.BindReadVertex(GainInputName, Gains[i]);

				FName PanInputName = PanInputBaseName;
				PanInputName.SetNumber(i);

				InOutVertexData.BindReadVertex(PanInputName, Pans[i]);
			}
		}

		// Bind output references to the vertex data.
		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override
		{
			InOutVertexData.BindReadVertex(OutputLeftBaseName, Outputs[0]);
			InOutVertexData.BindReadVertex(OutputRightBaseName, Outputs[1]);
		}

		// Resets operator state.
		void Reset(const IOperator::FResetParams& InParams)
		{
			Execute();
		}

		// Execute mixing.
		void Execute()
		{
			// Clear output buffers.
			for (uint32 chan = 0; chan < 2; ++chan)
			{
				TArrayView<float> OutputView = *Outputs[chan];
				FMemory::Memzero(OutputView.GetData(), OutputView.Num() * sizeof(float));
			}

			// Process each input.
			for (uint32 inputIndex = 0; inputIndex < static_cast<uint32>(Gains.Num()); ++inputIndex)
			{
				const float currentGain = *Gains[inputIndex];
				float leftPan = 0.f, rightPan = 0.f;
				ComputePanGains(*Pans[inputIndex], leftPan, rightPan);

				// Mix each channel.
				for (uint32 chanIndex = 0; chanIndex < 2; ++chanIndex)
				{
					float channelGain = currentGain;

						// Use updated pan logic.
						channelGain *= (chanIndex == 0) ? leftPan : rightPan;


					TArrayView<const float> InputView = *Inputs[inputIndex * 2 + chanIndex];
					TArrayView<float> OutputView = *Outputs[chanIndex];

					// Simple loop to mix input into output.
					for (int32 sample = 0; sample < OutputView.Num(); ++sample)
					{
						OutputView[sample] += InputView[sample] * channelGain;
					}
				}
			}
		}

	private:

		// Instance variables.
		TArray<FFloatReadRef> Gains;
		TArray<FFloatReadRef> Pans;
		TArray<FAudioBufferReadRef> Inputs;
		TArray<FAudioBufferWriteRef> Outputs;

		bool bEqualPower;

		// Updated ComputePanGains: For InPanAmount -1, fraction = 0 gives left gain = 1 and right gain = 0.
		void ComputePanGains(float InPanAmount, float& OutLeftGain, float& OutRightGain) const
		{
			float Fraction = 0.5f * (InPanAmount + 1.0f);
			if (bEqualPower)
			{
				// Equal-power panning: left = cos(angle), right = sin(angle) with angle = 0.5 * PI * Fraction.
				OutLeftGain = FMath::Cos(0.5f * PI * Fraction);
				OutRightGain = FMath::Sin(0.5f * PI * Fraction);
			}
			else
			{
				OutLeftGain = 1.0f - Fraction;
				OutRightGain = Fraction;
			}
		}

		static FNodeClassMetadata CreateNodeClassMetadata(const FName& InOperatorName,
			const FText& InDisplayName,
			const FText& InDescription,
			const FVertexInterface& InDefaultInterface)
		{
			FNodeClassMetadata Metadata{
				FNodeClassName{ "ConfigurableAudioMixer", InOperatorName, FName() },
				1, // Major Version
				0, // Minor Version
				InDisplayName,
				InDescription,
				PluginAuthor,
				PluginNodeMissingPrompt,
				InDefaultInterface,
				{ NodeCategories::Mix },
				{ INVTEXT("unDAW"), METASOUND_LOCTEXT("Metasound_AudioMixerKeyword", "Mixer") },
				FNodeDisplayStyle{}
			};

			return Metadata;
		}


}; // class TDAWConfigurableMixerNodeOperator

using FConfigurableMixerNode = TNodeFacade<FDAWConfigurableMixerNodeOperator>;

METASOUND_REGISTER_NODE_AND_CONFIGURATION(FConfigurableMixerNode, FConfigurableMixerConfiguration);

} // namespace Metasound


FConfigurableMixerConfiguration::FConfigurableMixerConfiguration()
	:NumInputs(4)
{
}

TInstancedStruct<FMetasoundFrontendClassInterface> FConfigurableMixerConfiguration::OverrideDefaultInterface(const FMetasoundFrontendClass& InNodeClass) const
{
	using namespace Metasound::ConfigurableMixerPrivate;
	
	return TInstancedStruct<FMetasoundFrontendClassInterface>::Make(FMetasoundFrontendClassInterface::GenerateClassInterface(GetVertexInterface(NumInputs)));
}

TSharedPtr<const Metasound::IOperatorData> FConfigurableMixerConfiguration::GetOperatorData() const
{
	return MakeShared<Metasound::ConfigurableMixerPrivate::FConfigurableMixerOperatorData>(NumInputs, bEqualPower);
}

#undef LOCTEXT_NAMESPACE