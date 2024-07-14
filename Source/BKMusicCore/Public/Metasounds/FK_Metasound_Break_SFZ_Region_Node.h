// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MetasoundEnumRegistrationMacro.h"
#include "MetasoundParamHelper.h"
#include "MetasoundWave.h"
#include "FKSFZAudioParameterInterfaces.h"

/**
 * second attempt at the SFZ player node
 */

namespace Metasound
{
	class BKMUSICCORE_API F_FK_SFZ_Metasound_Sample_Break_Data_Operator : public TExecutableOperator<F_FK_SFZ_Metasound_Sample_Break_Data_Operator>
	{
	public:

		static const FNodeClassMetadata& GetNodeInfo();
		static const FVertexInterface& GetVertexInterface();
		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults);

		//constructor, this is important
		F_FK_SFZ_Metasound_Sample_Break_Data_Operator(const FOperatorSettings& InSettings,
			F_FK_SFZ_Region_DataReadRef& in_SFZ_layer_readRef);

		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override;
		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override;

		void Execute();

	private:

		bool bSampleInitialized = false;

		//inputs
		const FOperatorSettings Settings;
		F_FK_SFZ_Region_DataReadRef SFZ_Region_Proxy;
		//outputs
		FWaveAssetWriteRef WaveAsset;

		FFloatWriteRef PitchCorrection;
		FTimeWriteRef Offset;
		FTimeWriteRef Delay;
		FTimeWriteRef LoopStart;
		FTimeWriteRef LoopDuration;
		FBoolWriteRef Loop;
		FBoolWriteRef bOneShot;
	};

	class BKMUSICCORE_API F_FK_SFZ_Metasound_AMPEG_Break_Data_Operator : public TExecutableOperator<F_FK_SFZ_Metasound_AMPEG_Break_Data_Operator>
	{
	public:

		static const FNodeClassMetadata& GetNodeInfo();
		static const FVertexInterface& GetVertexInterface();
		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults);

		//constructor, this is important
		F_FK_SFZ_Metasound_AMPEG_Break_Data_Operator(const FOperatorSettings& InSettings,
			F_FK_SFZ_Region_DataReadRef& in_SFZ_layer_readRef);

		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override;
		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override;

		void Execute();

	private:

		bool bSampleInitialized = false;

		//inputs
		const FOperatorSettings Settings;
		F_FK_SFZ_Region_DataReadRef SFZ_Region_Proxy;
		//outputs

		//These are timed values for the envelope
		FTimeWriteRef AmpegDelay;
		FTimeWriteRef AmpegAttack;
		FTimeWriteRef AmpegHold;
		FTimeWriteRef AmpegDecay;

		FTimeWriteRef AmpegRelease;

		//These are ampltidue values for the envelope
		FFloatWriteRef AmpegStart;
		FFloatWriteRef Amplitude;
		FFloatWriteRef AmpegSustain;
	};

	//------------------------------------------------------------------------------------
// FSFZPlayerNode
//------------------------------------------------------------------------------------
	class FFK_SFZ_Break_Sample_Data_Node : public FNodeFacade
	{
	public:
		// Constructor used by the Metasound Frontend.
		FFK_SFZ_Break_Sample_Data_Node(const FNodeInitData& InitData)
			: FNodeFacade(InitData.InstanceName, InitData.InstanceID, TFacadeOperatorClass<F_FK_SFZ_Metasound_Sample_Break_Data_Operator>())
		{}
	};

	class FFK_SFZ_Break_Ampeg_Data_Node : public FNodeFacade
	{
	public:
		// Constructor used by the Metasound Frontend.
		FFK_SFZ_Break_Ampeg_Data_Node(const FNodeInitData& InitData)
			: FNodeFacade(InitData.InstanceName, InitData.InstanceID, TFacadeOperatorClass<F_FK_SFZ_Metasound_AMPEG_Break_Data_Operator>())
		{}
	};
}