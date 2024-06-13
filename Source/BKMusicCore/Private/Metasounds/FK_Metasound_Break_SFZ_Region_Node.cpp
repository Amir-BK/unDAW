// Fill out your copyright notice in the Description page of Project Settings.


#include "Metasounds/FK_Metasound_Break_SFZ_Region_Node.h"
#include "MetasoundParamHelper.h"
#include "SFZ/UnDAWSFZAsset.h"

#define LOCTEXT_NAMESPACE "SFZMetaSound_BreakRegionNode"
#define TOKENIZE(x) #x
#define USE_AMPEG_VALUE_IF_SET(Name, Param) if(FloatsMap[Name]) {* TOKENIZE(Param) = FTime::FromSeconds(FloatsMap[Name]);};

namespace Metasound
{


	namespace FKMetasoundSFZRegionBreakDataNodes
	{
		//Input parameters
		METASOUND_PARAM(InParamNameSFZRegionProxy, "SFZ Region", "The SFZ region, representing a single wav file with parameters ")

		//Output Sample parameters
		METASOUND_PARAM(OutParamNameWaveAsset, "Sample Out ", "The Wav Asset for this region ")
		METASOUND_PARAM(OutParamNamePitchCorrection, "Pitch Correction", "Pitch correction for the sample in semi tones. Includes microtuning as defined by the SFZ instrument.")
		METASOUND_PARAM(OutParamNameOffset, "Start Time", "How far into the sample should playback begin, connect into the 'Start Time' pin on the wav player")
		METASOUND_PARAM(OutParamNameDelay, "Delay", "The amount of time playback of this sample should be delayed by, may include randomness")
		METASOUND_PARAM(OutParamNameLoopStart, "Loop Start", "If looping, when into the sample should the loop start.")
		METASOUND_PARAM(OutParamNameLoopDuration, "Loop Duration", "The duration of the loop")
		METASOUND_PARAM(OutParamNameLoop, "Loop", "should this sample loop")
		METASOUND_PARAM(OutParamNameOneShot, "One Shot", "One Shot samples are expected to play to completion ignoring 'Note Off' Events")

		//outputs Ampeg Parameters 
		METASOUND_PARAM(OutParamNameAmpegDelay, "Ampeg Delay ", "The Ampeg Delay")
		METASOUND_PARAM(OutParamNameAmpegAttack, "Ampeg Attack ", "The Ampeg Delay")
		METASOUND_PARAM(OutParamNameAmpegHold, "Ampeg Hold ", "The Ampeg Delay")
		METASOUND_PARAM(OutParamNameAmpegDecay, "Ampeg Decay ", "The Ampeg Delay")
		METASOUND_PARAM(OutParamNameAmpegSustain, "Ampeg Sustain ", "The Ampeg Delay")
		METASOUND_PARAM(OutParamNameAmpegRelease, "Ampeg Release ", "The Ampeg Delay")
		METASOUND_PARAM(OutParamNameAmpegStart, "Ampeg Start ", "The Ampeg Delay")

		//outputs Filter Parameters 
		METASOUND_PARAM(OutParamNameCutoff1, "Filter Cutoff ", "The Ampeg Delay")
		METASOUND_PARAM(OutParamNameCutoff2, "Filter2 Cutoff", "The Ampeg Delay")

		//General (ampeg?)
		METASOUND_PARAM(OutParamNameAmplitude, "Amplitude", "The Ampeg Delay")


			
	}


	//vertex interface for sample data node 
	const FVertexInterface& F_FK_SFZ_Metasound_Sample_Break_Data_Operator::GetVertexInterface()
	{
	
		using namespace FKMetasoundSFZRegionBreakDataNodes;

		static const FVertexInterface Interface(
			FInputVertexInterface(
				TInputDataVertex<F_FK_SFZ_Region_Data>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameSFZRegionProxy))
			),

