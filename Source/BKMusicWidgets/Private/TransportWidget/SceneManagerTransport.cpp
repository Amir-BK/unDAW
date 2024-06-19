// Fill out your copyright notice in the Description page of Project Settings.


#include "TransportWidget/SceneManagerTransport.h"

void USceneManagerTransportWidget::Init()
{
	if(DawSequencerData) SetTransportDuration(DawSequencerData->SequenceDuration * .001f);
	if(DawSequencerData) DawSequencerData->OnTimeStampUpdated.AddDynamic(this, &USceneManagerTransportWidget::OnTimestampUpdated);

}
