#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/StyleColors.h"
#include "Styling/CoreStyle.h"
#include "Brushes/SlateRoundedBoxBrush.h"




class FUndawStyle final
	: public FSlateStyleSet
{
public:	


	void Initialize()
	{

	}


	static FUndawStyle& Get()
	{
		static FUndawStyle Inst;
		return Inst;
	}


	FUndawStyle()
		: FSlateStyleSet("UndawStyle")
	{
		Set("MidiNoteBrush", new FSlateRoundedBoxBrush(FStyleColors::White, 2.0f, FStyleColors::AccentBlack, 1.0f));
		Set("MidiNoteBrush.Selected", new FSlateRoundedBoxBrush(FStyleColors::White, 0.0f, FStyleColors::AccentBlack, 1.0f));
	}



};