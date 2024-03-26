// Fill out your copyright notice in the Description page of Project Settings.


#include "ListenerConfig/BKListnerComponentConfigWidget.h"
#include "Widgets/Text/STextBlock.h"


inline void UBKListnerComponentConfigWidget::OnObjectSlected(UObject* SelectedObject)
{
	if (SelectedObject) {
		UE_LOG(LogTemp, Log, TEXT("Object Selected, Object Name Is: %s"), *SelectedObject->GetName())
			SelectedObjectName = SelectedObject->GetName();
		AActor* AsActor = Cast<AActor>(SelectedObject);

		if (AsActor)
		{
			UDAWListenerComponent* ListenerComponent =	AsActor->FindComponentByClass<UDAWListenerComponent>();
			if (ListenerComponent)
			{
				SelectedObjectName.Append(ListenerComponent->GetName());
				OnListenerComponentSelected(ListenerComponent);
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

//TSharedRef<SWidget> UBKListnerComponentConfigWidget::RebuildWidget()
//{
//	USelection::SelectObjectEvent.AddUObject(this, &UBKListnerComponentConfigWidget::OnObjectSlected);
//	USelection::SelectNoneEvent.AddUObject(this, &UBKListnerComponentConfigWidget::OnSelectNone);
//	
//	return SNew(STextBlock)
//		.Text_Lambda([&]() {
//
//		return FText::FromString(SelectedObjectName);
//			});
//
//}

void UBKListnerComponentConfigWidget::ReleaseSlateResources(bool bReleaseChildren)
{

}
