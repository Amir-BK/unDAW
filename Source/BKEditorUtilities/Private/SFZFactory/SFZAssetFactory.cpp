// Fill out your copyright notice in the Description page of Project Settings.


#include "SFZAssetFactory.h"
#include "BKMusicCore/Public/FKSFZAsset.h"
#include "Misc/Char.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "SlateFwd.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "SEnumCombo.h"

#include <AssetToolsModule.h>

#include "FK_SFZ_OpCode_Interpreter.h"


bool USFZOpCodeLineParser::TraverseString(FString StringToTraverse)
{
	TraversalString = StringToTraverse;
	CurrentIndex = 0;

	if (StringToTraverse.Contains(TEXT("=")))
	{
		TArray<FString> LinesArray;
		StringToTraverse.ParseIntoArrayLines(LinesArray);
		for (auto Line : LinesArray)
		{
			if (Line.StartsWith(TEXT("//"))) continue; //skip comments; 
			CheckForOpCodesAndTraverse(Line);
		}
			
		return true;
	}
	return false;	
}

void USFZOpCodeLineParser::CombineOpCodesWithParents()
{
	if (ParentPtr != nullptr && ParentPtr != this)
	{
		OpCodeToValuesMap.Append(ParentPtr->OpCodeToValuesMap);
	}
}

#define VAL_IS(a) TrimmedValue.Equals(TEXT(a), ESearchCase::IgnoreCase)
#define KEY_IS(a) key.ToString().Equals(TEXT(a), ESearchCase::IgnoreCase)


