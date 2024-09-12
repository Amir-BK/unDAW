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
	static FUndawStyle& Get()
	{
		static FUndawStyle Inst;
		return Inst;
	}


	FUndawStyle()
		: FSlateStyleSet("UndawStyle")
	{

	}



};