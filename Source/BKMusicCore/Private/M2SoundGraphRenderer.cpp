// Fill out your copyright notice in the Description page of Project Settings.


#include "M2SoundGraphRenderer.h"
#include "Engine/Engine.h"
#include "IAudioParameterInterfaceRegistry.h"
#include "Kismet/GameplayStatics.h"

#include "MetasoundGeneratorHandle.h"
#include "MetasoundGenerator.h"
#include "MetasoundAssetSubsystem.h"
#include "MetasoundAssetBase.h"
#include "MetasoundOutputSubsystem.h"
#include "MetasoundFrontendSearchEngine.h"
#include "M2SoundGraphStatics.h"
#include "Vertexes/M2SoundVertex.h"
#include "Interfaces/unDAWMetasoundInterfaces.h"




//void UM2SoundGraphRenderer::SendTransportCommand(EBKTransportCommands Command)
//{
//	if (AuditionComponentRef)
//	{
//		switch (Command)
//		{
//
//		case EBKTransportCommands::Play:
//			AuditionComponentRef->SetTriggerParameter(FName(TEXT("unDAW.Transport.Play")));
//			PlayState = EBKPlayState::Playing;
//			break;
//
//		case EBKTransportCommands::Stop:
//			AuditionComponentRef->SetTriggerParameter(FName(TEXT("unDAW.Transport.Stop")));
//			PlayState = EBKPlayState::ReadyToPlay;
//
//			break;
//
//		case EBKTransportCommands::Pause:
//		AuditionComponentRef->SetTriggerParameter(FName(TEXT("unDAW.Transport.Pause")));
//			PlayState = EBKPlayState::Paused;
//			break;
//		default:
//			break;
//
//
//		}
//	}
//}

void UDEPRECATED_M2SoundGraphRenderer::InitPerformer()
{

	//outer must be dawsequencerdata
	SessionData = CastChecked<UDAWSequencerData>(GetOuter());

	MSBuilderSystem = GEngine->GetEngineSubsystem<UMetaSoundBuilderSubsystem>();
	EMetaSoundBuilderResult BuildResult;
	FString BuilderString = FString::Printf(TEXT("unDAW-%s"), *SessionData->GetName());

	FMetaSoundBuilderNodeInputHandle OnFinished;

	auto BuilderName = FName(BuilderString);
	auto SavedBuilder = MSBuilderSystem->FindBuilder(BuilderName);

	if(SavedBuilder)
	{
		//found saved builder!
		UE_LOG(LogTemp, Log, TEXT("Found saved builder!"))
		BuilderContext = Cast<UMetaSoundSourceBuilder>(SavedBuilder);
		return;
	}
	else {
	BuilderContext = MSBuilderSystem->CreateSourceBuilder(BuilderName, OnPlayOutputNode, OnFinished, AudioOuts, BuildResult, SessionData->MasterOptions.OutputFormat, false);
	MSBuilderSystem->RegisterBuilder(BuilderName, BuilderContext);
	UE_LOG(LogTemp, Log, TEXT("Creating new builder"))
	}



	//after builder is created, create the nodes and connections

	//create renderer interface default IOs
	BuilderContext->AddInterface(FName(TEXT("unDAW Session Renderer")), BuildResult);

	//add main clock patch by reference - '/Script/MetasoundEngine.MetaSoundPatch'/unDAW/Patches/System/unDAW_MainClock.unDAW_MainClock'
	CreateMidiPlayerBlock();

	FSoftObjectPath MidiFilterPatchSoftPath(TEXT("/unDAW/Patches/System/unDAW_MidiFilter.unDAW_MidiFilter"));
	MidiFilterDocument = MidiFilterPatchSoftPath.TryLoad();
	//CurrentBuilder->AddNode(MidiFilterDocument, BuildResult);
	//UMetaSoundPatch* MidiPatch = Cast<UMetaSoundPatch>(MidiPlayerDocument.GetObject());

	CreateMainMixer();


	TArray<UM2SoundVertex*> AllVertices = UM2SoundGraphStatics::GetAllVertexesInSequencerData(SessionData);

	
	//first we traverse the vertexes and build them, each is expected to return meaningful builder data which we populate into the map
	//thus the presence of a node in the map is also a signifier that it has been intiialized within our renderer context
	//we probably need to differentiate between building a node for the context vs. creating the vertex
	for (const auto& Vertex :SessionData->GetVertexes())
	{
		FBuilderVertexCompositeData BuiltData;
		//Vertex->BuildVertex(this, BuiltData);
		VertexToBuilderDataMap.Add(Vertex, BuiltData);
	}

	//once all vertexes are built, we can make the non-static graph connections
	for (auto& [Vertex, Data] : VertexToBuilderDataMap)
	{
		//Vertex->UpdateConnections(this, Data);
	}

	SessionData->OnVertexAdded.AddDynamic(this, &UDEPRECATED_M2SoundGraphRenderer::UpdateVertex);
	SessionData->OnAudioParameterFromVertex.AddDynamic(this, &UDEPRECATED_M2SoundGraphRenderer::ReceiveAudioParameter);

}