void USFZOpCodeLineParser::PopulateSFZRegion(USFZRegion* Region, UFKSFZAsset* NewAsset, USFZAssetFactory* Factory)
{
	
	FString ResultPath;
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");//shameful reminder to the fact that I currently don't pass the octave offset
	
	//deal with defines 
	for(const auto&[key, value] : OpCodeToValuesMap)
	{
		FString TrimmedValue = value.TrimStartAndEnd();
		
		for (auto varDef : NewAsset->AssetDefines)
		{
			if (TrimmedValue.Equals(varDef.Key.TrimStartAndEnd()))
			{
				TrimmedValue = varDef.Value;
							
			}
						
		}
	// now the fun part? this is just the audio params
		switch(UFK_SFZ_OpCode_Interpreter::Get_Value_Category_For_OpCode(key.ToString()))
		{
		case SFZ_Undefined:
			Region->SFZStringParamsArray.Add(key, TrimmedValue);
			break;
		case SFZInteger:
			if(KEY_IS("group")) {Region->group = FCString::Atoi(*TrimmedValue); break;}
			Region->SFZIntParamsArray.Add(key, FCString::Atoi(*TrimmedValue));
			break;
		case SFZFloat:
			if(KEY_IS("hivel")) {Region->hiVel = FCString::Atof(*TrimmedValue); break;}
			if(KEY_IS("lovel")) {Region->loVel = FCString::Atof(*TrimmedValue); break;}
			Region->SFZFloatParamsArray.Add(key, FCString::Atof(*TrimmedValue));
			break;
		case LoopEnum:
			if(VAL_IS("no_loop")) Region->loopMode = No_Loop;
			if(VAL_IS("one_shot")) Region->loopMode = One_Shot;
			if(VAL_IS("loop_continuous")) Region->loopMode = Loop_Continuous;
			if(VAL_IS("loop_sustain")) Region->loopMode = Loop_Sustain;
			break;
		case TriggerEnum:
			if (VAL_IS("attack"))	Region->triggerType = E_SFZ_TRIGGERTYPE::Attack;
			if (VAL_IS("release"))	Region->triggerType = E_SFZ_TRIGGERTYPE::Release;
			if (VAL_IS("first"))	Region->triggerType = E_SFZ_TRIGGERTYPE::First;
			if (VAL_IS("legato"))	Region->triggerType = E_SFZ_TRIGGERTYPE::Legato;
			if (VAL_IS("release_key"))	Region->triggerType = E_SFZ_TRIGGERTYPE::Release_Key;
			break;
		case Note:
			if (KEY_IS("pitch_keycenter") || KEY_IS("key"))
			{
	
				Region->centerNoteValue = FMath::Clamp(USFZAssetFactory::ParseOpCodeValueToInt(TrimmedValue) + Factory->Octave_Offset * 12, 0, 127);

			};
			if (KEY_IS("lokey") || KEY_IS("key"))
			{
				Region->loNote = FMath::Clamp(USFZAssetFactory::ParseOpCodeValueToInt(TrimmedValue) + Factory->Octave_Offset * 12, 0, 127);

			};
			if (KEY_IS("hikey") || KEY_IS("key"))
			{
				Region->hiNote = FMath::Clamp(USFZAssetFactory::ParseOpCodeValueToInt(TrimmedValue) + Factory->Octave_Offset * 12, 0, 127);

			};
			
			break;
		case SFZSample:
			{
				// if value is *silence or any of the special sample types ignore for now
				if (VAL_IS("*silence") || VAL_IS("*noise") || VAL_IS("*sine")) break;

				//second if Factory already imported the file, don't import it again
				if (Factory->ImportedWavsMap.Contains(FName(value)) )
				{
					Region->WavAsset;
					if(auto ptr = Factory->ImportedWavsMap.Find(key); ptr != nullptr)
					{
						Region->WavAsset = *ptr;
					}
					break;
				}

				//else, the garbage importing logic + adding to the map when done.
				Region->wavFilePath = FPaths::ConvertRelativePathToFull(
					Factory->PathString, FPaths::Combine(Factory->DefaultPath, TrimmedValue));
				FPaths::MakeStandardFilename(Region->wavFilePath);
				FPaths::NormalizeFilename(Region->wavFilePath);

				FString standartizedLocalPath = FPaths::GetPath(value);
				standartizedLocalPath.RemoveFromStart(TEXT(".."));
				FPaths::NormalizeFilename(standartizedLocalPath);
				ResultPath = FPaths::Combine(
					Factory->UnrealSamplesSavePath + FPaths::MakeValidFileName(standartizedLocalPath));
				FPaths::MakePlatformFilename(Region->wavFilePath);
				if (FPaths::FileExists(Region->wavFilePath))
				{
					TArray<FString> fileToImport;
					fileToImport.Add(Region->wavFilePath);

					UAutomatedAssetImportData* newSample = NewObject<UAutomatedAssetImportData>();

					newSample->DestinationPath = ResultPath;
					newSample->Filenames = fileToImport;
					newSample->bReplaceExisting = true;

					FString Extension = FPaths::GetExtension(ResultPath);
					GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(
						Factory, USoundWave::StaticClass(), Factory, FName(*fileToImport[0]), *Extension);

					auto ResultArray = AssetToolsModule.Get().ImportAssetsAutomated(newSample);
					if (!ResultArray.IsEmpty())
					{
						Region->WavAsset = Cast<USoundWave>(ResultArray[0]);
						Region->ObjectPtrWavAsset = TObjectPtr<USoundWave>(Region->WavAsset);
						Factory->ImportedWavsMap.Add(FName(value), Region->WavAsset);
					}
					else
					{
						Factory->NumFilesFailedToImport++;
					}
					GEditor->GetEditorSubsystem<UImportSubsystem>()->
							 BroadcastAssetPostImport(Factory, Region->WavAsset);
				}
				else
				{
					Factory->NumFilesFailedToImport++;
				}
			}
			break;
		case SFZTime:
			Region->SFZTimedParamsArray.Add(key, FCString::Atoi(*TrimmedValue));
			break;
		case SFZBoolInt:
			break;
		case SFZFloatDecibels:
			break;
		case SFZNormalizedTime:
			break;
		case SFZ_LegatoHalfEnum:
			break;
		case SFZ_Triggers:
			break;
		}

	
	}
}

FString USFZOpCodeLineParser::CheckForOpCodesAndTraverse(const FString& SubString)
{
	//parse opcodes, if stopping condition is not met, continue recursion
	if (SubString.Contains(TEXT("=")))
	{
		TArray<FString> WhiteSpaceDelimited;
		const int NextOpCodeSignIndex = SubString.Find(TEXT("="));
			
		SubString.Left(NextOpCodeSignIndex).TrimStartAndEnd().ParseIntoArrayWS(WhiteSpaceDelimited);
		FString LValue = WhiteSpaceDelimited.Last();
		const int LastIndexOfValue = SubString.Find(WhiteSpaceDelimited.Last());
			
		//the recursive call
		FString RValue;
		RValue = CheckForOpCodesAndTraverse(SubString.RightChop(NextOpCodeSignIndex + 1));
		OpCodeToValuesMap.Add(TPair<FString, FString>(LValue, RValue));
		//returning the value to the caller
		return SubString.Left(LastIndexOfValue);
	}
	//exit condition, if no more opcodes, I am a value, return.
	return SubString.TrimStartAndEnd();
}

