#include "Input/RawInputAccumulator.h"

void FRawInputAccumulator::SetButtonState(EActionInputButton key, bool bPressed)
{
}

void FRawInputAccumulator::SetInputAxis(const FVector2D& axis)
{
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
