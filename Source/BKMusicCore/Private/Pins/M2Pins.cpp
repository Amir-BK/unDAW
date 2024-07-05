// Fill out your copyright notice in the Description page of Project Settings.


#include "Pins/M2Pins.h"

void UM2AudioTrackPin::BuildCompositePin(const UMetaSoundSourceBuilder& BuilderContext)
{
	UE_LOG(LogTemp, Warning, TEXT("UM2AudioTrackPin::BuildCompositePin"));
	
	switch (Direction)
	{
		case M2Sound::Pins::PinDirection::Input:
			//BuilderContext.AddInputPin(PlaceHolderData);
			break;

			case M2Sound::Pins::PinDirection::Output:
			//BuilderContext.AddOutputPin(PlaceHolderData);
			break;


	}
}