			FOutputVertexInterface(
				TOutputDataVertex<FWaveAsset>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameWaveAsset)),
				TOutputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNamePitchCorrection)),
				TOutputDataVertex<FTime>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameOffset)),
				TOutputDataVertex<FTime>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameDelay)),
				TOutputDataVertex<bool>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameLoop)),
				TOutputDataVertex<FTime>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameLoopStart)),
				TOutputDataVertex<FTime>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameLoopDuration)),
				TOutputDataVertex<bool>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameOneShot))
			)
		);
		return Interface;
	}

		//vertex interface for ampeg data node 
	const FVertexInterface& F_FK_SFZ_Metasound_AMPEG_Break_Data_Operator::GetVertexInterface()
	{
		using namespace FKMetasoundSFZRegionBreakDataNodes;

		static const FVertexInterface Interface(
			FInputVertexInterface(
				TInputDataVertex<F_FK_SFZ_Region_Data>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameSFZRegionProxy))
			),

			FOutputVertexInterface(
				TOutputDataVertex<FTime>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameAmpegDelay)),
				TOutputDataVertex<FTime>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameAmpegAttack)),
				TOutputDataVertex<FTime>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameAmpegHold)),
				TOutputDataVertex<FTime>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameAmpegDecay)),
				TOutputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameAmpegSustain)),
				TOutputDataVertex<FTime>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameAmpegRelease)),
				TOutputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameAmpegStart)),
				TOutputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameAmplitude))
			)
		);
		return Interface;
	}

	TUniquePtr<IOperator> F_FK_SFZ_Metasound_Sample_Break_Data_Operator::CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
	{
		using namespace FKMetasoundSFZRegionBreakDataNodes;
		
		const FInputVertexInterfaceData& InputCollection = InParams.InputData;
		const FInputVertexInterface& InputInterface = GetVertexInterface().GetInputInterface();
		

		F_FK_SFZ_Region_DataReadRef SFZ_FK_Region_Proxy = InputCollection.GetDataReadReference<F_FK_SFZ_Region_Data>(METASOUND_GET_PARAM_NAME(InParamNameSFZRegionProxy));
		
		return MakeUnique<F_FK_SFZ_Metasound_Sample_Break_Data_Operator>(InParams.OperatorSettings, SFZ_FK_Region_Proxy);
	}

	TUniquePtr<IOperator> F_FK_SFZ_Metasound_AMPEG_Break_Data_Operator::CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
	{
		using namespace FKMetasoundSFZRegionBreakDataNodes;
		
		const FInputVertexInterfaceData& InputCollection = InParams.InputData;
		const FInputVertexInterface& InputInterface = GetVertexInterface().GetInputInterface();
		

		F_FK_SFZ_Region_DataReadRef SFZ_FK_Region_Proxy = InputCollection.GetDataReadReference<F_FK_SFZ_Region_Data>(METASOUND_GET_PARAM_NAME(InParamNameSFZRegionProxy));
		
		return MakeUnique<F_FK_SFZ_Metasound_AMPEG_Break_Data_Operator>(InParams.OperatorSettings, SFZ_FK_Region_Proxy);
	}

	F_FK_SFZ_Metasound_Sample_Break_Data_Operator::F_FK_SFZ_Metasound_Sample_Break_Data_Operator(const FOperatorSettings& InSettings, F_FK_SFZ_Region_DataReadRef& in_SFZ_layer_readRef)
		:Settings(InSettings),
		SFZ_Region_Proxy(in_SFZ_layer_readRef),
		WaveAsset(FWaveAssetWriteRef::CreateNew(nullptr)),
		PitchCorrection(FFloatWriteRef::CreateNew(0)),
		Offset(FTimeWriteRef::CreateNew(0.0f)),
		Delay(FTimeWriteRef::CreateNew(0.0f)),
		LoopStart(FTimeWriteRef::CreateNew(0.0f)),
		LoopDuration(FTimeWriteRef::CreateNew(-1.0f)),
		Loop(FBoolWriteRef::CreateNew(false)),
		bOneShot(FBoolWriteRef::CreateNew(false))
	{}

	F_FK_SFZ_Metasound_AMPEG_Break_Data_Operator::F_FK_SFZ_Metasound_AMPEG_Break_Data_Operator(
	const FOperatorSettings& InSettings, F_FK_SFZ_Region_DataReadRef& in_SFZ_layer_readRef)
	:Settings(InSettings),
	SFZ_Region_Proxy(in_SFZ_layer_readRef),
	AmpegDelay(FTimeWriteRef::CreateNew(0.0f)),
	AmpegAttack(FTimeWriteRef::CreateNew(0.01f)),
	AmpegHold(FTimeWriteRef::CreateNew(0.0f)),
	AmpegDecay(FTimeWriteRef::CreateNew(0.02f)),
	AmpegRelease(FTimeWriteRef::CreateNew(0.03f)),
	AmpegStart(FFloatWriteRef::CreateNew(0)),
	Amplitude(FFloatWriteRef::CreateNew(1.0f)),
	AmpegSustain(FFloatWriteRef::CreateNew(1.0f))
	{}

	void F_FK_SFZ_Metasound_AMPEG_Break_Data_Operator::BindInputs(FInputVertexInterfaceData& InOutVertexData)
	{
		using namespace FKMetasoundSFZRegionBreakDataNodes;

		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InParamNameSFZRegionProxy), SFZ_Region_Proxy);
	}

	void F_FK_SFZ_Metasound_Sample_Break_Data_Operator::BindInputs(FInputVertexInterfaceData& InOutVertexData)
	{
		using namespace FKMetasoundSFZRegionBreakDataNodes;

		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InParamNameSFZRegionProxy), SFZ_Region_Proxy);
	}

	void F_FK_SFZ_Metasound_Sample_Break_Data_Operator::BindOutputs(FOutputVertexInterfaceData& InOutVertexData)
	{
		using namespace FKMetasoundSFZRegionBreakDataNodes;

		InOutVertexData.BindWriteVertex(METASOUND_GET_PARAM_NAME(OutParamNameWaveAsset), WaveAsset);
		InOutVertexData.BindWriteVertex(METASOUND_GET_PARAM_NAME(OutParamNameLoop), Loop);
		InOutVertexData.BindWriteVertex(METASOUND_GET_PARAM_NAME(OutParamNameOffset), Offset);
		InOutVertexData.BindWriteVertex(METASOUND_GET_PARAM_NAME(OutParamNameLoopStart), LoopStart);
		InOutVertexData.BindWriteVertex(METASOUND_GET_PARAM_NAME(OutParamNameLoopDuration), LoopDuration);
		InOutVertexData.BindWriteVertex(METASOUND_GET_PARAM_NAME(OutParamNameDelay), Delay);
		InOutVertexData.BindWriteVertex(METASOUND_GET_PARAM_NAME(OutParamNameOneShot), bOneShot);

		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutParamNamePitchCorrection), PitchCorrection);
	}

	void F_FK_SFZ_Metasound_AMPEG_Break_Data_Operator::BindOutputs(FOutputVertexInterfaceData& InOutVertexData)
	{
		using namespace FKMetasoundSFZRegionBreakDataNodes;

		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutParamNameAmpegDelay), AmpegDelay);
		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutParamNameAmpegAttack), AmpegAttack);
		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutParamNameAmpegHold), AmpegHold);
		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutParamNameAmpegDecay), AmpegDecay);
		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutParamNameAmpegSustain), AmpegSustain);
		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutParamNameAmpegRelease), AmpegRelease);
		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutParamNameAmpegStart), AmpegStart);
		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutParamNameAmplitude), Amplitude);

	}

	void F_FK_SFZ_Metasound_AMPEG_Break_Data_Operator::Execute()
	{
		{
			if(!SFZ_Region_Proxy->IsInitialized()) return;
			//auto& sample =  SFZ_Region_Proxy->GetProxy()->GetSoundwaveData()->g;
			if(!bSampleInitialized)
			{
				const auto FloatsMap = SFZ_Region_Proxy->GetProxy()->GetRegionPointer()->SFZFloatParamsArray;


				if(FloatsMap.Contains("Ampeg_Delay")) {* AmpegDelay = FTime::FromSeconds(FloatsMap["Ampeg_Delay"]);}
				if(FloatsMap.Contains("Ampeg_Attack")) {* AmpegAttack = FTime::FromSeconds(FloatsMap["Ampeg_Attack"]);}
				if(FloatsMap.Contains("Ampeg_Decay")) {* AmpegDecay = FTime::FromSeconds(FloatsMap["Ampeg_Decay"]);}
				if(FloatsMap.Contains("Ampeg_Hold")) {* AmpegHold = FTime::FromSeconds(FloatsMap["Ampeg_Hold"]);}
				if(FloatsMap.Contains("Ampeg_Release")) {* AmpegRelease = FTime::FromSeconds(FloatsMap["Ampeg_Release"]);}
				if(FloatsMap.Contains("Ampeg_Sustain")) {* AmpegSustain = FloatsMap["Ampeg_Sustain"] * 0.01;}
				
				if(FloatsMap.Contains("Ampeg_Start")) *AmpegStart = FloatsMap["Ampeg_Attack"];
				if(FloatsMap.Contains("Amplitude")) *Amplitude = FloatsMap["Amplitude"] * 0.01; else *Amplitude = 1.0f;
	

				bSampleInitialized = true;
			}
		}
	}


	//this is the actual Execution node, where we break the struct and put it in the nodes 
	void F_FK_SFZ_Metasound_Sample_Break_Data_Operator::Execute()
	{
		if(!SFZ_Region_Proxy->IsInitialized()) return;
		//auto& sample =  SFZ_Region_Proxy->GetProxy()->GetSoundwaveData()->g;
		if(!bSampleInitialized)
		{
			const auto PerfData = SFZ_Region_Proxy->GetProxy()->GetPerformanceData();
		
			const auto SoundData = SFZ_Region_Proxy->GetProxy()->GetSoundwaveData();
			if(SoundData != nullptr)
			{
				const auto WaveProxy = FWaveAsset(SoundData->CreateSoundWaveProxy()) ;//FWaveAsset(->CreateSoundWaveProxy());
				if (WaveProxy.IsSoundWaveValid()) *WaveAsset = WaveProxy;
			}

			*PitchCorrection = PerfData.Tune;
			*Loop = (PerfData.LoopMode == Loop_Continuous || PerfData.LoopMode == Loop_Sustain);
			*Offset = FTime::FromSeconds(PerfData.Offset);
			*LoopDuration = FTime::FromSeconds(PerfData.Loop_Duration);
			*LoopStart = FTime::FromSeconds(PerfData.Loop_Start);
			*bOneShot = PerfData.bOneShot;
			*Delay = FTime::FromSeconds(PerfData.Delay);
			bSampleInitialized = true;

			//new implementation without using intermediary struct
			const auto TimedFloatsMap = SFZ_Region_Proxy->GetProxy()->GetRegionPointer()->SFZNormalizedTimedParamsArray;
			if(TimedFloatsMap.Contains("Offset")) {* Offset = FTime::FromSeconds(TimedFloatsMap["Offset"]);}
		}

		}

	//node facade info
	const FNodeClassMetadata& Metasound::F_FK_SFZ_Metasound_Sample_Break_Data_Operator::GetNodeInfo()
	{
		auto InitNodeInfo = []() -> FNodeClassMetadata
		{
			FNodeClassMetadata Info;

			Info.ClassName = { TEXT("FK"), TEXT("SFZ"), TEXT("Sample") };
			Info.MajorVersion = 0;
			Info.MinorVersion = 2;
			Info.DisplayName = INVTEXT("FK SFZ Break Region Data");
			Info.Description = LOCTEXT("FK_SFZ_Description", "Breaks region data to usable metasound datatypes");
			Info.Author = FString(TEXT("FlyKick Studios"));
			Info.PromptIfMissing = FText::FromString(TEXT("FlyKick Studios SFZ Failed To Load SFZ Sample Loader"));
			Info.DefaultInterface = GetVertexInterface();
			Info.CategoryHierarchy = { INVTEXT("SFZ"), INVTEXT("Sampler") };

			return Info;
		};

		static const FNodeClassMetadata Info = InitNodeInfo();

		return Info;
	}

	const FNodeClassMetadata& F_FK_SFZ_Metasound_AMPEG_Break_Data_Operator::GetNodeInfo()
	{
		auto InitNodeInfo = []() -> FNodeClassMetadata
		{
			FNodeClassMetadata Info;

			Info.ClassName = { TEXT("FK"), TEXT("SFZ"), TEXT("AMPEG Data") };
			Info.MajorVersion = 0;
			Info.MinorVersion = 2;
			Info.DisplayName = INVTEXT("FK SFZ Break Region Ampeg Data");
			Info.Description = INVTEXT("Breaks region AMPEG Envelope data to usable metasound datatypes");
			Info.Author = FString(TEXT("FlyKick Studios"));
			Info.PromptIfMissing = FText::FromString(TEXT("FlyKick Studios SFZ Failed To Load SFZ AMPEG Node"));
			Info.DefaultInterface = GetVertexInterface();
			Info.CategoryHierarchy = { INVTEXT("SFZ"), INVTEXT("Sampler") };

			return Info;
		};

		static const FNodeClassMetadata Info = InitNodeInfo();

		return Info;
	}



	METASOUND_REGISTER_NODE(FFK_SFZ_Break_Sample_Data_Node)
	METASOUND_REGISTER_NODE(FFK_SFZ_Break_Ampeg_Data_Node)
}

	


#undef USE_AMPEG_VALUE_IF_SET
#undef LOCTEXT_NAMESPACE