FM2SoundMetasoundBuilderPinData CreatePinDataFromBuilderData(UMetaSoundSourceBuilder* Builder, FMetaSoundBuilderNodeInputHandle Input, EMetaSoundBuilderResult& BuildResult)
{
	FName NodeName;
	FName DataType;
	Builder->GetNodeInputData(Input, NodeName, DataType, BuildResult);
	FM2SoundMetasoundBuilderPinData PinData;
	PinData.PinName = NodeName;
	PinData.DataType = DataType;
	return PinData;
}

//This whole thing is a catastrophe, we need to refactor this to be more modular
void UDEPRECATED_M2SoundGraphRenderer::UpdateVertex(UM2SoundVertex* Vertex)
{
	
	
	EMetaSoundBuilderResult BuildResult;
	
	//clear vertex build results
	//Vertex->BuilderResults.Empty();

	//UE_LOG(LogTemp, Log, TEXT("Vertex Name: %s"), *Vertex->GetName())
	//Vertex->GetDocumentChecked().GetDocumentName();
	if (UM2SoundMidiInputVertex* InputVertex = Cast<UM2SoundMidiInputVertex>(Vertex))
	{
		//create track filter node
		InputVertex->MetasoundOutputs.Empty();
		//add node by patch reference - '/unDAW/Patches/System/unDAW_MidiFilter.unDAW_MidiFilter'
		auto ChannelFilterNode = BuilderContext->AddNode(MidiFilterDocument, BuildResult);
		InputVertex->BuilderResults.Add(FName("Main Node"), BuildResult);

		//connect input of filter node to midi player node stream output

		//add filter output to vertex outputs
		auto FilterOutputs = BuilderContext->FindNodeOutputs(ChannelFilterNode, BuildResult);
		for (const auto& Output : FilterOutputs)
		{
			FName OutputName;
			FName DataType;
			BuilderContext->GetNodeOutputData(Output, OutputName, DataType, BuildResult);
			InputVertex->MetasoundOutputs.Add(FM2SoundMetasoundBuilderPinData{ OutputName, DataType });
			//InputVertex->MidiStreamOutput = Output;
			//Connect the midi stream output to the midi player node
			InputVertex->BuilderResults.Add(OutputName, BuildResult);

			//hacky but we assume only one output here
		}

		auto& TrackMetadata = SessionData->GetTracksDisplayOptions(InputVertex->TrackId);


		//should happen way before we build the graph
		//InputVertex->TrackPrefix = FString::Printf(TEXT("Tr%d_Ch%d."), TrackMetadata.TrackIndexInParentMidi, TrackMetadata.ChannelIndexInParentMidi);

		//find midi input and connect it to main midi player
		auto MidiInput = BuilderContext->FindNodeInputByName(ChannelFilterNode, FName(TEXT("MIDI Stream")), BuildResult);

		//find int input named "track" and assign its default value to the vertex's track id
		auto TrackInput = BuilderContext->FindNodeInputByName(ChannelFilterNode, FName(TEXT("Track")), BuildResult);
		FName intDataType = TEXT("int32");

		auto TrackInputNodeOutput = BuilderContext->AddGraphInputNode(FName(InputVertex->TrackPrefix + TEXT("TrackNum")), TEXT("int32"), MSBuilderSystem->CreateIntMetaSoundLiteral(TrackMetadata.TrackIndexInParentMidi, intDataType), BuildResult);
		BuilderContext->ConnectNodes(TrackInputNodeOutput, TrackInput, BuildResult);

		//same for "channel"
		auto ChannelInput = BuilderContext->FindNodeInputByName(ChannelFilterNode, FName(TEXT("Channel")), BuildResult);
		auto ChannelInputNodeOutput = BuilderContext->AddGraphInputNode(FName(InputVertex->TrackPrefix + TEXT("Channel")), TEXT("int32"), MSBuilderSystem->CreateIntMetaSoundLiteral(TrackMetadata.ChannelIndexInParentMidi, intDataType), BuildResult);
		BuilderContext->ConnectNodes(ChannelInputNodeOutput, ChannelInput, BuildResult);
		//add to build results
		InputVertex->BuilderResults.Add(FName(TEXT("Track Connection")), BuildResult);


		BuilderContext->ConnectNodes(MainMidiStreamOutput, MidiInput, BuildResult);
		InputVertex->BuilderResults.Add(FName(TEXT("Connect to main player")), BuildResult);
		
		//CreateDefaultVertexesFromInputVertex(InSessionData, InputVertex, InputVertex->TrackId);

		
	}

	UM2SoundAudioOutput* OutputVertex;
	//until I implement a solution for pending connections the output needs to be created after the patch...
	//if (UM2SoundAudioOutput* OutputVertex = Cast< UM2SoundAudioOutput>(Vertex))
	if(false)
	{
		//really if this was encapsulated on the vertex itself it would be better, but here we go
		//check if map already contains this vertex, if not, assign an output
		
		if (!AudioOutsMap.Contains(Vertex))
		{
			AudioOutsMap.Add(Vertex, GetFreeAudioOutputAssignable());
		}

		auto& AssignedOutput = AudioOutsMap[Vertex];
		OutputVertex->MetasoundInputs.Empty();
		//get free output and give it to this vertex
		
		//auto FreeOutputs = GetFreeAudioOutput();
		//OutputVertex->AssignedOutput.AudioLeftOutputInputHandle = FreeOutputs.Pop();
		OutputVertex->MetasoundInputs.Add(CreatePinDataFromBuilderData(BuilderContext, AssignedOutput.AudioLeftOutputInputHandle, BuildResult));

		//OutputVertex->AssignedOutput.AudioRightOutputInputHandle = FreeOutputs.Pop();
		OutputVertex->MetasoundInputs.Add(CreatePinDataFromBuilderData(BuilderContext, AssignedOutput.AudioRightOutputInputHandle, BuildResult));
		//OutputVertex->AssignedOutput.GainParameterName = FName(GetName());
		OutputVertex->GainParameterName = FName(FGuid::NewGuid().ToString());

		//create float graph input with the GainParameterName and connect it to the gain input of the mixer in the builder
		FName OutDataType;
		auto NewGainInput = BuilderContext->AddGraphInputNode(OutputVertex->GainParameterName, TEXT("float"), MSBuilderSystem->CreateFloatMetaSoundLiteral(1.f, OutDataType), BuildResult);
		BuilderContext->ConnectNodes(NewGainInput, AssignedOutput.GainParameterInputHandle, BuildResult);
		OutputVertex->BuilderResults.Add(FName(TEXT("Connect to Gain")), BuildResult);

		//if has track input, get its audio outputs and connect them to this output
		// it's hacky but we assume only Track-Audio Output connections here, which must have two audio outputs
		if (OutputVertex->MainInput)
		{
			//find handle of the main input vertex in our map
			//check if map contains the node, otherwise it hasn't been built yet

			bool bInputHasBeenBuilt = VertexToNodeMap.Contains(OutputVertex->MainInput);
			if(bInputHasBeenBuilt)
			{
				//UpdateVertex(OutputVertex->MainInput);
				auto InputBuilderNodeHandle = VertexToNodeMap[OutputVertex->MainInput];
				//use the builder to find the audio outputs of the input node
				auto InputNodeOutputs = BuilderContext->FindNodeOutputsByDataType(InputBuilderNodeHandle, BuildResult, FName(TEXT("Audio")));
				//connect the audio outputs to this vertex's assigned audio output
				BuilderContext->ConnectNodes(InputNodeOutputs[0], AssignedOutput.AudioLeftOutputInputHandle, BuildResult);
				Vertex->BuilderResults.Add(FName(TEXT("Connect to instrument audio L")), BuildResult);
				BuilderContext->ConnectNodes(InputNodeOutputs[1], AssignedOutput.AudioRightOutputInputHandle, BuildResult);
				Vertex->BuilderResults.Add(FName(TEXT("Connect to instrument audio R")), BuildResult);
			}
			else {
				Vertex->BuilderResults.Add(FName(TEXT("Input not built yet!")), EMetaSoundBuilderResult::Failed);
			}

		}
		else {
			Vertex->BuilderResults.Add(FName(TEXT("No Input")), EMetaSoundBuilderResult::Succeeded);
			if(BuilderContext->NodeInputIsConnected(AssignedOutput.AudioLeftOutputInputHandle))
			{
				BuilderContext->DisconnectNodeInput(AssignedOutput.AudioLeftOutputInputHandle, BuildResult);
				Vertex->BuilderResults.Add(FName(TEXT("Disconnect L")), BuildResult);
			}
			if(BuilderContext->NodeInputIsConnected(AssignedOutput.AudioRightOutputInputHandle))
			{
				BuilderContext->DisconnectNodeInput(AssignedOutput.AudioRightOutputInputHandle, BuildResult);
				Vertex->BuilderResults.Add(FName(TEXT("Disconnect R")), BuildResult);
			}
		}
	}


	if (UM2SoundPatch* PatchVertex = Cast<UM2SoundPatch>(Vertex))
	{
		//check if we already have a node and if so, delete it 
		//@@TODO : This is too much! Sometimes only connections need to be updated!
		bool bHadPreviousConnection = false;
		if (VertexToNodeMap.Contains(PatchVertex))
		{
			auto NodeHandle = VertexToNodeMap[PatchVertex];
			BuilderContext->RemoveNode(NodeHandle, BuildResult);
			VertexToNodeMap.Remove(PatchVertex);
			bHadPreviousConnection = true;
		}

		TScriptInterface<IMetaSoundDocumentInterface> asDocInterface = PatchVertex->Patch;
		auto NewNodeHandle = BuilderContext->AddNode(asDocInterface, BuildResult);
		auto NodeInputs = BuilderContext->FindNodeInputs(NewNodeHandle, BuildResult);
		auto NodeOutputs = BuilderContext->FindNodeOutputs(NewNodeHandle, BuildResult);

		// clear vertex discovered I/Os

		//in theory, if we empty these and then recreate I/Os the I/Os that maintain their names can be reconnected
		PatchVertex->MetasoundInputs.Empty();
		PatchVertex->MetasoundOutputs.Empty();
		
		for( const auto& Input : NodeInputs)
		{
			FName NodeName;
			FName DataType;
			BuilderContext->GetNodeInputData(Input, NodeName, DataType, BuildResult);
			PatchVertex->MetasoundInputs.Add(FM2SoundMetasoundBuilderPinData{ NodeName, DataType });

		}


		if(PatchVertex->Outputs.Num() > 0)
		{
			TArray<FMetaSoundBuilderNodeOutputHandle> PatchAudioOutputs;
			
			for (const auto& Output : NodeOutputs)
			{

				FName OutputName;
				FName DataType;
				BuilderContext->GetNodeOutputData(Output, OutputName, DataType, BuildResult);

				if (DataType == FName(TEXT("Audio")))
				{
					PatchAudioOutputs.Add(Output);
				}

				BuilderContext->GetNodeOutputData(Output, OutputName, DataType, BuildResult);
				PatchVertex->MetasoundOutputs.Add(FM2SoundMetasoundBuilderPinData{ OutputName, DataType });
			}

		}

		
		VertexToNodeMap.Add(PatchVertex, NewNodeHandle);

		if(PatchVertex->MainInput)
		{
			//fusion patch reference for patch node
			auto PatchForLiteral = SessionData->GetTracksDisplayOptions(1).fusionPatch.Get();
			
			auto PatchInput = BuilderContext->FindNodeInputByName(NewNodeHandle, FName(TEXT("Patch")), BuildResult);
			auto PatchInputNodeOutput = BuilderContext->AddGraphInputNode(TEXT("Patch"), TEXT("FusionPatchAsset"), MSBuilderSystem->CreateObjectMetaSoundLiteral(PatchForLiteral), BuildResult);
			BuilderContext->ConnectNodes(PatchInputNodeOutput, PatchInput, BuildResult);

			//connect this instrument renderer to the midi stream output by vertex connection
			auto MidiStreamInput = BuilderContext->FindNodeInputByName(NewNodeHandle, FName(TEXT("unDAW Instrument.MidiStream")), BuildResult);
			auto AsInputVertex = Cast<UM2SoundMidiInputVertex>(PatchVertex->MainInput);

			//find interface track num input
			auto TrackInput = BuilderContext->FindNodeInputByName(NewNodeHandle, FName(TEXT("unDAW Instrument.MidiTrack")), BuildResult);
			//find the track number input via input vertex, it should already exist, use the track prefix from the input note
			//auto TrackNumGraphInput = CurrentBuilder->FindGraphInputNode(, BuildResult);
			BuilderContext->ConnectNodeInputToGraphInput(FName(AsInputVertex->TrackPrefix + TEXT("TrackNum")), TrackInput, BuildResult);
			//add to build results
			PatchVertex->BuilderResults.Add(FName(TEXT("Track Num Connection")), BuildResult);



			//BuilderContext->ConnectNodes(AsInputVertex->MidiStreamOutput, MidiStreamInput, BuildResult);
			//add to build results
			PatchVertex->BuilderResults.Add(FName(TEXT("Connect to Midi Track")), BuildResult);

		}

		//CreateDefaultVertexesFromInputVertex(InSessionData, PatchVertex, PatchVertex->TrackId);
		if(bHadPreviousConnection)
		{
			//iterate over the outputs and tell them to rebuild
			for (auto& Output : PatchVertex->Outputs)
			{
				UpdateVertex(Output);
			}
		}
	}


	Vertex->OnVertexUpdated.Broadcast();
	//Vertex->OnVertexNeedsBuilderNodeUpdates.AddDynamic(this, &UDEPRECATED_M2SoundGraphRenderer::UpdateVertex);

}