int USFZOpCodeLineParser::GetNextOpCodeStartIndex(int CurrentOpCode)
{
	const int NextOpCodeSignIndex = TraversalString.Find(TEXT("="), ESearchCase::IgnoreCase, ESearchDir::FromStart, CurrentOpCode);

	if (NextOpCodeSignIndex == TraversalString.Len())	return TraversalString.Len();


	FString Token = TraversalString.Mid(CurrentOpCode, NextOpCodeSignIndex);
	FString Value = TraversalString.Mid(NextOpCodeSignIndex, GetNextOpCodeStartIndex(NextOpCodeSignIndex));
	OpCodeToValuesMap.Add(TTuple<FString, FString>(Token, Value));
	return NextOpCodeSignIndex;

}

USFZAssetFactory::USFZAssetFactory()
{
	//Formats.Add(FString(TEXT("sfz;")) + NSLOCTEXT("USFZAssetFactory", "FormatSfz", "SFZ File").ToString());
	Formats.Add(TEXT("SFZ;Fusion Patch"));
	SupportedClass = UFusionPatch::StaticClass();
	bCreateNew = false;
	bEditorImport = true;
	Octave_Offset = 12;
}

UObject* USFZAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	
	
	UFKSFZAsset* NewAsset = nullptr;
	FName newName = FName(InName.ToString().Append(TEXT("_SFZ")));
	UFusionPatch* NewFusionPatch = NewObject<UFusionPatch>(InParent,InClass, InName, Flags);
	SFZFileName = InName.ToString().Append(TEXT("_TEMP"));


	
	NumFilesFailedToImport = 0;

	if (!Filename.IsEmpty() && !bOutOperationCanceled)
	{
		//let's try creating both an SFZ and a fusion patch 
		
		NewAsset = NewObject<UFKSFZAsset>(InParent, UFKSFZAsset::StaticClass(), FName(SFZFileName), Flags);;
		newSFZAsset = NewAsset;
		

		TMap<FString, FString> sfzFileDefines;

		//import tally for debug 
		

		//SFZ Controls that affect import values and their 'non affecting' defaults. 
		DefaultPath = ""; //if the SFZ controls section define a default path, use it
		Octave_Offset = 0; //
		



		//end <controls> 

		FString fileString;
		TArray<FString> fileRegions;
		TArray<FString> groupsArray;
		TArray<FString> delimetedArray;
		FString samplesFolderPath;

		FString globalString;
	
		USFZOpCodeLineParser* Global = nullptr;
		USFZOpCodeLineParser* Master = nullptr;
		USFZOpCodeLineParser* Group = nullptr;
		USFZOpCodeLineParser* Control = nullptr;
		USFZOpCodeLineParser* Region = nullptr;

		FFileHelper::LoadFileToString(fileString, *Filename);
		PathString = FPaths::GetPath(*Filename);

		UnrealSamplesSavePath = TEXT("/Game/Imported/SFZSamples/");
		FPaths::MakePlatformFilename(UnrealSamplesSavePath);

	

		//deal with defines
		if (fileString.Contains(TEXT("#define")))
		{
			TArray<FString> defines;
			fileString.ParseIntoArrayWS(defines);
			for (int i = 0; i < defines.Num(); i++)
			{
				if (defines[i].StartsWith(TEXT("$")))
				{
					NewAsset->AssetDefines.Add(TTuple<FString, FString>(defines[i], defines[i+1]));
				}
			}

			defines.Empty();

		}

		//parse hierarchy shit
		fileString.ParseIntoArray(delimetedArray, TEXT("<"));
		FScopedSlowTask ParseSFZRegionsTask(delimetedArray.Num(), FText::FromString(TEXT("Processing SFZ file and importing samples")));
		auto progressHandle = FSlateNotificationManager::Get().StartProgressNotification(FText::FromString("Processing SFZ File Sections"), delimetedArray.Num());
		int processedRegions = 0;

		ParseSFZRegionsTask.MakeDialog();

		//constexpr auto HierarchyLiterals[] = {TEXT("control>"), TEXT("master>"), TEXT("global>"), TEXT("group>"), TEXT("region>")};

		for (auto delimetedSection : delimetedArray)
		{
			ParseSFZRegionsTask.EnterProgressFrame();
			
			FSlateNotificationManager::Get().UpdateProgressNotification(progressHandle, ++processedRegions);


			USFZOpCodeLineParser* Parser = NewObject<USFZOpCodeLineParser>(this);

			
			//control is a bit of a special case so we hit continue at the end here,
			if (delimetedSection.RemoveFromStart(TEXT("control>")))
			{
				Control = Parser;
				Control->TraverseString(delimetedSection);
				NewAsset->ControlOpCodes = Control->OpCodeToValuesMap;

				if (Control->OpCodeToValuesMap.Contains(TEXT("default_path")))
				{
					DefaultPath = *Control->OpCodeToValuesMap.Find(TEXT("default_path"));
				}

				if (Control->OpCodeToValuesMap.Contains(TEXT("octave_offset")))
				{
					Octave_Offset = FCString::Atoi(**Control->OpCodeToValuesMap.Find(TEXT("octave_offset")));
				}

				//we continue because for every token other than 'control' we only traverse the string after processing the hierarchy,
				//control is only expected once regardless. TODO: this is rather hacky because we already perform logic for the region in the end
				continue;
			};
			// this sorts the hierarchy, I think looking at it again I have doubts TODO: review hierarchy sorting
			if (delimetedSection.RemoveFromStart(TEXT("global>")))
			{
				//I'm already confused but the idea is always set the top level for all lower groups if you're top level,
				//That way lower levels can rely on just getting one degree higher from hierarchy,
				//For all other than region reset the region parser pointer, as its validity will be used to finally process the opcodes
				// every element can traverse the hierarchy until they meet a null pointer.
				// this should have been implemented as a linked list
				Global = Parser;
				Master = Global;
				Group = Global;
				Region = nullptr;
			};

			if (delimetedSection.RemoveFromStart(TEXT("master>")))
			{
				Master = Parser;
				Master->ParentPtr = Global;
				Group = Master;
				Region = nullptr;
			};

			if (delimetedSection.RemoveFromStart(TEXT("group>")))
			{
				Group = Parser;
				Group->ParentPtr = Master;
				Region = nullptr;
			};
			// with the current structure this is the only place where we actually process the opcodes, and it means we process the same opcode multiple times
			// would probably be more reasonable to process the opcodes as we receive them, will also allow us to create chain bindings
			if (delimetedSection.RemoveFromStart(TEXT("region>")))
			{
				Region = Parser;
				Region->ParentPtr = Group;
			}

			Parser->TraverseString(delimetedSection);
			Parser->CombineOpCodesWithParents();

			//so if region is not null ptr it's because we are processing a region, create a new region, populate it and add it to the array in the factor
			// final mapping happens when all regions are added.
			if(Region != nullptr)
			{
				USFZRegion* OutRegion = NewObject<USFZRegion>(NewAsset, USFZRegion::StaticClass());
				Parser->PopulateSFZRegion(OutRegion, NewAsset, this);
				OutRegion->RegionOpCodes = Parser->OpCodeToValuesMap;
				//init ranges if they weren't init from file 
				if (OutRegion->loNote == -1) OutRegion->loNote = OutRegion->centerNoteValue;
				if (OutRegion->hiNote == -1) OutRegion->hiNote = OutRegion->centerNoteValue;
				NewAsset->Regions.Add(OutRegion);
			}
		}
	}

	for (auto&[name, value]  : NewAsset->ControlOpCodes)
	{
		FString KeyCopy = name.ToString();
		if (KeyCopy.RemoveFromStart(TEXT("label_cc")))
		{
			NewAsset->ccLabelsMap.Add(TTuple<int, FString>(FCString::Atoi(*KeyCopy), value));
		}
			if (KeyCopy.RemoveFromStart(TEXT("set_cc")))
		{
			NewAsset->ccDefaultValMap.Add(TTuple<int, int>(FCString::Atoi(*KeyCopy), (FCString::Atoi(*value))));
		}
	}

	if (NumFilesFailedToImport > 0) {
		FSlateNotificationManager::Get().AddNotification(FNotificationInfo(FText::FromString(FString::Printf(TEXT("Done importing with warnings, failed to import %d files"), NumFilesFailedToImport))));
	} else {
		FSlateNotificationManager::Get().AddNotification(FText::FromString(FString::Printf(TEXT("Import Complete"))));
	}

	// this inits the sfz asset with the new data it received
	NewAsset->MapNotesToRanges();
	TArray <FKeyzoneSettings> KeyZoneArray;


	ShowImportDialog(356);

	//FKeyzoneSettings newKeyzone;
	//newKeyzone.

	//Given the new purpose of this guy we this is where we actually populate the fusion patch

	for (auto& processedSFZRegion : NewAsset->Regions)
	{
		//processedSFZRegion->WavAsset->LoadingBehavior = ESoundWaveLoadingBehavior::RetainOnLoad;

		//FKeyzoneArgs newZoneArgs = FKeyzoneArgs();
		//newZoneArgs.MaxNote = processedSFZRegion->hiNote;
		
		FKeyzoneSettings newKeyzone = FKeyzoneSettings();
		newKeyzone.SoundWave = TObjectPtr<USoundWave>(processedSFZRegion->WavAsset);

		newKeyzone.RootNote = processedSFZRegion->centerNoteValue;
		newKeyzone.MaxNote = processedSFZRegion->hiNote;
		newKeyzone.MinNote = processedSFZRegion->loNote;
		newKeyzone.MinVelocity = processedSFZRegion->loVel;
		newKeyzone.MaxVelocity = processedSFZRegion->hiVel;
		newKeyzone.bUnpitched = SFZsettings.bUnpitched;
		newKeyzone.bVelocityToGain = SFZsettings.velToGain;

		newKeyzone.SampleStartOffset = processedSFZRegion->Offset;
		

		if (processedSFZRegion->SFZFloatParamsArray.Contains(TEXT("volume")))
		{
			
			newKeyzone.Gain = FMath::LogX(10, *processedSFZRegion->SFZFloatParamsArray.Find(TEXT("volume")));
		}


		if (processedSFZRegion->SFZFloatParamsArray.Contains(TEXT("tune")))
		{
			newKeyzone.FineTuneCents = (*processedSFZRegion->SFZFloatParamsArray.Find(TEXT("tune")));
		}

		switch(processedSFZRegion->triggerType)
		{
			case E_SFZ_TRIGGERTYPE::Release:
			case E_SFZ_TRIGGERTYPE::Release_Key:
				newKeyzone.bIsNoteOffZone = true;
			break;

		}
		//newKeyzone.TimeStretchConfig.PitchShifter = 
		//newKeyzone.a

		KeyZoneArray.Add(newKeyzone);
	}

	


	//Audio::FProxyDataInitParams InitParams;
	FFusionPatchSettings newSettings = FFusionPatchSettings();

	newSettings.Adsr->Target = EAdsrTarget::Volume;
	newSettings.Adsr->AttackTime = SFZsettings.ampegAttack;
	newSettings.Adsr->ReleaseTime = SFZsettings.ampegRelease;
	newSettings.Adsr->DecayTime = SFZsettings.ampegDecay;
	newSettings.Adsr->Depth = 1.0f;
	newSettings.Adsr->SustainLevel= 1.0f;
	newSettings.Adsr->IsEnabled = true;

	newSettings.KeyzoneSelectMode = SFZsettings.keyzoneSelectMode;



	newSettings.Adsr->Calculate();


	
		NewFusionPatch->UpdateKeyzones(KeyZoneArray);
		NewFusionPatch->UpdateSettings(newSettings);

		//NewFusionPatch->UpdateRenderableFor
	
		AdditionalImportedObjects.Add(newSFZAsset);
		//Importe
		//Impo

	return NewFusionPatch;
	//return NewAsset;
}


