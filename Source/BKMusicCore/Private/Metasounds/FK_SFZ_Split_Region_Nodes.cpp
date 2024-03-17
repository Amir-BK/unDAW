//// Copyright Epic Games, Inc. All Rights Reserved.
//
//#include "Internationalization/Text.h"
//#include "MetasoundExecutableOperator.h"
//#include "MetasoundFacade.h"
//#include "MetasoundNodeRegistrationMacro.h"
//#include "MetasoundStandardNodesCategories.h"
//#include "MetasoundStandardNodesNames.h"
//#include "MetasoundParamHelper.h"
//#include "MetasoundTrigger.h"
//#include "MetasoundVertex.h"
//#include "Metasounds/FKSFZAudioParameterInterfaces.h"
//
//#define LOCTEXT_NAMESPACE "FlyKickSFZNodes_SplitRegionData"
//
//#define REGISTER_SPLIT_SFZ_REGION_NODE(Number, DataCategory) \
//	using FSplitSFZRegionNode_##Number = TSplitSFZRegionNode<Number, DataCategory>; \
//	METASOUND_REGISTER_NODE(FSplitSFZRegionNode_##Number) \
//
//
//namespace Metasound
//{
//	namespace MetasoundSplitSFZRegionNodePrivate
//	{
//
//		
//		BKMUSICCORE_API FNodeClassMetadata CreateNodeClassMetadata(const FName& InOperatorName, const FText& InDisplayName, const FText& InDescription, const FVertexInterface& InDefaultInterface)
//		{
//			FNodeClassMetadata Metadata
//			{
//				FNodeClassName { "SFZ Split Region", InOperatorName, FName() },
//				0, // Major Version
//				1, // Minor Version
//				InDisplayName,
//				InDescription,
//				TEXT("FlyKick Studios"),
//				PluginNodeMissingPrompt,
//				InDefaultInterface,
//				{ INVTEXT("SFZ") },
//				{ },
//				FNodeDisplayStyle{}
//			};
//
//			return Metadata;
//		}
//	}
//
//	namespace SplitRegionDataVertexNames
//	{
//		//Input parameters
//		METASOUND_PARAM(InParamNameSFZRegionProxy, "SFZ Region", "The SFZ region, representing a single wav file with parameters ")
//
//
//		METASOUND_PARAM(OutputTrigger, "Out", "Triggered when any of the input triggers have been triggered. ")
//
//		BKMUSICCORE_API const FVertexName GetInputTriggerName(uint32 InIndex)
//		{
//			return *FString::Format(TEXT("In {0}"), { InIndex });
//		}
//
//		BKMUSICCORE_API const FText GetInputTriggerDescription(uint32 InIndex)
//		{
//			return METASOUND_LOCTEXT_FORMAT("SplitRegionDataInputTriggerDesc", "Trigger {0} input. The output trigger is hit when any of the input triggers are hit.", InIndex);
//		}
//
//		BKMUSICCORE_API const FText GetInputTriggerDisplayName(uint32 InIndex)
//		{
//			return METASOUND_LOCTEXT_FORMAT("SplitRegionDataInputTriggerDisplayName", "In {0}", InIndex);
//		}
//	}
//	namespace SFZMetaSoundsPrivate
//	{
//		class SFZ_Data_Specialization_Type
//		{
//		public:
//			static FString SpecializationName;
//
//			static FString GetSpecializationName() {return FString("No Specialization");};
//
//		};
//
//	
//		class SampleDataSpecialization: public SFZ_Data_Specialization_Type
//		{
//		public:
//
//			static FString GetSpecializationName() {return FString("SFZ Region Sample Data Break Node");};
//		};
//
//		class EnvelopeDataSpecialization: public SFZ_Data_Specialization_Type
//		{
//		public:
//
//			static FString GetSpecializationName() {return FString("SFZ Region Envelope Data Break Node");};
//		};
//	}
//	
//
//	template<uint32 NumInputs, typename SpecType>
//	class TSplitRegionDataOperator : public TExecutableOperator<TSplitRegionDataOperator<NumInputs, SpecType>>
//	{
//	public:
//		static const FVertexInterface& GetDefaultInterface()
//		{
//			using namespace SplitRegionDataVertexNames;
//
//			auto CreateDefaultInterface = []() -> FVertexInterface
//			{
//				FInputVertexInterface InputInterface;
//
//				for (uint32 i = 0; i < NumInputs; ++i)
//				{
//					const FDataVertexMetadata InputTriggerMetadata
//					{
//						  GetInputTriggerDescription(i) // description
//						, GetInputTriggerDisplayName(i) // display name
//					};
//
//					InputInterface.Add(TInputDataVertex<FTrigger>(GetInputTriggerName(i), InputTriggerMetadata));
//				}
//
//				FOutputVertexInterface OutputInterface;
//				OutputInterface.Add(TOutputDataVertex<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutputTrigger)));
//
//				return FVertexInterface(InputInterface, OutputInterface);
//			};
//
//			static const FVertexInterface DefaultInterface = CreateDefaultInterface();
//			return DefaultInterface;
//		}
//
//		static const FNodeClassMetadata& GetNodeInfo()
//		{
//			SpecType::GetSpecializationName();
//
//			auto CreateNodeClassMetadata = []() -> FNodeClassMetadata
//			{
//				FName OperatorName = *FString::Printf(TEXT("SFZ Split Region Data (%d)"), NumInputs);
//				FText NodeDisplayName = FText::FromString(SpecType::GetSpecializationName());//SFZMetaSoundsPrivate::SFZ_Data_Specialization_Type<SpecType>::GetSpecializationName());
//				const FText NodeDescription = METASOUND_LOCTEXT("SplitRegionDataDescription", "Will trigger output on any of the input triggers.");
//				FVertexInterface NodeInterface = GetDefaultInterface();
//
//				return MetasoundSplitSFZRegionNodePrivate::CreateNodeClassMetadata(OperatorName, NodeDisplayName, NodeDescription, NodeInterface);
//			};
//
//			static const FNodeClassMetadata Metadata = CreateNodeClassMetadata();
//			return Metadata;
//		}
//
//		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, TArray<TUniquePtr<IOperatorBuildError>>& OutErrors)
//		{
//			using namespace SplitRegionDataVertexNames;
//
//			const FInputVertexInterface& InputInterface = InParams.Node.GetVertexInterface().GetInputInterface();
//			const FInputVertexInterfaceData& InputCollection = InParams.InputData;
//
//			TArray<FTriggerReadRef> InputTriggers;
//
//			for (uint32 i = 0; i < NumInputs; ++i)
//			{
//				InputTriggers.Add(InputCollection.GetOrConstructDataReadReference<FTrigger>(GetInputTriggerName(i), InParams.OperatorSettings));
//			}
//
//			F_FK_SFZ_Region_DataReadRef SFZ_FK_Region_Proxy = InputCollection.GetOrConstructDataReadReference<F_FK_SFZ_Region_Data>(METASOUND_GET_PARAM_NAME(InParamNameSFZRegionProxy), InParams.OperatorSettings);
//
//
//			return MakeUnique<TSplitRegionDataOperator<NumInputs, SpecType>>(InParams.OperatorSettings, MoveTemp(InputTriggers), SFZ_FK_Region_Proxy);
//		}
//
//		TSplitRegionDataOperator(const FOperatorSettings& InSettings, const TArray<FTriggerReadRef>&& InInputTriggers, F_FK_SFZ_Region_DataReadRef& in_SFZ_layer_readRef)
//			: InputTriggers(InInputTriggers)
//			, OutputTrigger(FTriggerWriteRef::CreateNew(InSettings)),
//				Settings(InSettings),
//				SFZ_Region_Proxy(in_SFZ_layer_readRef)
//		{
//		}
//
//		virtual ~TSplitRegionDataOperator() = default;
//
//
//		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override
//		{
//			using namespace SplitRegionDataVertexNames;
//			for (uint32 i = 0; i < NumInputs; ++i)
//			{
//				InOutVertexData.BindReadVertex(GetInputTriggerName(i), InputTriggers[i]);
//			}
//		}
//
//		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override
//		{
//			using namespace SplitRegionDataVertexNames;
//			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutputTrigger), OutputTrigger);
//		}
//
//		virtual FInputVertexInterfaceData GetInputs() const override
//		{
//			// This should never be called. Bind(...) is called instead. This method
//			// exists as a stop-gap until the API can be deprecated and removed.
//			checkNoEntry();
//			return {};
//		}
//
//		virtual FInputVertexInterfaceData GetOutputs() const override
//		{
//			// This should never be called. Bind(...) is called instead. This method
//			// exists as a stop-gap until the API can be deprecated and removed.
//			checkNoEntry();
//			return {};
//		}
//
//		void Execute()
//		{
//			OutputTrigger->AdvanceBlock();
//
//			for (uint32 i = 0; i < NumInputs; ++i)
//			{
//				InputTriggers[i]->ExecuteBlock(
//					[&](int32 StartFrame, int32 EndFrame)
//					{
//					},
//					[this, i](int32 StartFrame, int32 EndFrame)
//					{
//						OutputTrigger->TriggerFrame(StartFrame);
//					}
//					);
//			}
//		}
//
//		void Reset(const IOperator::FResetParams& InParams)
//		{
//			OutputTrigger->Reset();
//		}
//
//	private:
//
//		TArray<FTriggerReadRef> InputTriggers;
//		FTriggerWriteRef OutputTrigger;
//
//		//inputs
//		const FOperatorSettings Settings;
//		F_FK_SFZ_Region_DataReadRef SFZ_Region_Proxy;
//	};
//
//	/** TSplitSFZRegionNode
//	*
//	*  Will output a trigger whenever any of its input triggers are set.
//	*/
//	template<uint32 NumInputs, typename DataType>
//	class BKMUSICCORE_API TSplitSFZRegionNode : public FNodeFacade
//	{
//	public:
//		/**
//		 * Constructor used by the Metasound Frontend.
//		 */
//		TSplitSFZRegionNode(const FNodeInitData& InInitData)
//			: FNodeFacade(InInitData.InstanceName, InInitData.InstanceID, TFacadeOperatorClass<TSplitRegionDataOperator<NumInputs, DataType>>())
//		{}
//
//		virtual ~TSplitSFZRegionNode() = default;
//	};
//
//	REGISTER_SPLIT_SFZ_REGION_NODE(1, SFZMetaSoundsPrivate::SampleDataSpecialization)
//	REGISTER_SPLIT_SFZ_REGION_NODE(2, SFZMetaSoundsPrivate::EnvelopeDataSpecialization)
//	//REGISTER_SPLIT_SFZ_REGION_NODE(2, Envelope)
//
//	
//	/*REGISTER_SPLIT_SFZ_REGION_NODE(4)
//	REGISTER_SPLIT_SFZ_REGION_NODE(5)
//	REGISTER_SPLIT_SFZ_REGION_NODE(6)
//	REGISTER_SPLIT_SFZ_REGION_NODE(7)
//	REGISTER_SPLIT_SFZ_REGION_NODE(8)*/
//}
//
//#undef LOCTEXT_NAMESPACE
