// Fill out your copyright notice in the Description page of Project Settings.


#include "GlyphButton.h"
#include "Widgets/Input/SButton.h"
#include <BKMusicWidgets.h>
#include <TimeSyncedPanel.h>

TSharedRef<SButton> UTransportGlyphButton::CreateTransportButton(EBKTransportCommands Command)
{
	FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("unDAW"))->GetBaseDir();
	FString outString;

	switch (Command)
	{
	case EBKTransportCommands::Play:
		outString.AppendChar(TCHAR(0xF04B));
		break;
	case EBKTransportCommands::Pause:
		outString.AppendChar(TCHAR(0xF04C));
		break;

	case EBKTransportCommands::Stop:
		outString.AppendChar(TCHAR(0xF04D));
		break;

	case EBKTransportCommands::Kill:
		outString.AppendChar(TCHAR(0xF057));
		break;

	case EBKTransportCommands::Init:
		outString.AppendChar(TCHAR(0xF021));
		break;

	default:
		outString = "";
		break;
	}

	return SNew(SButton)
		.VAlign(EVerticalAlignment::VAlign_Center)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(STextBlock)
					.Text(FText::FromString(outString))
					.Font(FSlateFontInfo(PluginDir / TEXT("Resources/UtilityIconsFonts/icons.ttf"), 24))
					.Justification(ETextJustify::Center)


			];
}

bool UTransportGlyphButton::SetParentEditor(UObject* MidiEditor)
{
	ITimeSyncedPanel* interfaceCast = (ITimeSyncedPanel*) (MidiEditor);
	if (interfaceCast)
	{
		UE_LOG(LogTemp, Log, TEXT("Yay?"))
			//ParentEditorWidgetPointer = interfaceCast;
		return true;
			
	}
	
	return false;
}

void UTransportGlyphButton::SetActiveState(bool isActive)
{
	IsActive = isActive;
}

TSharedRef<SWidget> UTransportGlyphButton::RebuildWidget()
{
	FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("unDAW"))->GetBaseDir();
	FString outString;
	
	switch (TransportCommand.GetValue())
	{
	case EBKTransportCommands::Play:
		outString.AppendChar(TCHAR(0xF04B));
			break;
	case EBKTransportCommands::Pause:
		outString.AppendChar(TCHAR(0xF04C));
		break;

	case EBKTransportCommands::Stop:
		outString.AppendChar(TCHAR(0xF04D));
		break;

	case EBKTransportCommands::Kill:
		outString.AppendChar(TCHAR(0xF057));
		break;

	case EBKTransportCommands::Init:
		outString.AppendChar(TCHAR(0xF021));
		break;

	default:
		outString = "";
		break;
	}
	
	return SNew(SButton)
		//.ButtonColorAndOpacity_Lambda(([&]() {
		//return IsActive ? FLinearColor::Blue : FLinearColor::Gray; }))
		.ButtonColorAndOpacity(IconColor)
		.OnClicked(FOnClicked::CreateLambda([&, data = TransportCommand.GetValue()]() {
		TransportButtonClicked.Broadcast(TransportCommand.GetValue());
		//UE_LOG(LogTemp, Log, TEXT("Do we get in here sir?"))
		 return FReply::Handled(); }))
		.VAlign(EVerticalAlignment::VAlign_Center)
		.HAlign(EHorizontalAlignment::HAlign_Center)
		[
		SNew(STextBlock)
			.Text(FText::FromString(outString))
			.Font(FSlateFontInfo(PluginDir / TEXT("Resources/UtilityIconsFonts/icons.ttf"), 24))
			.Justification(ETextJustify::Center)

			
		];
}

void UTransportGlyphButton::ReleaseSlateResources(bool bReleaseChildren)
{

}

void UQuantizationValueGlyphButton::SetActiveState(bool isActive)
{
	IsActive = isActive;
}