void UDEPRECATED_M2SoundGraphRenderer::UpdateVertexConnections(UM2SoundVertex* Vertex)
{

}

void UDEPRECATED_M2SoundGraphRenderer::CreateAuditionableMetasound(UAudioComponent* InComponent, bool bReceivesLiveUpdates)
{

	if(!BuilderContext)
	{
		//This should never be called, a builder must be set up via InitPerformer prior to attempting to create a live audition component
		checkNoReentry();
	}
	
	if (AuditionComponentRef)
	{
		AuditionComponentRef->Stop();
		AuditionComponentRef->DestroyComponent();
	}


	AuditionComponentRef = InComponent;


	PlayState = EBKPlayState::ReadyToPlay;
	FOnCreateAuditionGeneratorHandleDelegate OnCreateAuditionGeneratorHandle;
	OnCreateAuditionGeneratorHandle.BindUFunction(this, TEXT("OnMetaSoundGeneratorHandleCreated"));
	BuilderContext->Audition(this, AuditionComponentRef, OnCreateAuditionGeneratorHandle, bReceivesLiveUpdates);

}

void UDEPRECATED_M2SoundGraphRenderer::CreateVertex(UM2SoundVertex* Vertex)
{
	//can't add vertex that is already initialized to the context.
	//if (IsVertexInitialized(Vertex)) return;

	//Vertex->BuildVertex(this);

	InitializedVertexes.Add(Vertex);


	// this delegate already gets called by the GraphData whenever a vertex is updated, as its a multicast we should already be covered
	//bind to vertex updates
	//Vertex->OnVertexUpdated.AddDynamic(this, &UDEPRECATED_M2SoundGraphRenderer::UpdateVertex);
}

