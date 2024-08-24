// Fill out your copyright notice in the Description page of Project Settings.

#include "EngravingSubsystem.h"

#include "EngineGlobals.h"

void UEngravingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	//FBKMusicWidgetsModule::GetContentDir() + "Fonts\metadata\"
	//FString PluginContentDir = IPluginManager::Get().FindPlugin(TEXT("unDAW"))->GetContentDir();

	//FString SMUFLGlyphNamesFilepath = PluginContentDir + TEXT("/Fonts/Metadata/glyphnames.json");
	//FString SMUFLClassesMetaFilepath = PluginContentDir + TEXT("/Fonts/Metadata/classes.json");
	//FString result, classesResult;

	//FFileHelper::LoadFileToString(result, *SMUFLGlyphNamesFilepath);
	//FFileHelper::LoadFileToString(classesResult, *SMUFLClassesMetaFilepath);

	//TSharedPtr<FJsonObject> GlyphsJSON, ClassesJSON;

	//FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(result), GlyphsJSON);
	//for (auto i = GlyphsJSON.Get()->Values.CreateConstIterator(); i; ++i)
	//{
	//	auto JSONObject = i->Value->AsObject();
	//	FString tempString = i->Value->AsObject()->GetStringField(TEXT("codepoint"));
	//	GlyphToUnicodeMap.Add(FName(i->Key), FParse::HexNumber(*tempString.Right(4)));
	//	//UE_LOG(LogTemp, Warning,TEXT("Key is: %s"), *i->Key);
	//};

	//FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(classesResult), ClassesJSON);
	//for (auto i = ClassesJSON.Get()->Values.CreateConstIterator(); i; ++i)
	//{
	//	FGlyphCategory newCategory;

	//	auto ContentArray = i->Value->AsArray();
	//	for (auto& item : ContentArray) {
	//		newCategory.ContentGlyphs.Add(FName(item.Get()->AsString()));
	//	}
	//	//FString tempString = i->Value->AsObject()->GetStringField(TEXT("codepoint"));
	//	//GlyphToUnicodeMap.Add(FName(i->Key), FParse::HexNumber(*tempString.Right(4)));
	//	SMUFLClasses.Add(FName(i->Key), newCategory);
	//};
}

int32 UEngravingSubsystem::GetUnicodeIntForGlyph(FName GlyphName)
{
	int32* ret = GlyphToUnicodeMap.Find(GlyphName);

	return (ret != nullptr) ? *ret : 0;
}

void UEngravingSubsystem::PopulateDataAssetWithGlyphDataFromJSON(UFont* FontObject, FFilePath Metadata_File_Path, UMusicFontDataAsset* Font_Data_Asset)
{
	//find which glyphs are actually renderable by the font
	auto FontCache = FSlateApplication::Get().GetRenderer()->GetFontCache();
	auto FontData = FontCache->GetDefaultFontData(FSlateFontInfo(FontObject, 36));

	for (auto& i : GEngine->GetEngineSubsystem<UEngravingSubsystem>()->GlyphToUnicodeMap)
	{
		if (FontCache->CanLoadCodepoint(FontData, i.Value))
		{
			Font_Data_Asset->glyphs.Add(i.Key, FGlyphData(i.Key, i.Value));
		}
	}

	//get the metadata JSON, iterate over it, populate glyphs metadata
	FString result;
	TSharedPtr<FJsonObject> metadataJSON;

	FFileHelper::LoadFileToString(result, *(Metadata_File_Path.FilePath));

	if (FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(result), metadataJSON))
	{
		for (auto i = metadataJSON.Get()->Values.CreateConstIterator(); i; ++i)
		{
			auto JSONObject = i->Value->AsObject();
			auto key = i->Key;
			// so we just need to do a series of if statements to parse the JSON, I think.

			if (key.Equals(TEXT("glyphAdvanceWidths"), ESearchCase::IgnoreCase))
			{
				for (auto advancedWidthsEntries = JSONObject->Values.CreateConstIterator(); advancedWidthsEntries; ++advancedWidthsEntries)
				{
					if (Font_Data_Asset->glyphs.Contains(FName(advancedWidthsEntries->Key)))
					{
						Font_Data_Asset->glyphs[FName(advancedWidthsEntries->Key)].metadata.Width =
							(float)advancedWidthsEntries->Value->AsNumber();
					};
				};
			};
			//glyph anchors
			if (key.Equals(TEXT("glyphsWithAnchors"), ESearchCase::IgnoreCase))
			{
				for (auto withAnchorsEntries = JSONObject->Values.CreateConstIterator();
					withAnchorsEntries; ++withAnchorsEntries)
				{
					if (Font_Data_Asset->glyphs.Contains(FName(withAnchorsEntries->Key)))
					{
						for (auto glyphAnchors = withAnchorsEntries->Value.Get()->AsObject()->Values.CreateConstIterator();
							glyphAnchors; ++glyphAnchors)
						{
							FVector2f pointFromJSON = FVector2f(glyphAnchors->Value->AsArray()[0]->AsNumber(),
								glyphAnchors->Value->AsArray()[1]->AsNumber());
							Font_Data_Asset->glyphs[FName(withAnchorsEntries->Key)].metadata.anchors.Add(FName(glyphAnchors->Key), pointFromJSON);
						}
					};
				};
			};

			//glyph bboxes
			if (key.Equals(TEXT("glyphBBoxes"), ESearchCase::IgnoreCase))
			{
				for (auto glyphBBoxesJSONs = JSONObject->Values.CreateConstIterator();
					glyphBBoxesJSONs; ++glyphBBoxesJSONs)
				{
					if (Font_Data_Asset->glyphs.Contains(FName(glyphBBoxesJSONs->Key)))
					{
						for (auto glyphBBoxes = glyphBBoxesJSONs->Value.Get()->AsObject()->Values.CreateConstIterator();
							glyphBBoxes; ++glyphBBoxes)
						{
							FVector2f pointFromJSON = FVector2f(glyphBBoxes->Value->AsArray()[0]->AsNumber(),
								glyphBBoxes->Value->AsArray()[1]->AsNumber());
							Font_Data_Asset->glyphs[FName(glyphBBoxesJSONs->Key)].metadata.bbox.Add(FName(glyphBBoxes->Key), pointFromJSON);
						}
					};
				};
			};
		};
	};
}