TSharedRef<SWidget> UQuantizationValueGlyphButton::RebuildWidget()
{
	
	FString PluginDir = IPluginManager::Get().FindPlugin(TEXT("unDAW"))->GetBaseDir();
	FString outString = "";

	switch (QuantizationValue)
	{
	case EMusicTimeSpanOffsetUnits::Ms:

		break;
	case EMusicTimeSpanOffsetUnits::Bars:
		outString.AppendChar(TCHAR(0xE034));
		break;
	case EMusicTimeSpanOffsetUnits::Beats:
		outString.AppendChar(TCHAR(0xE0A4));

		break;
	case EMusicTimeSpanOffsetUnits::ThirtySecondNotes:
		outString.AppendChar(TCHAR(0xE1DB));
		break;
	case EMusicTimeSpanOffsetUnits::SixteenthNotes:
		outString.AppendChar(TCHAR(0xE1D9));
		break;
	case EMusicTimeSpanOffsetUnits::EighthNotes:
		outString.AppendChar(TCHAR(0xE1D7));
		break;
	case EMusicTimeSpanOffsetUnits::QuarterNotes:
		outString.AppendChar(TCHAR(0xE1D5));
		break;
	case EMusicTimeSpanOffsetUnits::HalfNotes:
		outString.AppendChar(TCHAR(0xE1D3));
		break;
	case EMusicTimeSpanOffsetUnits::WholeNotes:
		outString.AppendChar(TCHAR(0xE1D2));
		break;
	case EMusicTimeSpanOffsetUnits::DottedSixteenthNotes:
		outString.AppendChar(TCHAR(0xE1D9));
		outString.AppendChar(TCHAR(0xE1E7));
		break;
	case EMusicTimeSpanOffsetUnits::DottedEighthNotes:
		outString.AppendChar(TCHAR(0xE1D7));
		outString.AppendChar(TCHAR(0xE1E7));
		break;
	case EMusicTimeSpanOffsetUnits::DottedQuarterNotes:
		outString.AppendChar(TCHAR(0xE1D5));
		outString.AppendChar(TCHAR(0xE1E7));
		break;
	case EMusicTimeSpanOffsetUnits::DottedHalfNotes:
		outString.AppendChar(TCHAR(0xE1D3));
		outString.AppendChar(TCHAR(0xE1E7));
		break;
	case EMusicTimeSpanOffsetUnits::DottedWholeNotes:
		outString.AppendChar(TCHAR(0xE1D2));
		outString.AppendChar(TCHAR(0xE1E7));
		break;
	case EMusicTimeSpanOffsetUnits::SixteenthNoteTriplets:
		outString.AppendChar(TCHAR(0xE1D9));
		outString.AppendChar(TCHAR(0xE083));

		break;
	case EMusicTimeSpanOffsetUnits::EighthNoteTriplets:
		outString.AppendChar(TCHAR(0xE1D7));
		outString.AppendChar(TCHAR(0xE083));
		break;
	case EMusicTimeSpanOffsetUnits::QuarterNoteTriplets:
		outString.AppendChar(TCHAR(0xE1D5));
		outString.AppendChar(TCHAR(0xE083));
		break;
	case EMusicTimeSpanOffsetUnits::HalfNoteTriplets:
		outString.AppendChar(TCHAR(0xE1D3));
		outString.AppendChar(TCHAR(0xE083));
		break;
	default:
		break;
	}
	
	return SNew(SButton)
		.ButtonColorAndOpacity_Lambda(([&]() {
		return IsActive ? SelectedIconColor : IconColor; }))
		.OnClicked(FOnClicked::CreateLambda([&, data = QuantizationValue]() {
		QuantizationButtonClicked.Broadcast(QuantizationValue);
		//UE_LOG(LogTemp, Log, TEXT("Do we get in here sir?"))
		return FReply::Handled(); }))
		.VAlign(EVerticalAlignment::VAlign_Center)
		.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(STextBlock)
					.Text(FText::FromString(outString))
					.Font(FSlateFontInfo(PluginDir / TEXT("/Resources/Petaluma/redist/otf/Petaluma.otf"), FontSize))
					.Justification(ETextJustify::Center)

			];
}

void UQuantizationValueGlyphButton::ReleaseSlateResources(bool bReleaseChildren)
{

}