void UDEPRECATED_M2SoundGraphRenderer::SaveMetasoundToAsset()
{

}


void UDEPRECATED_M2SoundGraphRenderer::Tick(float DeltaTime)
{
	//UE_LOG(LogTemp, Log, TEXT("Tick! Sequencer Asset Name: %s"), *SessionData->GetName())
	//This should actually only be called in editor, as otherwise the metasound watch output subsystem should handle ticking the watchers
	if (!AuditionComponentRef) return;

		GeneratorHandle->UpdateWatchers();


}

inline bool UDEPRECATED_M2SoundGraphRenderer::IsTickable() const { return bShouldTick; }

inline ETickableTickType UDEPRECATED_M2SoundGraphRenderer::GetTickableTickType() const
{
	return ETickableTickType::Conditional;
}

inline TStatId UDEPRECATED_M2SoundGraphRenderer::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FMyTickableThing, STATGROUP_Tickables);
}

inline bool UDEPRECATED_M2SoundGraphRenderer::IsTickableWhenPaused() const
{
	return true;
}

inline bool UDEPRECATED_M2SoundGraphRenderer::IsTickableInEditor() const
{
	return true;
}

inline void UDEPRECATED_M2SoundGraphRenderer::RemoveFromParent()
{
	OnDeleted.Broadcast();
}

