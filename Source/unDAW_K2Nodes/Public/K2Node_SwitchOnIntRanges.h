// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2Node_Switch.h"
#include "K2Node_SwitchOnIntRanges.generated.h"

/**
 * This struct is used to pass additional data to the comparison function, the comparison function is static and takes two arguments, the first is the float to compare
 */

USTRUCT(BlueprintType)
struct FSwitchIntRange
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = PinOptions, meta = (NoSpinbox = true))
	int RangeMax = 0;

	UPROPERTY(EditAnywhere, Category = PinOptions)
	int RangeMin = 0;

	UPROPERTY(EditAnywhere, Category = PinOptions)
	bool Inclusive = true;
};

/**
 * 
 */
UCLASS()
class UNDAWK2_NODES_API UK2Node_SwitchOnIntRanges : public UK2Node_Switch
{
	GENERATED_BODY()


public:
	UPROPERTY()
	TArray<FName> PinNames;

	//This is the array that will hold the actual vector values for the pins, when the array is changed we will update the pins
	UPROPERTY(EditAnywhere, Category = PinOptions)
	TArray<FSwitchIntRange> PinValues;

	UK2Node_SwitchOnIntRanges();

	//this method is used to register the node in the blueprint editor
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;


	UFUNCTION(BlueprintPure, Category = PinOptions, meta = (BlueprintInternalUseOnly = "TRUE"))
	static bool IsIntNotWithinRange(int& A, FSwitchIntRange& B);

	//as we want to display the details view for the node we need to override this function and return true
	virtual bool ShouldShowNodeProperties() const override { return true; }

	FText GetTooltipText() const override;

	//K2Node_Switch Interface


	virtual void CreateSelectionPin() override;

	//I don't think this one is actually being used
	virtual FName GetPinNameGivenIndex(int32 Index) const override;
	virtual FEdGraphPinType GetInnerCaseType() const override;
	virtual FEdGraphPinType GetPinType() const override;

	//ultimately all the pins are just Exec pins, the K2Node_Switch iterates over all output exec pins and calls GetExportTextForPin, 
	// so the indices used to populate the pin in CreateCasePins should be consistent with the indices used in your GetExportTextForPin implementation
	// and values array
	virtual void CreateCasePins() override;
	virtual FName GetUniquePinName() override;

	//Generates the export text for the case pins, this is critical as the blueprint compiler uses this text to generate the structs that are used in the comparison function
	FString GetExportTextForPin(const UEdGraphPin* InPin) const override;

	virtual bool SupportsAddPinButton() const { return false; }
	//End of K2Node_Switch Interface

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void ValidateNodeDuringCompilation(class FCompilerResultsLog& MessageLog) const override;
};
