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


#define LOCTEXT_NAMESPACE "unDAWMetasounds_ConsoleMixerNode"

namespace Metasound
{
	// Template operator used for mixing audio with gain and pan controls.
	// This version removes debug logging and uses updated panning logic.
	template<uint32 NumInputs, uint32 NumChannels>
	class TDAWConsoleMixerNodeOperator : public TExecutableOperator<TDAWConsoleMixerNodeOperator<NumInputs, NumChannels>>
	{
	public:
		// Constructor: Set up input/output buffers.
		TDAWConsoleMixerNodeOperator(const FBuildOperatorParams& InParams,
			const TArray<FAudioBufferReadRef>&& InInputBuffers,
			const TArray<FFloatReadRef>&& InGainValues,
			const TArray<FFloatReadRef>&& InPanValues)
			: Gains(InGainValues)
			, Pans(InPanValues)
			, Inputs(InInputBuffers)
			, bEqualPower(false)
		{
			// Create output write references.
			for (uint32 i = 0; i < NumChannels; ++i)
			{
				Outputs.Add(FAudioBufferWriteRef::CreateNew(InParams.OperatorSettings));
			}

			Reset(InParams);
		}

		virtual ~TDAWConsoleMixerNodeOperator() = default;

		// Defines the default vertex interface for this node.
		static const FVertexInterface& GetDefaultInterface()
		{
			auto CreateDefaultInterface = []() -> FVertexInterface
				{
					// Build the input interface: audio channels for each input,
					// as well as gain and pan values.
					FInputVertexInterface InputInterface;
					for (uint32 InputIndex = 0; InputIndex < NumInputs; ++InputIndex)
					{
						// Audio channels.
						for (uint32 ChanIndex = 0; ChanIndex < NumChannels; ++ChanIndex)
						{
#if WITH_EDITOR
							const FDataVertexMetadata AudioInputMetadata{
								GetAudioInputDescription(InputIndex, ChanIndex),
								GetAudioInputDisplayName(InputIndex, ChanIndex)
							};
#else
							const FDataVertexMetadata AudioInputMetadata;
#endif // WITH_EDITOR
							InputInterface.Add(TInputDataVertex<FAudioBuffer>(GetAudioInputName(InputIndex, ChanIndex), AudioInputMetadata));
						}
#if WITH_EDITOR
						FDataVertexMetadata GainPinMetadata{
							GetGainInputDescription(InputIndex),
							GetGainInputDisplayName(InputIndex)
						};
#else
						FDataVertexMetadata GainPinMetadata;
#endif // WITH_EDITOR
						InputInterface.Add(TInputDataVertex<float>(GainInputNames[InputIndex], GainPinMetadata, 1.0f));
#if WITH_EDITOR
						FDataVertexMetadata PanPinMetadata{
							GetPanInputDescription(InputIndex),
							GetPanInputDisplayName(InputIndex)
						};
#else
						FDataVertexMetadata PanPinMetadata;
#endif // WITH_EDITOR
						InputInterface.Add(TInputDataVertex<float>(PanInputNames[InputIndex], PanPinMetadata, 0.0f));
					}

					// Build the output interface.
					FOutputVertexInterface OutputInterface;
					for (uint32 ChanIndex = 0; ChanIndex < NumChannels; ++ChanIndex)
					{
#if WITH_EDITOR
						const FDataVertexMetadata AudioOutputMetadata{
							GetAudioOutputDescription(ChanIndex),
							GetAudioOutputDisplayName(ChanIndex)
						};
#else
						const FDataVertexMetadata AudioOutputMetadata;
#endif // WITH_EDITOR
						OutputInterface.Add(TOutputDataVertex<FAudioBuffer>(AudioOutputNames[ChanIndex], AudioOutputMetadata));
					}

					return FVertexInterface(InputInterface, OutputInterface);
				};

			static const FVertexInterface DefaultInterface = CreateDefaultInterface();
			return DefaultInterface;
		}