void UDEPRECATED_M2SoundGraphRenderer::SendSeekCommand(float InSeek)
{
	UE_LOG(LogTemp, Log, TEXT("Seek Command Received! %f"), InSeek)
	if (AuditionComponentRef)
	{
		AuditionComponentRef->SetFloatParameter(FName(TEXT("unDAW.Transport.SeekTarget")), InSeek * 1000.f);
		AuditionComponentRef->SetTriggerParameter(FName(TEXT("unDAW.Transport.Seek")));
	}
}

void UDEPRECATED_M2SoundGraphRenderer::ReceiveMetaSoundMidiStreamOutput(FName OutputName, const FMetaSoundOutput Value)
{
	//UE_LOG(LogTemp, Log, TEXT("Output Received! %s, Data type: %s"), *OutputName.ToString(), *Value.GetDataTypeName().ToString())
		//auto MidiClockValue = Value.GetDataTypeName();
}

void UDEPRECATED_M2SoundGraphRenderer::ReceiveMetaSoundMidiClockOutput(FName OutputName, const FMetaSoundOutput Value)
{
	//UE_LOG(LogTemp, Log, TEXT("Output Received! %s, Data type: %s"), *OutputName.ToString(), *Value.GetDataTypeName().ToString())
	Value.Get(CurrentTimestamp);
	OnTimestampUpdated.Broadcast(CurrentTimestamp);
	OnMusicTimestampFromPerformer.ExecuteIfBound(CurrentTimestamp);
	
}