int32 USFZAssetFactory::ParseOpCodeValueToInt(FString const& Value)
{

	//first we check if we got a number, cause numbers are good, we return the number
	int resultValue = FCString::Atoi(*Value);
	if (resultValue != 0)
	{
		return resultValue;
	}

	FString caseNormalized = Value.ToLower();
	int digitsAccumulator = 0;
	int flipSign = 1;

	for (auto c : caseNormalized.GetCharArray())
	{

		if (TChar<TCHAR>::IsDigit(c))
			{
			digitsAccumulator = (digitsAccumulator * 10) + FCString::Atoi(&c);
			}
		
		if (TChar<TCHAR>::IsAlpha(c))
		{
			if (c == 'd')
			{
				resultValue = 2;
			}

			if (c == 'e')
			{
				resultValue = 4;
			}

			if (c == 'f')
			{
				resultValue = 5;
			}

			if (c == 'g')
			{
				resultValue = 7;
			}

			if (c == 'a')
			{
				resultValue = 9;
			}

			if (c == 'b')
			{
				resultValue = 11;
			}
		}
		if (c == '#')
		{
			resultValue++;
		}
		if (c == '-')
		{
			flipSign = -1;
		}
	}
	
	return resultValue + (digitsAccumulator * flipSign * 12);
}

