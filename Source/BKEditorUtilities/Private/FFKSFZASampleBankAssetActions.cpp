// Fill out your copyright notice in the Description page of Project Settings.


#include "FFKSFZASampleBankAssetActions.h"
#include "BKMusicCore/Public/UnDAWSFZAsset.h"


UClass* FFksfzaSampleBankAssetActions::GetSupportedClass() const
{
	return UFKSFZAsset::StaticClass();
}

FText FFksfzaSampleBankAssetActions::GetName() const
{
	return INVTEXT("SFZ Sample Bank");;
}

FColor FFksfzaSampleBankAssetActions::GetTypeColor() const
{
	return FColor::Cyan;
}

uint32 FFksfzaSampleBankAssetActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}
