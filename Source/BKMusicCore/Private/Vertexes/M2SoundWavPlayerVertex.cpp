// Fill out your copyright notice in the Description page of Project Settings.


#include "Vertexes/M2SoundWavPlayerVertex.h"

void UM2SoundWavPlayerVertex::BuildVertex()
{
	//EMetaSoundBuilderResult BuildResult;
	TScriptInterface<IMetaSoundDocumentInterface> WavPlayerMetasoundDocument = UnDAW::SystemPatches::TimestampWavPlayerPath.TryLoad();
	Patch = Cast<UMetaSoundPatch>(WavPlayerMetasoundDocument.GetObject());

	//GetBuilderContext().AddNode(WavPlayerMetasoundDocument, BuildResult);

	//BuilderResults.Add(FName(TEXT("Add Wav Player Node")), BuildResult);

	UM2SoundPatch::BuildVertex();
}