void SImportSFZSettingsDialog::Construct(const FArguments& InArgs)
{
	ChildSlot
		[
			SNew(SBorder)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
				[

					SNew(SVerticalBox)
						// Text for the conform option
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Padding(10)
						[
							SAssignNew(SFZFileInfoText, STextBlock)
								.TextStyle(&SFZImportInfoStyle)
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Padding(5)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT(
									"Please select the options suitable for this fusion patch")))
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Padding(5)
						[
							SNew(SNumericEntryBox<float>)
								.AllowSpin(true)
								.MinValue(0.0f)
								.MaxValue(2000.0f)
								.MinSliderValue(0.0f)
								.MaxSliderValue(2000.0f)
								.WheelStep(0.01f)
								.SliderExponent(0.01f)
								.MinFractionalDigits(6)
								//.Value(&settings.ampegAttack)
								.Value(this, &SImportSFZSettingsDialog::getAmpegAttack)
								.OnValueChanged(this, &SImportSFZSettingsDialog::setAmpegAttack)
								.Label()
								[
									SNew(STextBlock)
										.Text(FText::FromString(TEXT("Envelope Attack")))
										.TextStyle(&SFZImportInfoStyle)
								]

						]	
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Padding(5)
						[
							SNew(SNumericEntryBox<float>)
								.AllowSpin(true)
								.MinValue(0.0f)
								.MaxValue(2000.0f)
								.MinSliderValue(0.0f)
								.MaxSliderValue(2000.0f)
								.WheelStep(0.01f)
								.MinFractionalDigits(6)

								.Value(this, &SImportSFZSettingsDialog::getAmpegDecay)
								.OnValueChanged(this, &SImportSFZSettingsDialog::setAmpegDecay)
								.Label()
								[
									SNew(STextBlock)
										.Text(FText::FromString(TEXT("Envelope Decay")))
										.TextStyle(&SFZImportInfoStyle)
								]

						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Padding(5)
						[
							SNew(SNumericEntryBox<float>)
								.AllowSpin(true)
								.MinValue(0.0f)
								.MaxValue(2000.0f)
								.Value(this, &SImportSFZSettingsDialog::getAmpegRelease)
								.OnValueChanged(this, &SImportSFZSettingsDialog::setAmpegRelease)
								.MinSliderValue(0.0f)
								.MaxSliderValue(2000.0f)
								.WheelStep(0.01f)
								.MinFractionalDigits(6)
								.Label()
								[
									SNew(STextBlock)
										.Text(FText::FromString(TEXT("Envelope Release")))
										.TextStyle(&SFZImportInfoStyle)
								]

						]


						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Padding(5)
						[
							SNew(SCheckBox)
							//	.OnCheckStateChanged_Lambda
								.OnCheckStateChanged(this, &SImportSFZSettingsDialog::setUnpitchedBool)
								.Content()
								[
									SNew(STextBlock)
										.Text(FText::FromString(TEXT("Unpitched (percussive samples)")))
								]

						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Padding(5)
						[
							SNew(SCheckBox)
								.OnCheckStateChanged(this, &SImportSFZSettingsDialog::setVelToGain)
								.IsChecked(true)
								.Content()
								[
									SNew(STextBlock)
										.Text(FText::FromString(TEXT("Velocity to Gain")))
								]

						//combobox
						]

							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Padding(5)
							[
								SNew(SEnumComboBox, StaticEnum<EKeyzoneSelectMode>())
									.CurrentValue(this, &SImportSFZSettingsDialog::KeyzoneModeGetCurrentValue)
									.OnEnumSelectionChanged(this, &SImportSFZSettingsDialog::SetKeyzoneMode)

							]

						// "Ok" button
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(15)
						.VAlign(VAlign_Bottom)
						.HAlign(HAlign_Center)
						[
							SAssignNew(OkButton, SButton)
								.Text(FText::FromString(TEXT("Ok")))
						]
				]



				];
	
}
