// Fill out your copyright notice in the Description page of Project Settings.


#include "ListenerConfig/BKListnerComponentConfigWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SNumericEntryBox.h"


inline void UBKListnerComponentConfigWidget::OnObjectSlected(UObject* SelectedObject)
{
	if (SelectedObject) {
		UE_LOG(LogTemp, Log, TEXT("Object Selected, Object Name Is: %s"), *SelectedObject->GetName())
			SelectedObjectName = SelectedObject->GetName();
		AActor* AsActor = Cast<AActor>(SelectedObject);

		if (AsActor)
		{
			ControlledComponent =	AsActor->FindComponentByClass<UDAWListenerComponent>();
			if (ControlledComponent)
			{
				SelectedObjectName.Append(ControlledComponent->GetName());
				OnListenerComponentSelected(ControlledComponent);
				//if(ComponentDetailsView) ComponentDetailsView->
			}
		}
	}
	else {
		SelectedObjectName = "";
	}

}

void UBKListnerComponentConfigWidget::OnSelectNone()
{
	SelectedObjectName = "";
}

void UBKListnerComponentConfigWidget::NativeConstruct()
{
	USelection::SelectObjectEvent.AddUObject(this, &UBKListnerComponentConfigWidget::OnObjectSlected);
	USelection::SelectNoneEvent.AddUObject(this, &UBKListnerComponentConfigWidget::OnSelectNone);
}

TSharedRef<SWidget> UBKListnerComponentConfigWidget::RebuildWidget()
{
	USelection::SelectObjectEvent.AddUObject(this, &UBKListnerComponentConfigWidget::OnObjectSlected);
	USelection::SelectNoneEvent.AddUObject(this, &UBKListnerComponentConfigWidget::OnSelectNone);
	
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(STextBlock)
				.Text_Lambda([&]() {return FText::FromString(SelectedObjectName); })
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[

			SNew(STextBlock)
				.Text_Lambda([&]() {return FText::FromString((ControlledComponent && ControlledComponent->SceneManager) ? ControlledComponent->SceneManager->GetName() : TEXT("No scene manager"));	})
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(SNumericEntryBox<float>)
				.AllowSpin(true)
				.MinValue(0.0f)
				.MaxValue(2000.0f)
				.Value(5.0f)
				//.OnValueChanged(this, &SImportSFZSettingsDialog::setAmpegRelease)
				.MinSliderValue(0.0f)
				.MaxSliderValue(2000.0f)
				.WheelStep(0.01f)
				.MinFractionalDigits(6)
				.Label()
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("MaxNote")))
						//.TextStyle(&SFZImportInfoStyle)
				]

		];
}


void UBKListnerComponentConfigWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
}