void UDEPRECATED_M2SoundGraphRenderer::ReceiveAudioParameter(FAudioParameter Parameter)
{
	if(AuditionComponentRef) AuditionComponentRef->SetParameter(MoveTemp(Parameter));
}


void FindAllMetaSoundClasses()
{
	using namespace Metasound;
	using namespace Metasound::Frontend;

	FMetasoundFrontendClass RegisteredClass;

	//auto AllClasses = ISearchEngine::Get().FindAllClasses(true);
	//for (const auto MetaClass : AllClasses)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("Class: %s"), *MetaClass.Metadata.GetClassName().GetFullName().ToString())

	//}

}


bool SwitchOnBuildResult(EMetaSoundBuilderResult BuildResult)
{
	switch (BuildResult)
	{
	case EMetaSoundBuilderResult::Succeeded:
		return true;
		break;
	case EMetaSoundBuilderResult::Failed:
		return false;
		break;

		default:
			return false;
	}
}

void UDEPRECATED_M2SoundGraphRenderer::PopulateAssignableOutputsArray(TArray<FAssignableAudioOutput>& OutAssignableOutputs, const TArray<FMetaSoundBuilderNodeInputHandle> InMixerNodeInputs)
{
	// we need to keep the audio outputs and the float parameters organized within the assignable outputs array
	// so we can easily assign them to the audio outputs of the mixer node

	//EMetaSoundBuilderResult BuildResult;


	for (SIZE_T i = 0; i < InMixerNodeInputs.Num(); i += 3)
	{
		//the outputs are always sorted "In 0 L", "In 0 R", "Gain 0"
		FAssignableAudioOutput AssignableOutput;
		AssignableOutput.AudioLeftOutputInputHandle = InMixerNodeInputs[i];
		AssignableOutput.AudioRightOutputInputHandle = InMixerNodeInputs[i + 1];
		AssignableOutput.GainParameterInputHandle = InMixerNodeInputs[i + 2];

		MasterOutputs.Add(AssignableOutput);

	}

}

