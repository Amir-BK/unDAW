// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
//#include "GraphEditor.h"
#include "Framework/MarqueeRect.h"
#include <M2SoundGraphData.h>

struct FPianorollMarqueeOperation;
struct Rect;

/** Helper for managing marquee operations */
struct FPianorollMarqueeOperation
{
	FPianorollMarqueeOperation()
		: Operation(Add)
	{
	}

	enum Type
	{
		/** Holding down Alt removes nodes */
		Remove,
		/** Holding down Shift adds to the selection */
		Add,
		/** When nothing is pressed, marquee replaces selection */
		Replace,
		/** Holding down Ctrl toggles the selection state of all encompassed nodes */
		Invert,
	} Operation;

	bool IsValid() const
	{
		return Rect.IsValid();
	}

	void Start(const FVector2D& InStartLocation, FPianorollMarqueeOperation::Type InOperationType)
	{
		Rect = FMarqueeRect(InStartLocation);
		Operation = InOperationType;
	}

	void End()
	{
		Rect = FMarqueeRect();
	}


	/** Given a mouse event, figure out what the marquee selection should do based on the state of Shift and Ctrl keys */
	static FPianorollMarqueeOperation::Type OperationTypeFromMouseEvent(const FPointerEvent& MouseEvent)
	{
		if (MouseEvent.IsControlDown())
		{
			return FPianorollMarqueeOperation::Invert;
		}
		else if (MouseEvent.IsShiftDown())
		{
			return FPianorollMarqueeOperation::Add;
		}
		else if (MouseEvent.IsAltDown())
		{
			return FPianorollMarqueeOperation::Remove;
		}
		else
		{
			return FPianorollMarqueeOperation::Replace;
		}
	}

public:
	/** The marquee rectangle being dragged by the user */
	FMarqueeRect Rect;

	/** Nodes that will be selected or unselected by the current marquee operation */
	TSet<FLinkedMidiEvents*> AffectedNotes;
	//FGraphPanelSelectionSet AffectedNodes;
};