		// Returns node class metadata based on channel count.
		static const FNodeClassMetadata& GetNodeInfo()
		{
			auto CreateNodeClassMetadataMono = []() -> FNodeClassMetadata
				{
					FName OperatorName = *FString::Printf(TEXT("Console Audio Mixer (Mono, %d)"), NumInputs);
					FText NodeDisplayName = METASOUND_LOCTEXT_FORMAT("MonoMixer", "Console Mono Mixer ({0})", NumInputs);
					const FText NodeDescription = METASOUND_LOCTEXT("MixerDescription1", "Scales and sums inputs with corresponding gain values.");
					FVertexInterface NodeInterface = GetDefaultInterface();

					return CreateNodeClassMetadata(OperatorName, NodeDisplayName, NodeDescription, NodeInterface);
				};

			auto CreateNodeClassMetadataStereo = []() -> FNodeClassMetadata
				{
					FName OperatorName = *FString::Printf(TEXT("Console Audio Mixer (Stereo, %d)"), NumInputs);
					FText NodeDisplayName = METASOUND_LOCTEXT_FORMAT("StereoMixer", "Console Stereo Mixer ({0})", NumInputs);
					const FText NodeDescription = METASOUND_LOCTEXT("MixerDescription2", "Scales and sums inputs with corresponding gain values.");
					FVertexInterface NodeInterface = GetDefaultInterface();

					return CreateNodeClassMetadata(OperatorName, NodeDisplayName, NodeDescription, NodeInterface);
				};

			auto CreateNodeClassMetadataMultiChan = []() -> FNodeClassMetadata
				{
					FName OperatorName = *FString::Printf(TEXT("Console Audio Mixer (%d-Channel, %d)"), NumChannels, NumInputs);
					FText NodeDisplayName = METASOUND_LOCTEXT_FORMAT("NChannelMixer", "Console {0}-Channel Mixer ({1})", NumChannels, NumInputs);
					const FText NodeDescription = METASOUND_LOCTEXT("MixerDescription3", "Scales and sums input audio by their corresponding gain values.");
					FVertexInterface NodeInterface = GetDefaultInterface();

					return CreateNodeClassMetadata(OperatorName, NodeDisplayName, NodeDescription, NodeInterface);
				};

			static const FNodeClassMetadata Metadata = (NumChannels == 1) ? CreateNodeClassMetadataMono() :
				(NumChannels == 2) ? CreateNodeClassMetadataStereo() : CreateNodeClassMetadataMultiChan();
			return Metadata;
		}

		// Creates an operator instance from input data.
		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			const FInputVertexInterfaceData& InputData = InParams.InputData;

			TArray<FAudioBufferReadRef> InputBuffers;
			TArray<FFloatReadRef> InputGains;
			TArray<FFloatReadRef> InputPans;

			for (uint32 i = 0; i < NumInputs; ++i)
			{
				for (uint32 chan = 0; chan < NumChannels; ++chan)
				{
					InputBuffers.Add(InputData.GetOrConstructDataReadReference<FAudioBuffer>(GetAudioInputName(i, chan), InParams.OperatorSettings));
				}

				InputGains.Add(InputData.GetOrCreateDefaultDataReadReference<float>(GainInputNames[i], InParams.OperatorSettings));
				InputPans.Add(InputData.GetOrCreateDefaultDataReadReference<float>(PanInputNames[i], InParams.OperatorSettings));
			}

			return MakeUnique<TDAWConsoleMixerNodeOperator<NumInputs, NumChannels>>(InParams, MoveTemp(InputBuffers), MoveTemp(InputGains), MoveTemp(InputPans));
		}

		// Bind input references to the vertex data.
		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override
		{
			for (uint32 i = 0; i < NumInputs; ++i)
			{
				for (uint32 chan = 0; chan < NumChannels; ++chan)
				{
					InOutVertexData.BindReadVertex(GetAudioInputName(i, chan), Inputs[i * NumChannels + chan]);
				}
				InOutVertexData.BindReadVertex(GainInputNames[i], Gains[i]);
				InOutVertexData.BindReadVertex(PanInputNames[i], Pans[i]);
			}
		}