void UDEPRECATED_M2SoundGraphRenderer::CreateMainMixer()
{
	// create master mixer
	EMetaSoundBuilderResult BuildResult;
	const auto MasterMixerNode = BuilderContext->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("AudioMixer")), FName(TEXT("Audio Mixer (Stereo, 8)")))
		, BuildResult);

	auto MixerOutputs = BuilderContext->FindNodeOutputs(MasterMixerNode, BuildResult);
	//auto AudioOuts = GetAvailableOutput();
	bool usedLeft = false;
	for (const auto& Output : MixerOutputs)
	{
		FName NodeName;
		FName DataType;
		BuilderContext->GetNodeOutputData(Output, NodeName, DataType, BuildResult);
		if (DataType == FName(TEXT("Audio")))
		{
			BuilderContext->ConnectNodes(Output, AudioOuts[usedLeft ? 1 : 0], BuildResult);
			usedLeft = true;

		}
	}

	PopulateAssignableOutputsArray(MasterOutputs, BuilderContext->FindNodeInputs(MasterMixerNode, BuildResult));

	//MasterOutputsArray.Append(CurrentBuilder->FindNodeInputsByDataType(MasterMixerNode, BuildResult, FName(TEXT("Audio"))));
		
}

void UDEPRECATED_M2SoundGraphRenderer::AddVertex(UM2SoundVertex* Vertex)
{
		//can't add vertex that is already initialized to the context.
		//if(IsVertexInitialized(Vertex)) return;
	
		InitializedVertexes.Add(Vertex);

		//bind to vertex updates
		//Vertex->OnVertexUpdated.AddDynamic(this, &UDEPRECATED_M2SoundGraphRenderer::UpdateVertex);

		//Vertex->OnVertexNeedsBuilderNodeUpdates.AddDynamic(this, &UDEPRECATED_M2SoundGraphRenderer::UpdateVertex);
		//Vertex->OnVertexNeedsBuilderConnectionUpdates.AddDynamic(this, &UDEPRECATED_M2SoundGraphRenderer::UpdateVertexConnections);

		//if vertex is output, add it to the master outputs
		//if (UM2SoundAudioOutput* OutputVertex = Cast<UM2SoundAudioOutput>(Vertex))
		//{
		//		AudioOutsMap.Add(Vertex, GetFreeAudioOutputAssignable());
		//}
}





	void UDEPRECATED_M2SoundGraphRenderer::AttachAnotherMasterMixerToOutput()
	{
		EMetaSoundBuilderResult BuildResult;
		const auto MasterMixerNode = BuilderContext->AddNodeByClassName(FMetasoundFrontendClassName(FName(TEXT("AudioMixer")), FName(TEXT("Audio Mixer (Stereo, 8)")))
			, BuildResult);
		
				auto MixerOutputs = BuilderContext->FindNodeOutputs(MasterMixerNode, BuildResult);
				//auto AudioOuts = GetAvailableOutput();
				//print the num of the MasterOutputs it should be exactly 1!
				UE_LOG(LogTemp, Log, TEXT("Master Outputs Num: %d"), MasterOutputs.Num())
				auto LastRemainingOutput = MasterOutputs.Pop();
				BuilderContext->ConnectNodes(MixerOutputs[1], LastRemainingOutput.AudioLeftOutputInputHandle, BuildResult);
				BuilderContext->ConnectNodes(MixerOutputs[0], LastRemainingOutput.AudioRightOutputInputHandle, BuildResult);

				PopulateAssignableOutputsArray(MasterOutputs, BuilderContext->FindNodeInputs(MasterMixerNode, BuildResult));
				//MasterOutputsArray.Append(CurrentBuilder->FindNodeInputsByDataType(MasterMixerNode, BuildResult, FName(TEXT("Audio"))));
	}



	FAssignableAudioOutput UDEPRECATED_M2SoundGraphRenderer::GetFreeAudioOutputAssignable()
	{
		if(MasterOutputs.Num() > 1)
		{
			return MasterOutputs.Pop();
		}
		else
		{
			AttachAnotherMasterMixerToOutput();
			return GetFreeAudioOutputAssignable();
		}
	}


	// we do not use this due to the issues with the frontend only registering the patch when the metasound graph is opened, which is annoying.
	bool UDEPRECATED_M2SoundGraphRenderer::CreateMidiPlayerBlock()
{
		// Create the midi player block
	EMetaSoundBuilderResult BuildResult;
	FSoftObjectPath MidiPlayerAssetRef(TEXT("/unDAW/Patches/System/unDAW_MainClock.unDAW_MainClock"));
	TScriptInterface<IMetaSoundDocumentInterface> MidiPlayerDocument = MidiPlayerAssetRef.TryLoad();
	UMetaSoundPatch* MidiPatch = Cast<UMetaSoundPatch>(MidiPlayerDocument.GetObject());
	//MidiPatch->RegisterGraphWithFrontend();
	MidiPlayerNode = BuilderContext->AddNode(MidiPlayerDocument, BuildResult);

	if (SwitchOnBuildResult(BuildResult))
	{
		BuilderContext->ConnectNodeInputsToMatchingGraphInterfaceInputs(MidiPlayerNode, BuildResult);
	}

	if (SwitchOnBuildResult(BuildResult))
	{
		BuilderContext->ConnectNodeOutputsToMatchingGraphInterfaceOutputs(MidiPlayerNode, BuildResult);
	}

	FMetaSoundBuilderNodeInputHandle MidiFileInput = BuilderContext->FindNodeInputByName(MidiPlayerNode, FName(TEXT("MIDI File")), BuildResult);
	MainMidiStreamOutput = BuilderContext->FindNodeOutputByName(MidiPlayerNode, FName(TEXT("unDAW.Midi Stream")), BuildResult);
	auto MidiInputPinOutputHandle = BuilderContext->AddGraphInputNode(TEXT("Midi File"), TEXT("MidiAsset"), MSBuilderSystem->CreateObjectMetaSoundLiteral(SessionData->HarmonixMidiFile), BuildResult);
	BuilderContext->ConnectNodes(MidiInputPinOutputHandle, MidiFileInput, BuildResult);


	return SwitchOnBuildResult(BuildResult);

	//FMetaSoundBuilderNodeInputHandle MidiPlayerNode = CurrentBuilder->CreateMidiPlayerNode(FName(TEXT("Midi Player")), FLinearColor::Green, 0);
}

