#include "Input/RawInputAccumulator.h"

void FRawInputAccumulator::SetButtonState(EActionInputButton key, bool bPressed)
{
	const uint64 buttonMask = uint64{1} << static_cast<uint8>(key);

	if (bPressed)
	{
		if ((HeldMask & buttonMask) == 0)
		{
			LatchedPressedMask |= buttonMask;
		}
		
		HeldMask |= buttonMask;
	}
	else
	{
		if ((HeldMask & buttonMask) != 0)
		{
			LatchedReleasedMask |= buttonMask;
		}
		
		HeldMask &= ~buttonMask;
	}

	++LastInputSequence;
}

void FRawInputAccumulator::SetInputAxis(const FVector2D& axis)
{
	LatestMoveAxis = axis;
	++LastInputSequence;
}

FActionInputFrame FRawInputAccumulator::ConsumeFrame(int32 FrameId)
{
	FActionInputFrame Frame;
	Frame.FrameId = FrameId;
	Frame.HeldMask = HeldMask;
	Frame.PressedMask = LatchedPressedMask;
	Frame.ReleasedMask = LatchedReleasedMask;
	Frame.MoveAxis = LatestMoveAxis;
	Frame.LastInputSequence = LastInputSequence;

	LatchedPressedMask = 0;
	LatchedReleasedMask = 0;

	return Frame;
}