		// Bind output references to the vertex data.
		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override
		{
			for (uint32 i = 0; i < NumChannels; ++i)
			{
				InOutVertexData.BindReadVertex(AudioOutputNames[i], Outputs[i]);
			}
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
			for (uint32 chan = 0; chan < NumChannels; ++chan)
			{
				TArrayView<float> OutputView = *Outputs[chan];
				FMemory::Memzero(OutputView.GetData(), OutputView.Num() * sizeof(float));
			}

			// Process each input.
			for (uint32 inputIndex = 0; inputIndex < NumInputs; ++inputIndex)
			{
				const float currentGain = *Gains[inputIndex];
				float leftPan = 0.f, rightPan = 0.f;
				ComputePanGains(*Pans[inputIndex], leftPan, rightPan);

				// Mix each channel.
				for (uint32 chanIndex = 0; chanIndex < NumChannels; ++chanIndex)
				{
					float channelGain = currentGain;
					if (NumChannels > 1)
					{
						// Use updated pan logic.
						channelGain *= (chanIndex == 0) ? leftPan : rightPan;
					}

					TArrayView<const float> InputView = *Inputs[inputIndex * NumChannels + chanIndex];
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
		// Helper functions to initialize vertex names.
		static TArray<FVertexName> InitializeAudioInputNames()
		{
			TArray<FVertexName> Names;
			Names.Reserve(NumInputs * NumChannels);
			for (uint32 inputIndex = 0; inputIndex < NumInputs; ++inputIndex)
			{
				for (uint32 channelIndex = 0; channelIndex < NumChannels; ++channelIndex)
				{
					FString Name = FString::Printf(TEXT("In %d"), inputIndex);
					if (NumChannels == 2)
					{
						Name += (channelIndex == 0) ? TEXT(" L") : TEXT(" R");
					}
					else if (NumChannels > 2)
					{
						Name += FString::Printf(TEXT(", %d"), channelIndex);
					}
					Names.Add(*Name);
				}
			}
			return Names;
		}

		static TArray<FVertexName> InitializeAudioOutputNames()
		{
			TArray<FVertexName> Names;
			Names.Reserve(NumChannels);
			for (uint32 chanIndex = 0; chanIndex < NumChannels; ++chanIndex)
			{
				FString Name = TEXT("Out");
				if (NumChannels == 2)
				{
					Name += (chanIndex == 0) ? TEXT(" L") : TEXT(" R");
				}
				else if (NumChannels > 2)
				{
					Name += FString::Printf(TEXT(" %d"), chanIndex);
				}
				Names.Add(*Name);
			}
			return Names;
		}

		static TArray<FVertexName> InitializeGainInputNames()
		{
			TArray<FVertexName> Names;
			Names.Reserve(NumInputs);
			for (uint32 i = 0; i < NumInputs; ++i)
			{
				Names.Add(*FString::Printf(TEXT("Gain %d"), i));
			}
			return Names;
		}

		static TArray<FVertexName> InitializePanInputNames()
		{
			TArray<FVertexName> Names;
			Names.Reserve(NumInputs);
			for (uint32 i = 0; i < NumInputs; ++i)
			{
				Names.Add(*FString::Printf(TEXT("Pan Amount %d"), i));
			}
			return Names;
		}

		static const FVertexName& GetAudioInputName(uint32 inputIndex, uint32 channelIndex)
		{
			return AudioInputNames[inputIndex * NumChannels + channelIndex];
		}

		// Static arrays to hold vertex names.
		static inline const TArray<FVertexName> AudioInputNames = InitializeAudioInputNames();
		static inline const TArray<FVertexName> AudioOutputNames = InitializeAudioOutputNames();
		static inline const TArray<FVertexName> GainInputNames = InitializeGainInputNames();
		static inline const TArray<FVertexName> PanInputNames = InitializePanInputNames();

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
				FNodeClassName{ "ConsoleAudioMixer", InOperatorName, FName() },
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

#if WITH_EDITOR
		static const FText GetAudioInputDescription(uint32 InputIndex, uint32 ChannelIndex)
		{
			return METASOUND_LOCTEXT_FORMAT("AudioMixerAudioInputDescription", "Audio Input #: {0}, Channel: {1}", InputIndex, ChannelIndex);
		}

		static const FText GetAudioInputDisplayName(uint32 InputIndex, uint32 ChannelIndex)
		{
			if (NumChannels == 1)
			{
				return METASOUND_LOCTEXT_FORMAT("AudioMixerAudioInput1In", "In {0}", InputIndex);
			}
			else if (NumChannels == 2)
			{
				return (ChannelIndex == 0)
					? METASOUND_LOCTEXT_FORMAT("AudioMixerAudioInput2InL", "In {0} L", InputIndex)
					: METASOUND_LOCTEXT_FORMAT("AudioMixerAudioInput2InR", "In {0} R", InputIndex);
			}
			return METASOUND_LOCTEXT_FORMAT("AudioMixerAudioInputIn", "In {0}, {1}", InputIndex, ChannelIndex);
		}

		static const FText GetGainInputDisplayName(uint32 InputIndex)
		{
			return METASOUND_LOCTEXT_FORMAT("AudioMixerGainInputDisplayName", "Gain {0} (Lin)", InputIndex);
		}

		static const FText GetGainInputDescription(uint32 InputIndex)
		{
			return METASOUND_LOCTEXT_FORMAT("AudioMixerGainInputDescription", "Gain Input #: {0}", InputIndex);
		}

		static const FText GetPanInputDisplayName(uint32 InputIndex)
		{
			return METASOUND_LOCTEXT_FORMAT("AudioMixerPanInputDisplayName", "Pan Amount {0}", InputIndex);
		}

		static const FText GetPanInputDescription(uint32 InputIndex)
		{
			return METASOUND_LOCTEXT_FORMAT("AudioMixerPanInputDescription", "Pan Amount Input #: {0}", InputIndex);
		}

		static const FText GetAudioOutputDisplayName(uint32 ChannelIndex)
		{
			if (NumChannels == 1)
			{
				return METASOUND_LOCTEXT("AudioMixerAudioOutput1Out", "Out");
			}
			else if (NumChannels == 2)
			{
				return (ChannelIndex == 0)
					? METASOUND_LOCTEXT("AudioMixerAudioOutput2OutL", "Out L")
					: METASOUND_LOCTEXT("AudioMixerAudioOutput2OutR", "Out R");
			}
			return METASOUND_LOCTEXT_FORMAT("AudioMixerAudioOutputOut", "Out {0}", ChannelIndex);
		}

		static const FText GetAudioOutputDescription(uint32 ChannelIndex)
		{
			return METASOUND_LOCTEXT_FORMAT("AudioMixerAudioOutputDescription", "Summed output for channel: {0}", ChannelIndex);
		}
#endif // WITH_EDITOR
	}; // class TDAWConsoleMixerNodeOperator

	// Facade class used by the Metasound Frontend.
	template<uint32 NumInputs, uint32 NumChannels>
	class TDAWConsoleMixerNode : public FNodeFacade
	{
	public:
		TDAWConsoleMixerNode(const FNodeInitData& InInitData)
			: FNodeFacade(InInitData.InstanceName,
				InInitData.InstanceID,
				TFacadeOperatorClass<TDAWConsoleMixerNodeOperator<NumInputs, NumChannels>>())
		{
		}

		virtual ~TDAWConsoleMixerNode() = default;
	};

	// Macro to register the node for different input/channel configurations.
#define REGISTER_AUDIOMIXER_NODE(A, B) \
		using FAudioMixerNode_##A##_##B = TDAWConsoleMixerNode<A, B>; \
		METASOUND_REGISTER_NODE(FAudioMixerNode_##A##_##B)

	// Register some common variants.
	// Stereo variants:
	REGISTER_AUDIOMIXER_NODE(2, 2)
		//REGISTER_AUDIOMIXER_NODE(3, 2)
		REGISTER_AUDIOMIXER_NODE(4, 2)
		//REGISTER_AUDIOMIXER_NODE(5, 2)
		//REGISTER_AUDIOMIXER_NODE(6, 2)
		//REGISTER_AUDIOMIXER_NODE(8, 2)
		REGISTER_AUDIOMIXER_NODE(8, 2)
		REGISTER_AUDIOMIXER_NODE(16, 2)

		// You can add similar registrations for mono or multichannel variants as needed.
} // namespace Metasound

#undef LOCTEXT_NAMESPACE