void UDEPRECATED_M2SoundGraphRenderer::CreateAndAuditionPreviewAudioComponent()
{
	PlayState = EBKPlayState::ReadyToPlay;
	FOnCreateAuditionGeneratorHandleDelegate OnCreateAuditionGeneratorHandle;
	OnCreateAuditionGeneratorHandle.BindUFunction(this, TEXT("OnMetaSoundGeneratorHandleCreated"));
	BuilderContext->Audition(this, AuditionComponentRef, OnCreateAuditionGeneratorHandle, true);
}

void UDEPRECATED_M2SoundGraphRenderer::OnMetaSoundGeneratorHandleCreated(UMetasoundGeneratorHandle* Handle)
{

	UMetaSoundAssetSubsystem* AssetSubsystem = GEngine->GetEngineSubsystem<UMetaSoundAssetSubsystem>();
	//FMetasoundAssetBase test = Cast<UMetaSoundSource>();
	AssetSubsystem->Get()->AddOrUpdateAsset(*AuditionComponentRef->GetSound()->_getUObject());
	//UE_LOG(LogTemp, Log, TEXT("Handle Created!"))
	GeneratorHandle = Handle;
	AuditionComponentRef->GetSound()->VirtualizationMode = EVirtualizationMode::PlayWhenSilent;
	AuditionComponentRef->SetTriggerParameter(FName("unDAW.Transport.Prepare"));
	OnMidiClockOutputReceived.BindLambda([this](FName OutputName, const FMetaSoundOutput Value) { ReceiveMetaSoundMidiClockOutput(OutputName, Value); });
	OnMidiStreamOutputReceived.BindLambda([this](FName OutputName, const FMetaSoundOutput Value) { ReceiveMetaSoundMidiStreamOutput(OutputName, Value); });


	bool CanWatchStream = GeneratorHandle->WatchOutput(FName("unDAW.Midi Stream"), OnMidiStreamOutputReceived);
	bool CanWatchClock = GeneratorHandle->WatchOutput(FName("unDAW.Midi Clock"), OnMidiClockOutputReceived);
	bShouldTick = true;

	OnDAWPerformerReady.Broadcast();
}




